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

void cal_agg(TupleSet &tupleSet, TupleSet &ret, int begin, int end);

static RC schema_add_field(Table *table, const char *field_name, TupleSchema &schema);

RC create_out_schema(const char *db, const Selects selects, TupleSchema &tupleSchema, TupleSchema *tupleSchemas,
                     std::vector<Condition> &t_t_conditions, int &num);

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

void out_agg_type(AggType aggType, std::ostream &ss) {
    switch (aggType) {
        case Max: {
            ss << "max";
        }
            break;
        case Min: {
            ss << "min";
        }
            break;
        case Count: {
            ss << "count";
        }
            break;
        case Avg: {
            ss << "avg";
        }
            break;
        case Null:
            assert(0);
            break;
    }
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

/**
 * 判读一个表达式是否为最简单的att
 * @param relAttr
 * @param ret
 * @return
 */
bool is_simple_ex(RelAttr relAttr, std::vector<RelAttr> &ret) {
    bool is_simple = true;
    switch (relAttr.extype) {
        case E: {

        }
        case T: {
            if (relAttr.second == nullptr) {
                is_simple = is_simple_ex(*relAttr.first, ret);
            } else {
                is_simple = false;
                is_simple_ex(*relAttr.first, ret);
                is_simple_ex(*relAttr.second, ret);
            }
        }
            break;
        case F: {
            is_simple = false;
            is_simple_ex(*relAttr.first, ret);
            break;
        }
        case id: {
            ret.emplace_back(relAttr);
            is_simple = true;
            break;
        }
        case val: {
            ret.emplace_back(relAttr);
            is_simple = true;
            break;
        }
    }
    return is_simple;
}
Condition cast_condition_to_simple(Condition condition) {
    std::vector<RelAttr> lefts;
    std::vector<RelAttr> rights;
    bool is_simple = is_simple_ex(condition.left_attr, lefts) && is_simple_ex(condition.right_attr, rights);
    RelAttr left_attr = lefts[0];
    RelAttr right_attr = rights[0];
    if (is_simple) {
        if ((left_attr.extype == val && condition.right_attr.extype == val) || // 两边都是值
            (left_attr.extype == id && right_attr.extype == val)) {
            DefaultConditionFilter *condition_filter = new DefaultConditionFilter();
            Condition condition1;
            condition1.comp = condition.comp;
            if ((left_attr.extype == val && condition.right_attr.extype == val)) {
                condition1.right_is_attr = 0;
                condition1.left_is_attr = 0;
                condition1.right_value = right_attr.value;
                condition1.left_value = left_attr.value;

            }
            if ((left_attr.extype == id && right_attr.extype == val)) {
                condition1.right_is_attr = 0;
                condition1.left_is_attr = 1;
                condition1.right_value = right_attr.value;
                condition1.left_attr = left_attr;
            }
            if ((left_attr.extype == val && right_attr.extype == id)) {
                condition1.right_is_attr = 1;
                condition1.left_is_attr = 0;
                condition1.left_value = left_attr.value;
                condition1.right_attr = right_attr;
            }
            if (left_attr.extype == id && right_attr.extype == id) {
                condition1.right_is_attr = 1;
                condition1.left_is_attr = 1;
                condition1.left_attr = left_attr;
                condition1.right_attr = right_attr;
            }
            return condition1;
        }
    }
    LOG_PANIC("Not a simple value in condition.");
    assert(0);
}
/**
 * 创建输出模式并进行校验,构造每个表的选择属性,schemas i是realtions i的投影模式,并构造flitet
 * @param selects
 * @param tupleSchema
 * @return
 */
RC create_out_schema(const char *db, Selects selects, TupleSchema &tupleSchema, TupleSchema *tupleSchemas,
                     std::vector<Condition> &t_t_conditions, int &num) {
    std::vector<std::string> table_names;
    table_names.reserve(selects.relation_num - 1);
    for (size_t i = 0; i < selects.relation_num - 1; i++) {
        table_names.emplace_back(selects.relations[i]);
    }
    //直接把所有表取出来吧
    for (int i = 0; i < selects.relation_num; i++) {
        TupleSchema tmpSchema;
        Table *table = DefaultHandler::get_default().find_table(db, selects.relations[i]);
        if (table == nullptr) {//表不存在
            return RC::SCHEMA_TABLE_NOT_EXIST;
        }
        TupleSchema::from_table(table, tmpSchema);
        tupleSchemas[i].append_if_not_exists(tmpSchema);
    }
    //下面对表达式的属性进行查询
    for (int i = 0; i < selects.attr_num; i++) {//遍历要从后往前，因为sql解析的缘故
        RelAttr &attr_top = selects.attributes[i];
        std::vector<RelAttr> rels;
        int size=0;
        bool is_simple = is_simple_ex(attr_top, rels);
        if (!is_simple) {
            tupleSchema.appned(attr_top);
        }
        for (int j = 0; j < rels.size(); j++) {
            RelAttr &attr = rels[j];
            if(attr.extype==val){
                continue;
            }
            if (nullptr == attr.relation_name) {
                if (0 == strcmp("*", attr.attribute_name)) {
                    // 列出所有查询表的所有字段 是不是也要反着来
                    for (int j = selects.relation_num - 1; j >= 0; j--) {
                        TupleSchema tmpSchema;
                        Table *table = DefaultHandler::get_default().find_table(db, selects.relations[j]);
                        if (table == nullptr) {//表不存在
                            return RC::SCHEMA_TABLE_NOT_EXIST;
                        }
                        //要为输出模式添加字段
                        TupleSchema schema;
                        TupleSchema::from_table(table, schema);
                        for (int size = 0; size < schema.fields().size(); size++) {
                            RelAttr *relAttr= (RelAttr*)malloc(sizeof(RelAttr));
                            relAttr->aggType = Null;
                            relAttr->extype = id;
                            relAttr->id=0;
                            relAttr->relation_name =(char*) malloc(sizeof(char)* strlen(schema.field(size).table_name()));
                            relAttr->attribute_name = (char*) malloc(sizeof(char)* strlen(schema.field(size).field_name()));
                            strcpy(relAttr->relation_name,schema.field(size).table_name());
                            strcpy(relAttr->attribute_name,schema.field(size).field_name());
                            tupleSchema.appned(*relAttr);
                        }
                    }

                } else {
                    if (selects.relation_num != 1) {
                        return RC::MISMATCH;
                    } else {
                        Table *table = DefaultHandler::get_default().find_table(db, selects.relations[0]);
                        //列出表的相关字段不去重
                        int id = tupleSchemas[0].index_of_field(table->name(), attr.attribute_name);
                        attr.relation_name=selects.relations[0];
                        if (id < 0) {
                            return RC::MISMATCH;
                        }
                        if (is_simple) {
                            tupleSchema.appned(attr);
                        }

                    }
                }
            } else {
                TupleSchema tmpSchema;
                Table *table = DefaultHandler::get_default().find_table(db, attr.relation_name);
                //要在后面的relationom里 这里是校验
                size_t j;
                for (j = 0; j < selects.relation_num; j++) {
                    if (strcmp(attr.relation_name, selects.relations[j]) == 0) {
                        break;
                    }
                }
                if (j == selects.relation_num) {//在后面没找到
                    return RC::SCHEMA_TABLE_NOT_EXIST;//虽然这个错误不太标准
                }
                if (0 == strcmp("*", attr.attribute_name)) {
                    //要为输出模式添加字段
                    TupleSchema schema;
                    TupleSchema::from_table(table, schema);
                    for (int size = 0; size < schema.fields().size(); size++) {
                        RelAttr *relAttr= (RelAttr*)malloc(sizeof(RelAttr));
                        relAttr->aggType = Null;
                        relAttr->extype = id;
                        relAttr->id=0;
                        relAttr->relation_name =(char*) malloc(sizeof(char)* strlen(schema.field(size).table_name()));
                        relAttr->attribute_name = (char*) malloc(sizeof(char)* strlen(schema.field(size).field_name()));
                        strcpy(relAttr->relation_name,schema.field(size).table_name());
                        strcpy(relAttr->attribute_name,schema.field(size).field_name());
                        tupleSchema.appned(*relAttr);
                    }
                } else {
                    //列出表的相关字段不去重
                    int id = tupleSchemas[j].index_of_field(attr.relation_name, attr.attribute_name);
                    if (id < 0) {
                        return RC::MISMATCH;
                    }
                    if (is_simple) {
                        tupleSchema.appned(attr);
                    }
                }
            }
        }
    }
    std::vector<DefaultConditionFilter *> condition_filters;
    for (size_t i = 0; i < selects.condition_num; i++) {
        const Condition &condition = selects.conditions[i];//conditon必须在from里面 对conditio进行检查，返回表之间的联系以及表达式
        std::vector<RelAttr> lefts;
        std::vector<RelAttr> rights;
        bool is_simple = is_simple_ex(condition.left_attr, lefts) && is_simple_ex(condition.right_attr, rights);
        //TODO 检查所有的属性都在表里面

        if (is_simple) {//如果是简单表达式则判断是不是两表的比较，并进行校验
            RelAttr left_attr = lefts[0];
            RelAttr right_attr = rights[0];
            if (left_attr.extype == id && right_attr.extype == id &&
                strcmp(left_attr.relation_name, right_attr.relation_name) != 0) {//表之间联系
                //要找到对应的表位置与，属性位置
                //先找左边
                AttrType left;
                AttrType right;
                int count = 0;
                for (int j = 0; j < selects.relation_num; j++) {
                    if (strcmp(left_attr.relation_name, selects.relations[j]) == 0) {

                        int id = tupleSchemas[j].index_of_field(left_attr.relation_name,
                                                                left_attr.attribute_name);
                        if (id == -1) {
                            Table *table = DefaultHandler::get_default().find_table(db,
                                                                                    left_attr.relation_name);
                            RC rc = schema_add_field(table,left_attr.attribute_name, tupleSchemas[j]);
                            if (rc != RC::SUCCESS) {
                                return RC::SCHEMA_FIELD_MISSING;
                            }
                        }
                        id = tupleSchemas[j].index_of_field(left_attr.relation_name,
                                                            left_attr.attribute_name);

                        left = tupleSchemas[j].field(id).type();
                        count++;
                    }
                    if (strcmp(right_attr.relation_name, selects.relations[j]) == 0) {

                        int id = tupleSchemas[j].index_of_field(right_attr.relation_name,
                                                                right_attr.attribute_name);
                        if (id == -1) {
                            Table *table = DefaultHandler::get_default().find_table(db,
                                                                                    right_attr.relation_name);
                            RC rc = schema_add_field(table, right_attr.attribute_name, tupleSchemas[j]);
                            if (rc != RC::SUCCESS) {
                                return RC::SCHEMA_FIELD_MISSING;
                            }
                        }
                        id = tupleSchemas[j].index_of_field(right_attr.relation_name,
                                                            right_attr.attribute_name);

                        right = tupleSchemas[j].field(id).type();
                        count++;
                    }
                }
                //进行左右比较
                if (count != 2) {
                    return RC::MISMATCH;
                }
                if (right != left) {
                    if ((right == AttrType::FLOATS && left == AttrType::INTS) ||
                        (left == AttrType::FLOATS && right == AttrType::INTS)) {
                    } else {
                        return RC::MISMATCH;
                    }
                }
                num++;
                t_t_conditions.push_back(condition);
            }
        } else {
            //对表达式的属性进行校验，现不做，看看测试怎么说
            t_t_conditions.push_back(condition);
        }
    }

    return RC::SUCCESS;
}

bool condition_match(Condition &a, Condition &b, Condition &ret) {
    char *name;
    char *attr;
    Value value;
    ret.right_is_attr = 0;
    ret.left_is_attr = 1;
    ret.comp = a.comp;
    if (a.right_is_attr == 1) {
        name = a.right_attr.relation_name;
        attr = a.right_attr.attribute_name;
        value = a.left_value;
    } else {
        name = a.left_attr.relation_name;
        attr = a.left_attr.attribute_name;
        value = a.right_value;
    }
    ret.right_value = value;
    if (b.left_is_attr == 1 && b.right_is_attr == 1 && b.comp == EQUAL_TO &&
        (strcmp(b.left_attr.relation_name, b.right_attr.relation_name) != 0)) {
        if (strcmp(name, b.left_attr.relation_name) == 0 && strcmp(attr, b.left_attr.attribute_name) == 0) {//
            ret.left_attr = b.right_attr;
            return true;
        } else if (strcmp(name, b.right_attr.relation_name) == 0 && strcmp(attr, b.right_attr.attribute_name) == 0) {
            ret.left_attr = b.left_attr;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

/**
 * 基数估计
 * @param a
 * @param b
 * @param conditions
 * @return
 */
int cal_card(TupleSet &a, TupleSet &b, std::vector<Condition> conditions) {
    //查找连接条件
    int ret = a.size() * b.size();
    if (std::max(a.size(), b.size()) == 0) {
        return 0;
    }
    for (int i = 0; i < conditions.size(); i++) {
        Condition condition = conditions[i];
        int left_id;
        int right_id;
        if ((left_id = a.get_schema().index_of_field(condition.left_attr.relation_name,
                                                     condition.left_attr.attribute_name)) != -1 &&
            (right_id = b.get_schema().index_of_field(condition.right_attr.relation_name,
                                                      condition.right_attr.attribute_name)) != -1) {
            if (condition.comp == EQUAL_TO) {
                ret = ret / std::max(a.size(), b.size());
            }
        } else if ((right_id = b.get_schema().index_of_field(condition.left_attr.relation_name,
                                                             condition.left_attr.attribute_name)) != -1 &&
                   (left_id = a.get_schema().index_of_field(condition.right_attr.relation_name,
                                                            condition.right_attr.attribute_name)) != -1) {
            if (condition.comp == EQUAL_TO) {
                ret = ret / std::max(a.size(), b.size());
            }
        }
    }
    return ret;
}
void set_table_name(RelAttr* relAttr,char *tablename){
    switch (relAttr->extype) {
        case E:{

        }
        case T:{
            set_table_name(relAttr->first,tablename);
            if(relAttr->second!= nullptr){
                set_table_name(relAttr->second,tablename);
            }
            break;
        }
        case F:{
            set_table_name(relAttr->first,tablename);
            break;
        }
        case id:{
            relAttr->relation_name=tablename;
            break;
        }
        case val:{
            break;
        }

    }
}

RC ExecuteStage::sub_select(Selects &selects, TupleSet &ret, Trx *trx) {
    RC rc = RC::SUCCESS;
    TupleSchema schemas[selects.relation_num];//每个表的模式
    int order_size = selects.order_num;
    TupleSchema out_schema;
    std::vector<Condition> t_t_conditions;
    struct filter filters[20];
    int filter_num = 0;//filters的个数
    //构造condition
    int condition_size = selects.condition_num;
    bool sign[MAX_NUM];
    memset(sign, 0, sizeof(bool) * MAX_NUM);
//    for (int i = 0; i < selects.condition_num; i++) {//这个地方是为了连接筛选，不重要
//        Condition condition = selects.conditions[i];
//        if ((condition.left_is_attr == 1 && condition.right_is_attr == 0) ||
//            (condition.left_is_attr == 0 && condition.right_is_attr == 1)) {
//            for (int j = 0; j < condition_size; j++) {
//                if (i != j && sign[j] == false) {
//                    Condition condition1 = selects.conditions[j];
//                    Condition add;
//                    if (condition_match(condition, condition1, add)) {
//                        selects.conditions[selects.condition_num++] = add;
//                        sign[j] = true;
//                    }
//                }
//            }
//        }
//    }
    const char *db = "sys";
    //构造最后的输出schema与各表查询的schema,以及构建表间的比较
    //应该构造新的filter来减少元组
    if ((rc = create_out_schema(db, selects, out_schema, schemas, t_t_conditions, filter_num)) != RC::SUCCESS) {
        const char *failure_ptr = "FAILURE\n";//这种地方复制有点冗余
        return rc;
    }
    // 把所有的表和只跟这张表关联的condition都拿出来，生成最底层的select 执行节点
    std::vector<SelectExeNode *> select_nodes;
    for (size_t i = 0; i < selects.relation_num; i++) {
        const char *table_name = selects.relations[i];
        SelectExeNode *select_node = new SelectExeNode;
        rc = create_selection_executor(trx, selects, db, table_name, *select_node, schemas[i]);
        if (rc != RC::SUCCESS) {
            delete select_node;
            for (SelectExeNode *&tmp_node: select_nodes) {
                delete tmp_node;
            }
            const char *failure_ptr = "FAILURE\n";
            return rc;
        }
        select_nodes.push_back(select_node);
    }

    if (select_nodes.empty()) {
        LOG_ERROR("No table given");
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
            return rc;
        } else {
            tuple_sets.push_back(std::move(tuple_set));
        }
    }
    //如果是但表，则set以下tablename;
    if(tuple_sets.size()==1){
        for(int i=0;i<t_t_conditions.size();i++){
            set_table_name(&t_t_conditions[i].left_attr,selects.relations[0]);
            set_table_name(&t_t_conditions[i].right_attr,selects.relations[0]);
        }
        for(int i=0;i<out_schema.get_col_number();i++){
            set_table_name(out_schema.getRel(i),selects.relations[0]);
        }
        //给加一个一个元组的表
        TupleSet set;
        Tuple tuple;
        tuple.add(1, false);
        set.add(std::move(tuple));
        tuple_sets.emplace_back(std::move(set));
    }
    TupleSet tupleSet;

    std::stringstream ss;
    int old_tuple_set_szie = tuple_sets.size();
    while (tuple_sets.size() > 1) {//表连接
        int min_card = tuple_sets[0].size() * tuple_sets[1].size();
        std::vector<TupleSet>::iterator min_ip = tuple_sets.begin();
        std::vector<TupleSet>::iterator min_jp = min_ip + 1;
//        //选取连接的表
//        for (std::vector<TupleSet>::iterator i = tuple_sets.begin(); i != tuple_sets.end(); i++) {
//            for (std::vector<TupleSet>::iterator j = i + 1; j != tuple_sets.end(); j++) {
//                int tmp = cal_card(*i, *j, t_t_conditions);//
//                if (tmp < min_card) {
//                    min_card = tmp;
//                    min_ip = i;
//                    min_jp = j;
//                }
//            }
//        }
        TupleSet tmpSet;
        TupleSchema schema;
        schema.append((*min_ip).get_schema());
        schema.append(min_jp->get_schema());
        tmpSet.set_schema(schema);
        min_ip->join(*min_jp, tmpSet, t_t_conditions);
        tuple_sets.erase(min_jp);
        tuple_sets.erase(min_ip);
        tuple_sets.push_back(std::move(tmpSet));//这个地方可能因为临时变量出问题
    }
    TupleSet &join_tupleset = tuple_sets.front();

    //先在连接结果排序
    if (selects.order_num != 0) {
        Tuple::clear();
        for (int i = selects.order_num - 1; i >= 0; i--) {
            RelAttr attr = selects.orders[i];
            int id;
            if (attr.relation_name == nullptr) {
                id = join_tupleset.get_schema().index_of_field(selects.relations[0], attr.attribute_name);

            } else {
                id = join_tupleset.get_schema().index_of_field(attr.relation_name, attr.attribute_name);
            }
            if (id != -1) {
                Tuple::append_order_attr(id, selects.ascs[i]);
            } else {
                return RC::SCHEMA_FIELD_MISSING;//虽然错误并不是如此
            }
        }
        for (int i = 0; i < join_tupleset.size(); i++) {
            int min = i;
            for (int j = i + 1; j < join_tupleset.size(); j++) {
                if (!comp(join_tupleset.get(min), join_tupleset.get(j))) {
                    min = j;
                }
            }
            join_tupleset.swap(i, min);
        }
    }
    //处理聚合查询的输出
    bool has_agg = false;
    for (int i = 0; i < selects.attr_num; i++) {
        if (selects.attributes[i].aggType != Null) {
            has_agg = true;
        }
    }
    if (has_agg) { //TODO 这个地方可能在分组时出错
        //建立新的schema
        TupleSchema agg_schema;
        for (int i = 0; i < selects.attr_num; i++) {
            AttrType attrType;
            std::vector<RelAttr> attrs;
            RelAttr attr;
            is_simple_ex(selects.attributes[i],attrs);
            attr=attrs[0];
            if (strcmp(attr.attribute_name, "*") == 0 || attr.aggType == Count) {
                attrType = INTS;
            } else {
                if (attr.relation_name == nullptr) {
                    Table *table = DefaultHandler::get_default().find_table(db, selects.relations[0]);
                    attrType = table->table_meta().field(attr.attribute_name)->type();
                } else {
                    Table *table = DefaultHandler::get_default().find_table(db, attr.relation_name);
                    attrType = table->table_meta().field(attr.attribute_name)->type();
                }

            }
            if (attr.aggType == Avg) {
                attrType = FLOATS;
            }
            if (attr.relation_name == nullptr) {
                agg_schema.add_agg(attrType, selects.relations[0], attr.attribute_name,
                                   false, attr.aggType);
            } else {
                agg_schema.add_agg(attrType, attr.relation_name, attr.attribute_name,
                                   false, attr.aggType);
            }
        }
        ret.set_schema(agg_schema);
        //处理排序
        std::vector<std::tuple<int, int>> groups;
        if (selects.order_num != 0) {

            int begin = 0;
            int end = 0;
            for (int i = 1; i < join_tupleset.size(); i++) {//TODO 这里要处理最后一组
                Tuple *a = join_tupleset.get(i - 1);
                Tuple *b = join_tupleset.get(i);
                if ((*a) == (*b)) {
                    end++;
                } else {
                    std::tuple<int, int> range = std::make_tuple(begin, end);
                    groups.emplace_back(range);
                    begin = i;
                    end = begin;
                }
            }
            if (join_tupleset.size() != 0) {
                std::tuple<int, int> range = std::make_tuple(begin, end);
                groups.emplace_back(range);
            }
        } else {
            if (join_tupleset.size() != 0) {
                std::tuple<int, int> range = std::make_tuple(0, join_tupleset.size() - 1);
                groups.emplace_back(range);
            }

        }
        for (int i = 0; i < groups.size(); i++) {
            cal_agg(join_tupleset, ret, std::get<0>(groups.at(i)), std::get<1>(groups.at(i)));
        }
    } else {
        //没有聚合运算直接复制过去
        ret.set_schema(out_schema);
        //将连接结果映射到输出结果 TODO最后映射比较好
        for (int i = 0; i < join_tupleset.size(); i++) {
            Tuple *tuple = join_tupleset.get(i);
            TupleSchema schema;
            schema.append(join_tupleset.schema());
            Tuple addtuple;
            std::vector<std::shared_ptr<TupleValue>> values;
            for ( int j = 0; j < out_schema.relfields().size(); j++) {
                RelAttr attr=*out_schema.getRel(j);
                int success=0;
                std::shared_ptr<TupleValue> tupleValue;
                double value=getvalue(tuple,&schema,attr,&success,tupleValue);
                if(success==0){
                    values.emplace_back(tupleValue);
                }
                else{
                    values.emplace_back(new FloatValue(value, false));
                };
            }
            addtuple.add(values);
            ret.add(std::move(addtuple));
        }
    }
    for (SelectExeNode *&tmp_node: select_nodes) {
        delete tmp_node;
    }
    return rc;
}

// 这里没有对输入的某些信息做合法性校验，比如查询的列名、where条件中的列名等，没有做必要的合法性校验
// 需要补充上这一部分. 校验部分也可以放在resolve，不过跟execution放一起也没有关系
RC ExecuteStage::do_select(const char *db, Query *sql, SessionEvent *session_event) {

    RC rc = RC::SUCCESS;
    Session *session = session_event->get_client()->session;
    Trx *trx = session->current_trx();
    TupleSet outTupleset;
    Selects &selects = sql->sstr.selection;
    //构造最后的输出schema与各表查询的schema,以及构建表间的比较
    //应该构造新的filter来减少元组
    if ((rc = sub_select(selects, outTupleset, trx)) != RC::SUCCESS) {
        const char *failure_ptr = "FAILURE\n";//这种地方复制有点冗余
        session_event->set_response(failure_ptr);
        end_trx_if_need(session, trx, false);
        return rc;
    }
    //成功选择则输出
    // 把所有的表和只跟这张表关联的condition都拿出来，生成最底层的select 执行节点
    std::stringstream ss;
    if (selects.relation_num > 1) {
        outTupleset.print_with_table(ss);
    } else {
        outTupleset.print(ss);
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
    schema.add_if_not_exists(field_meta->type(), table->name(), field_meta->name(), field_meta->nullable());
    return RC::SUCCESS;
}

static RC schema_add_field_except_exist(Table *table, const char *field_name, TupleSchema &schema) {
    const FieldMeta *field_meta = table->table_meta().field(field_name);
    if (nullptr == field_meta) {
        LOG_WARN("No such field. %s.%s", table->name(), field_name);
        return RC::SCHEMA_FIELD_MISSING;
    }

    schema.add(field_meta->type(), table->name(), field_meta->name(), field_meta->nullable());
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

    for (int i = 0; i < selects.attr_num; i++) {
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
            RC rc = condition_filter->init(*table, condition, trx);
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
        std::vector<RelAttr> lefts;
        std::vector<RelAttr> rights;
        bool is_simple = is_simple_ex(condition.left_attr, lefts) && is_simple_ex(condition.right_attr, rights);
        if (is_simple) {
            RelAttr left_attr = lefts[0];
            RelAttr right_attr = rights[0];
            if ((left_attr.extype == val && right_attr.extype == val) || // 两边都是值
                (left_attr.extype == id && right_attr.extype == val &&
                 match_table(selects, left_attr.relation_name, table_name)) ||  // 左边是属性右边是值
                (left_attr.extype == val && right_attr.extype == id &&
                 match_table(selects, right_attr.relation_name, table_name)) ||  // 左边是值，右边是属性名
                (left_attr.extype == id && right_attr.extype == id &&
                 match_table(selects, left_attr.relation_name, table_name) &&
                 match_table(selects, right_attr.relation_name, table_name)) // 左右都是属性名，并且表名都符合
                    ) {
                DefaultConditionFilter *condition_filter = new DefaultConditionFilter();
                Condition condition1;
                condition1.comp = condition.comp;//
                if ((left_attr.extype == val && right_attr.extype == val)) {
                    condition1.right_is_attr = 0;
                    condition1.left_is_attr = 0;
                    condition1.right_value = right_attr.value;
                    condition1.left_value = left_attr.value;

                }
                if ((left_attr.extype == id && right_attr.extype == val)) {
                    condition1.right_is_attr = 0;
                    condition1.left_is_attr = 1;
                    condition1.right_value = right_attr.value;
                    condition1.left_attr = left_attr;
                }
                if ((left_attr.extype == val && right_attr.extype == id)) {
                    condition1.right_is_attr = 1;
                    condition1.left_is_attr = 0;
                    condition1.left_value = left_attr.value;
                    condition1.right_attr = right_attr;
                }
                if (left_attr.extype == id && right_attr.extype == id) {
                    condition1.right_is_attr = 1;
                    condition1.left_is_attr = 1;
                    condition1.left_attr = left_attr;
                    condition1.right_attr = right_attr;
                }
                RC rc = condition_filter->init(*table, condition1, trx);
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

    }
    return select_node.init(trx, table, std::move(tupleSchema), std::move(condition_filters));
}

/**
 * 只是把一个值输出出来
 * @param tupleSet
 * @param id
 * @param aggType
 * @param os
 */
void cal_agg(TupleSet &tupleSet, TupleSet &ret, int begin, int end) {
    Tuple tuple;
    for (int i = 0; i < ret.get_schema().fields().size(); i++) {
        TupleField tupleField = ret.get_schema().field(i);
        AggType aggType = tupleField.agg_type();
        int id = tupleSet.get_schema().index_of_field(tupleField.table_name(), tupleField.field_name());
        switch (aggType) {
            case Max: {
                int max = begin;
                for (int i = begin; i <= end; i++) {
                    if (tupleSet.get(i)->get(id).is_null()) {
                    } else {
                        begin = i;
                        break;
                    }
                }
                for (int i = begin; i <= end; i++) {
                    if (tupleSet.get(i)->get(id).is_null()) {

                    } else {
                        if (tupleSet.get(i)->get(id).compare(tupleSet.get(max)->get(id)) > 0) {
                            max = i;
                        }
                    }

                }
                std::vector<std::shared_ptr<TupleValue>> other;
                other.emplace_back(tupleSet.get(max)->values().at(id));
                tuple.add(other);
            }
                break;
            case Min: {
                int min = begin;
                for (int i = begin; i <= end; i++) {
                    if (tupleSet.get(i)->get(id).is_null()) {
                    } else {
                        begin = i;
                        break;
                    }
                }

                for (int i = begin; i <= end; i++) {
                    if (tupleSet.get(i)->get(id).is_null()) {

                    } else {
                        if (tupleSet.get(i)->get(id).compare(tupleSet.get(min)->get(id)) < 0) {
                            min = i;
                        }
                    }

                }
                std::vector<std::shared_ptr<TupleValue>> other;
                other.emplace_back(tupleSet.get(min)->values().at(id));
                tuple.add(other);
            }
                break;
            case Count: {
                if (strcmp(tupleField.field_name(), "*") == 0) {
                    tuple.add(end - begin + 1, false);
                } else {
                    //这里处理Null
                    int count = 0;
                    for (int i = begin; i <= end; i++) {
                        if (tupleSet.get(i)->get(id).is_null()) {

                        } else {
                            count++;
                        }

                    }
                    tuple.add(count, false);
                }
            }
                break;
            case Avg: {//这个只在int float中出现 TODO 如何处理null
                float avg = 0;
                int size = 0;
                if (tupleSet.get_schema().field(id).type() == FLOATS) {
                    for (int i = begin; i <= end; i++) {
                        if (tupleSet.get(i)->get(id).is_null()) {

                        } else {
                            avg += ((const FloatValue &) tupleSet.get(i)->get(id)).getFValue();
                            size++;
                        }

                    }
                } else if (tupleSet.get_schema().field(id).type() == INTS) {
                    for (int i = begin; i <= end; i++) {
                        if (tupleSet.get(i)->get(id).is_null()) {

                        } else {
                            avg += ((const IntValue &) tupleSet.get(i)->get(id)).getIValue();
                            size++;
                        }
                    }
                }
                if (size == 0) {
                    tuple.add(1, true);
                } else {
                    float ret = avg * 1.0 / size;
                    tuple.add(ret, false);
                }

                break;
            }
            case Null: {
                std::vector<std::shared_ptr<TupleValue>> other;
                other.emplace_back(tupleSet.get(begin)->values().at(id));
                tuple.add(other);
            }
                break;
        }
    }
    ret.add(std::move(tuple));
}
