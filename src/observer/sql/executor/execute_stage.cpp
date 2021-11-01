/* Copyright (c) 2021 Xie Meiyi(xiemeiyi@hust.edu.cn) and OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Longda on 2021/4/13.
//

#include <string>
#include <sstream>
#include <stack>

#include "execute_stage.h"

#include "common/io/io.h"
#include "common/log/log.h"
#include "common/seda/timer_stage.h"
#include "common/lang/string.h"
#include "session/session.h"
#include "event/storage_event.h"
#include "event/sql_event.h"
#include "event/session_event.h"
#include "event/execution_plan_event.h"
#include "sql/executor/execution_node.h"
#include "sql/executor/tuple.h"
#include "storage/common/table.h"
#include "storage/default/default_handler.h"
#include "storage/common/condition_filter.h"
#include "storage/trx/trx.h"

using namespace common;//
//函数声明
RC create_selection_executor(Trx *trx, const Selects &selects, const char *db, const char *table_name,
                             SelectExeNode &select_node);

static RC schema_add_field(Table *table, const char *field_name, TupleSchema &schema);

RC create_out_schema(const char *db, const Selects selects, TupleSchema &tupleSchema, TupleSchema *tupleSchemas,struct filter* filter,int &num);

static RC schema_add_field_except_exist(Table *table, const char *field_name, TupleSchema &schema);

RC create_selection_executor(Trx *trx, const Selects &selects, const char *db, const char *table_name,
                             SelectExeNode &select_node, TupleSchema tupleSchema);

//! Constructor
ExecuteStage::ExecuteStage(const char *tag) : Stage(tag) {}

//! Destructor
ExecuteStage::~ExecuteStage() {}

//! Parse properties, instantiate a stage object
Stage *ExecuteStage::make_stage(const std::string &tag) {
    ExecuteStage *stage = new(std::nothrow) ExecuteStage(tag.c_str());
    if (stage == nullptr) {
        LOG_ERROR("new ExecuteStage failed");
        return nullptr;
    }
    stage->set_properties();
    return stage;
}

//! Set properties for this object set in stage specific properties
bool ExecuteStage::set_properties() {
    //  std::string stageNameStr(stageName);
    //  std::map<std::string, std::string> section = theGlobalProperties()->get(
    //    stageNameStr);
    //
    //  std::map<std::string, std::string>::iterator it;
    //
    //  std::string key;

    return true;
}

//! Initialize stage params and validate outputs
bool ExecuteStage::initialize() {
    LOG_TRACE("Enter");

    std::list<Stage *>::iterator stgp = next_stage_list_.begin();
    default_storage_stage_ = *(stgp++);
    mem_storage_stage_ = *(stgp++);

    LOG_TRACE("Exit");
    return true;
}

//! Cleanup after disconnection
void ExecuteStage::cleanup() {
    LOG_TRACE("Enter");

    LOG_TRACE("Exit");
}

void ExecuteStage::handle_event(StageEvent *event) {
    LOG_TRACE("Enter\n");

    handle_request(event);

    LOG_TRACE("Exit\n");
    return;
}

void ExecuteStage::callback_event(StageEvent *event, CallbackContext *context) {
    LOG_TRACE("Enter\n");

    // here finish read all data from disk or network, but do nothing here.
    ExecutionPlanEvent *exe_event = static_cast<ExecutionPlanEvent *>(event);
    SQLStageEvent *sql_event = exe_event->sql_event();
    sql_event->done_immediate();

    LOG_TRACE("Exit\n");
    return;
}

void ExecuteStage::handle_request(common::StageEvent *event) {
    ExecutionPlanEvent *exe_event = static_cast<ExecutionPlanEvent *>(event);
    SessionEvent *session_event = exe_event->sql_event()->session_event();
    Query *sql = exe_event->sqls();
    const char *current_db = session_event->get_client()->session->get_current_db().c_str();

    CompletionCallback *cb = new(std::nothrow) CompletionCallback(this, nullptr);
    if (cb == nullptr) {
        LOG_ERROR("Failed to new callback for ExecutionPlanEvent");
        exe_event->done_immediate();
        return;
    }
    exe_event->push_callback(cb);

    switch (sql->flag) {
        case SCF_SELECT: { // select
            do_select(current_db, sql, exe_event->sql_event()->session_event());
            exe_event->done_immediate();
        }
            break;
            //这是和储存相关的event
        case SCF_INSERT:
        case SCF_UPDATE:
        case SCF_DELETE:
        case SCF_CREATE_TABLE:
        case SCF_SHOW_TABLES:
        case SCF_DESC_TABLE:
        case SCF_DROP_TABLE:
        case SCF_CREATE_INDEX:
        case SCF_DROP_INDEX:
        case SCF_LOAD_DATA: {
            StorageEvent *storage_event = new(std::nothrow) StorageEvent(exe_event);
            if (storage_event == nullptr) {
                LOG_ERROR("Failed to new StorageEvent");
                event->done_immediate();
                return;
            }

            default_storage_stage_->handle_event(storage_event);
        }
            break;
        case SCF_SYNC: {
            RC rc = DefaultHandler::get_default().sync();
            session_event->set_response(strrc(rc));
            exe_event->done_immediate();
        }
            break;
        case SCF_BEGIN: {
            session_event->get_client()->session->set_trx_multi_operation_mode(true);
            exe_event->done_immediate();
        }
            break;
        case SCF_COMMIT: {
            Trx *trx = session_event->get_client()->session->current_trx();
            RC rc = trx->commit();
            session_event->get_client()->session->set_trx_multi_operation_mode(false);
            session_event->set_response(strrc(rc));
            exe_event->done_immediate();
        }
            break;
        case SCF_ROLLBACK: {
            Trx *trx = session_event->get_client()->session->current_trx();
            RC rc = trx->rollback();
            session_event->get_client()->session->set_trx_multi_operation_mode(false);
            session_event->set_response(strrc(rc));
            exe_event->done_immediate();
        }
            break;
        case SCF_HELP: {
            const char *response = "show tables;\n"
                                   "desc `table name`;\n"
                                   "create table `table name` (`column name` `column type`, ...);\n"
                                   "create index `index name` on `table` (`column`);\n"
                                   "insert into `table` values(`value1`,`value2`);\n"
                                   "update `table` set column=value [where `column`=`value`];\n"
                                   "delete from `table` [where `column`=`value`];\n"
                                   "select [ * | `columns` ] from `table`;\n";
            session_event->set_response(response);
            exe_event->done_immediate();
        }
            break;
        case SCF_EXIT: {
            // do nothing
            const char *response = "Unsupported\n";
            session_event->set_response(response);
            exe_event->done_immediate();
        }
            break;
        default: {
            exe_event->done_immediate();
            LOG_ERROR("Unsupported command=%d\n", sql->flag);
        }
    }
}

void end_trx_if_need(Session *session, Trx *trx, bool all_right) {
    if (!session->is_trx_multi_operation_mode()) {
        if (all_right) {
            trx->commit();
        } else {
            trx->rollback();
        }
    }
}
bool do_filter(std::vector<Tuple*>* stack,struct filter* filters,int num){
    for(int i=0;i<num;i++){
        int ret;
        if(filters[i].is_same_type){
            ret=stack->at(filters[i].left_table)->get(filters[i].left_value).compare(stack->at(filters[i].right_table)->get(filters[i].right_value));
        } else{
            float result=stack->at(filters[i].left_table)->get(filters[i].left_value).getFValue()-stack->at(filters[i].right_table)->get(filters[i].right_value).getFValue();
            if(result>0){
                ret=1;
            } else if(result<0){
                ret=-1;
            }
            else{
                ret=0;
            }
        }
        switch (filters[i].op) {
            case CompOp::EQUAL_TO:{
                if(ret!=0){
                    return false;
                }
                break;
            }

            case CompOp::GREAT_EQUAL: {
                if (ret < 0) {
                    return false;
                }
                break;
            }
            case CompOp::GREAT_THAN:{
                if(ret<=0){
                    return false;
                }
                break;
            }
            case CompOp::LESS_EQUAL:{
                if(ret>0){
                    return false;
                }
                break;
            }
            case CompOp::LESS_THAN:{
                if(ret>=0){
                    return false;
                }
                break;
            }
            case CompOp::NOT_EQUAL:{
                if(ret==0){
                    return false;
                }
                break;
            }
            case CompOp::NO_OP:
                break;

        }

    }
    return true;
}
/**
 * 创建输出模式并进行校验,构造每个表的选择属性,schemas i是realtions i的投影模式,并构造flitet
 * @param selects
 * @param tupleSchema
 * @return
 */
RC create_out_schema(const char *db, const Selects selects, TupleSchema &tupleSchema, TupleSchema *tupleSchemas,struct filter* filters,int &num) {
    std::vector<std::string> table_names;
    for(int i=0;i<selects.relation_num-1;i++){
        table_names.emplace_back(selects.relations[i]);
    }
    for (int i = selects.attr_num - 1; i >= 0; i--) {//遍历要从后往前，因为sql解析的缘故
        const RelAttr &attr = selects.attributes[i];
        if (nullptr == attr.relation_name) {
            if (0 == strcmp("*", attr.attribute_name)) {
                // 列出所有查询表的所有字段 是不是也要反着来
                for (int j = selects.relation_num - 1; j >= 0; j--) {
                    TupleSchema tmpSchema;
                    Table *table = DefaultHandler::get_default().find_table(db, selects.relations[j]);
                    if (table == nullptr) {//表不存在
                        return RC::SCHEMA_TABLE_NOT_EXIST;
                    }
                    TupleSchema::from_table(table, tmpSchema);
                    tupleSchemas[j].append_if_not_exists(tmpSchema);
                    tupleSchema.append(tmpSchema);
                }
            } else{
                if(selects.relation_num!=1){
                    return RC::MISMATCH;
                }
                else{
                    Table *table = DefaultHandler::get_default().find_table(db, selects.relations[0]);
                    //列出表的相关字段不去重
                    RC rc = schema_add_field_except_exist(table, attr.attribute_name, tupleSchema);
                    if (rc != RC::SUCCESS) {
                        return rc;
                    }
                    rc = schema_add_field(table, attr.attribute_name, tupleSchemas[0]);
                    if (rc != RC::SUCCESS) {
                        return rc;
                    }
                }
            }
        }
        else {
            TupleSchema tmpSchema;
            Table *table = DefaultHandler::get_default().find_table(db, attr.relation_name);
            //要在后面的relationo里 这里是校验
            int j;
            for (j = 0; j < selects.relation_num; j++) {
                if (strcmp(attr.relation_name, selects.relations[j]) == 0) {
                    break;
                }
            }
            if (j == selects.relation_num) {//在后面没找到
                return RC::SCHEMA_TABLE_NOT_EXIST;//虽然这个错误不太标准
            }
            if (0 == strcmp("*", attr.attribute_name)) {
                // 列出该查询表的所有字段 是不是也要反着来
                TupleSchema tmpSchema;
                Table *table = DefaultHandler::get_default().find_table(db, attr.relation_name);
                TupleSchema::from_table(table, tmpSchema);
                tupleSchema.append(tmpSchema);
                tupleSchemas[j].append_if_not_exists(tmpSchema);
            } else {
                //列出表的相关字段不去重
                RC rc = schema_add_field_except_exist(table, attr.attribute_name, tupleSchema);
                if (rc != RC::SUCCESS) {
                    return rc;
                }
                rc = schema_add_field(table, attr.attribute_name, tupleSchemas[j]);
                if (rc != RC::SUCCESS) {
                    return rc;
                }
            }
        }
    }
    std::vector<DefaultConditionFilter *> condition_filters;
    for (size_t i = 0; i < selects.condition_num; i++) {
        const Condition &condition = selects.conditions[i];//conditon必须在from里面
        if(condition.left_is_attr == 1 && condition.right_is_attr == 1&& strcmp(condition.right_attr.relation_name,condition.left_attr.relation_name)!=0){//表之间联系
                //要找到对应的表位置与，属性位置
                //先找左边
                AttrType left;
                AttrType right;
                filters[num].op=condition.comp;
                int count=0;
                for(int j=0;j<selects.relation_num;j++){
                    if(strcmp(condition.left_attr.relation_name,selects.relations[j])==0){
                        filters[num].left_table=j;
                        int id=tupleSchemas[j].index_of_field(condition.left_attr.relation_name,condition.left_attr.attribute_name);
                        if(id==-1){
                            Table *table = DefaultHandler::get_default().find_table(db, condition.left_attr.relation_name);
                            RC rc = schema_add_field(table, condition.left_attr.attribute_name, tupleSchemas[j]);
                            if(rc!=RC::SUCCESS){
                                return RC::SCHEMA_FIELD_MISSING;
                            }
                        }
                        id=tupleSchemas[j].index_of_field(condition.left_attr.relation_name,condition.left_attr.attribute_name);
                        filters[num].left_value=id;
                        left=tupleSchemas[j].field(id).type();
                        count++;
                    }
                    if(strcmp(condition.right_attr.relation_name,selects.relations[j])==0){
                        filters[num].right_table=j;
                        int id=tupleSchemas[j].index_of_field(condition.right_attr.relation_name,condition.right_attr.attribute_name);
                        if(id==-1){
                            Table *table = DefaultHandler::get_default().find_table(db, condition.right_attr.relation_name);
                            RC rc = schema_add_field(table, condition.right_attr.attribute_name, tupleSchemas[j]);
                            if(rc!=RC::SUCCESS){
                                return RC::SCHEMA_FIELD_MISSING;
                            }
                        }
                        id=tupleSchemas[j].index_of_field(condition.right_attr.relation_name,condition.right_attr.attribute_name);
                        filters[num].right_value=id;
                        right=tupleSchemas[j].field(id).type();
                        count++;
                    }
                }
                //进行左右比较
                if(count!=2){
                    return RC::MISMATCH;
                }
                if(right!=left){
                    if((right==AttrType::FLOATS&&left==AttrType::INTS)||(left==AttrType::FLOATS&&right==AttrType::INTS)){
                        filters[num].is_same_type= false;
                    }
                    else{
                        return RC::MISMATCH;
                    }
                }
                num++;
        }
    }
    return RC::SUCCESS;
}

// 这里没有对输入的某些信息做合法性校验，比如查询的列名、where条件中的列名等，没有做必要的合法性校验
// 需要补充上这一部分. 校验部分也可以放在resolve，不过跟execution放一起也没有关系
RC ExecuteStage::do_select(const char *db, Query *sql, SessionEvent *session_event) {

    RC rc = RC::SUCCESS;
    Session *session = session_event->get_client()->session;
    Trx *trx = session->current_trx();
    TupleSchema schemas[sql->sstr.selection.relation_num];//每个表的模式
    const Selects &selects = sql->sstr.selection;
    TupleSchema out_schema;
    struct filter filters[20];
    int num=0;//filters的个数
    //构造最后的输出schema与各表查询的schema,以及构建表间的比较
    if ((rc = create_out_schema(db, selects, out_schema, schemas,filters,num)) != RC::SUCCESS) {
        const char *failure_ptr = "FAILURE\n";//这种地方复制有点冗余
        session_event->set_response(failure_ptr);
        end_trx_if_need(session, trx, false);
        return rc;
    }
    //构造查询的内容到输出内容的映射
    int size=out_schema.get_field_size();
    struct out_map map[size];
    for(int j=0;j<size;j++) {
        TupleField field = out_schema.field(j);
        for (int i = 0; i < selects.relation_num; i++) {
            int id = schemas[i].index_of_field(field.table_name(), field.field_name());
            if (id != -1) {
                map[num].table = i;
                map[num].value = id;
                num++;
                break;
            }
        }
    }
    // 把所有的表和只跟这张表关联的condition都拿出来，生成最底层的select 执行节点

    std::vector<SelectExeNode *> select_nodes;
    for (size_t i = 0; i <selects.relation_num; i++) {
        const char *table_name = selects.relations[i];
        SelectExeNode *select_node = new SelectExeNode;
        rc = create_selection_executor(trx, selects, db, table_name, *select_node, schemas[i]);
        if (rc != RC::SUCCESS) {
            delete select_node;
            for (SelectExeNode *&tmp_node: select_nodes) {
                delete tmp_node;
            }
            const char *failure_ptr = "FAILURE\n";
            session_event->set_response(failure_ptr);
            end_trx_if_need(session, trx, false);
            return rc;
        }
        select_nodes.push_back(select_node);
    }

    if (select_nodes.empty()) {
        LOG_ERROR("No table given");
        end_trx_if_need(session, trx, false);
        return RC::SQL_SYNTAX;
    }

    std::vector<TupleSet> tuple_sets;
    for (SelectExeNode *&node: select_nodes) {
        TupleSet tuple_set;
        rc = node->execute(tuple_set);
        if (rc != RC::SUCCESS) {
            for (SelectExeNode *&tmp_node: select_nodes) {
                delete tmp_node;
            }
            end_trx_if_need(session, trx, false);
            return rc;
        } else {
            tuple_sets.push_back(std::move(tuple_set));
        }
    }
    TupleSet tupleSet;
    tupleSet.set_schema(out_schema);
    std::stringstream ss;
    if (tuple_sets.size() > 1) {
        // 本次查询了多张表，需要做join操作
        //递归笛卡尔
        int size = tuple_sets.size();
        int count[size];
        int height = 0;
        memset(count, 0, size * sizeof(int));
        std::vector<Tuple*> stack;
        bool flag=true;
        while (flag) {//没有数据会bug
            if (height != size) {
                if(count[height]>=tuple_sets.at(height).size()){
                    break;
                }
                stack.emplace_back(tuple_sets.at(height).get(count[height]));
                count[height]++;
                height++;
                continue;
            }
            if(height==size){
                //输出tuple 如果符合条件
                Tuple tuple;//先进行判断再add
                bool add= do_filter(&stack,filters,num);
                if(add){//向输出里加入
                    std::vector<std::shared_ptr<TupleValue>> values;
                    for(int i=0;i<out_schema.fields().size();i++){
                        values.emplace_back(stack[map[i].table]->values()[map[i].value]);
                    }
                    tuple.add(values);
                    tupleSet.add(std::move(tuple));
                }
                stack.pop_back();
                height--;
                while (count[height]>=tuple_sets.at(height).size()){
                    if(height==0){
                        flag= false;
                        break;
                    }
                    stack.pop_back();
                    count[height]=0;
                    height--;
                }
            }
        }
        //表的答应
        tupleSet.print(ss);
    } else {
        // 当前只查询一张表，直接返回结果即可
        tuple_sets.front().print(ss);
    }

    for (SelectExeNode *&tmp_node: select_nodes) {
        delete tmp_node;
    }
    session_event->set_response(ss.str());
    end_trx_if_need(session, trx, true);
    return rc;
}

bool match_table(const Selects &selects, const char *table_name_in_condition, const char *table_name_to_match) {
    if (table_name_in_condition != nullptr) {
        return 0 == strcmp(table_name_in_condition, table_name_to_match);
    }

    return selects.relation_num == 1;
}

static RC schema_add_field(Table *table, const char *field_name, TupleSchema &schema) {
    const FieldMeta *field_meta = table->table_meta().field(field_name);
    if (nullptr == field_meta) {
        LOG_WARN("No such field. %s.%s", table->name(), field_name);
        return RC::SCHEMA_FIELD_MISSING;
    }

    schema.add_if_not_exists(field_meta->type(), table->name(), field_meta->name());
    return RC::SUCCESS;
}

static RC schema_add_field_except_exist(Table *table, const char *field_name, TupleSchema &schema) {
    const FieldMeta *field_meta = table->table_meta().field(field_name);
    if (nullptr == field_meta) {
        LOG_WARN("No such field. %s.%s", table->name(), field_name);
        return RC::SCHEMA_FIELD_MISSING;
    }

    schema.add(field_meta->type(), table->name(), field_meta->name());
    return RC::SUCCESS;
}

// 把所有的表和只跟这张表关联的condition都拿出来，生成最底层的select 执行节点，根据selects里的attr
RC create_selection_executor(Trx *trx, const Selects &selects, const char *db, const char *table_name,
                             SelectExeNode &select_node) {
    // 列出跟这张表关联的Attr
    TupleSchema schema;
    Table *table = DefaultHandler::get_default().find_table(db, table_name);
    if (nullptr == table) {
        LOG_WARN("No such table [%s] in db [%s]", table_name, db);
        return RC::SCHEMA_TABLE_NOT_EXIST;
    }

    for (int i = selects.attr_num - 1; i >= 0; i--) {
        const RelAttr &attr = selects.attributes[i];
        if (nullptr == attr.relation_name || 0 == strcmp(table_name, attr.relation_name)) {
            if (0 == strcmp("*", attr.attribute_name)) {
                // 列出这张表所有字段
                TupleSchema::from_table(table, schema);
                break; // TODO 没有校验，给出* 之后，再写字段的错误
            } else {
                // 列出这张表相关字段
                RC rc = schema_add_field(table, attr.attribute_name, schema);
                if (rc != RC::SUCCESS) {
                    return rc;
                }
            }
        }
    }

    // 找出仅与此表相关的过滤条件, 或者都是值的过滤条件
    std::vector<DefaultConditionFilter *> condition_filters;
    for (size_t i = 0; i < selects.condition_num; i++) {
        const Condition &condition = selects.conditions[i];
        if ((condition.left_is_attr == 0 && condition.right_is_attr == 0) || // 两边都是值
            (condition.left_is_attr == 1 && condition.right_is_attr == 0 &&
             match_table(selects, condition.left_attr.relation_name, table_name)) ||  // 左边是属性右边是值
            (condition.left_is_attr == 0 && condition.right_is_attr == 1 &&
             match_table(selects, condition.right_attr.relation_name, table_name)) ||  // 左边是值，右边是属性名
            (condition.left_is_attr == 1 && condition.right_is_attr == 1 &&
             match_table(selects, condition.left_attr.relation_name, table_name) &&
             match_table(selects, condition.right_attr.relation_name, table_name)) // 左右都是属性名，并且表名都符合
                ) {
            DefaultConditionFilter *condition_filter = new DefaultConditionFilter();
            RC rc = condition_filter->init(*table, condition);
            if (rc != RC::SUCCESS) {
                delete condition_filter;
                for (DefaultConditionFilter *&filter: condition_filters) {
                    delete filter;
                }
                return rc;
            }
            condition_filters.push_back(condition_filter);
        }
    }

    return select_node.init(trx, table, std::move(schema), std::move(condition_filters));
}

// 把所有的表和只跟这张表关联的condition都拿出来，生成最底层的select 执行节点，根据selects里的attr,并且根据tupleschema选择属性
RC create_selection_executor(Trx *trx, const Selects &selects, const char *db, const char *table_name,
                             SelectExeNode &select_node, TupleSchema tupleSchema) {
    // 列出跟这张表关联的Attr
    Table *table = DefaultHandler::get_default().find_table(db, table_name);
    if (nullptr == table) {
        LOG_WARN("No such table [%s] in db [%s]", table_name, db);
        return RC::SCHEMA_TABLE_NOT_EXIST;
    }
    // 找出仅与此表相关的过滤条件, 或者都是值的过滤条件
    std::vector<DefaultConditionFilter *> condition_filters;
    for (size_t i = 0; i < selects.condition_num; i++) {
        const Condition &condition = selects.conditions[i];
        if ((condition.left_is_attr == 0 && condition.right_is_attr == 0) || // 两边都是值
            (condition.left_is_attr == 1 && condition.right_is_attr == 0 &&
             match_table(selects, condition.left_attr.relation_name, table_name)) ||  // 左边是属性右边是值
            (condition.left_is_attr == 0 && condition.right_is_attr == 1 &&
             match_table(selects, condition.right_attr.relation_name, table_name)) ||  // 左边是值，右边是属性名
            (condition.left_is_attr == 1 && condition.right_is_attr == 1 &&
             match_table(selects, condition.left_attr.relation_name, table_name) &&
             match_table(selects, condition.right_attr.relation_name, table_name)) // 左右都是属性名，并且表名都符合
                ) {
            DefaultConditionFilter *condition_filter = new DefaultConditionFilter();
            RC rc = condition_filter->init(*table, condition);
            if (rc != RC::SUCCESS) {
                delete condition_filter;
                for (DefaultConditionFilter *&filter: condition_filters) {
                    delete filter;
                }
                return rc;
            }
            condition_filters.push_back(condition_filter);
        }
    }

    return select_node.init(trx, table, std::move(tupleSchema), std::move(condition_filters));
}
