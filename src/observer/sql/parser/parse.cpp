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

#include <mutex>
#include "sql/parser/parse.h"
#include "rc.h"
#include "common/log/log.h"
#include <vector>

RC parse(char *st, Query *sqln);

std::vector<RelAttr *> stack;
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
void set_sub(){
    assert(stack.size() >= 1);
    stack[stack.size()-1]->num=-1;
}
void e_e_t(CalOp op) {
    assert(stack.size() >= 2);
    RelAttr *e = (RelAttr *) calloc(1, sizeof(RelAttr));
    e->extype = E;
    e->op = op;
    e->num=1;
    e->first = stack[stack.size() - 2];
    e->second = stack[stack.size() - 1];
    e->relation_name= nullptr;
    e->attribute_name= nullptr;
    stack.pop_back();
    stack.pop_back();
    stack.emplace_back(e);
}
void e_t() {
    assert(stack.size() >= 1);
    RelAttr *e = (RelAttr *) calloc(1, sizeof(RelAttr));
    e->extype = E;
    e->first = stack[stack.size() - 1];
    e->second = nullptr;
    e->op = None;
    e->num=1;
    e->relation_name= nullptr;
    e->attribute_name= nullptr;
    stack.pop_back();
    stack.emplace_back(e);
}
void t_t_f(CalOp op) {
    assert(stack.size() >= 2);
    RelAttr *e = (RelAttr *) calloc(1, sizeof(RelAttr));
    e->extype = T;
    e->op=op;
    e->first = stack[stack.size() - 2];
    e->second = stack[stack.size() - 1];
    e->relation_name= nullptr;
    e->attribute_name= nullptr;
    e->num=1;
    stack.pop_back();
    stack.pop_back();
    stack.emplace_back(e);
}
void t_f() {
    assert(stack.size() >= 1);
    RelAttr *e = (RelAttr *) calloc(1, sizeof(RelAttr));
    e->extype = T;
    e->first = stack[stack.size() - 1];
    e->second = nullptr;
    e->op = None;
    e->relation_name= nullptr;
    e->attribute_name= nullptr;
    e->num=1;
    stack.pop_back();
    stack.emplace_back(e);
}
void f_e() {
    assert(stack.size() >= 1);
    RelAttr *e = (RelAttr *) calloc(1, sizeof(RelAttr));
    e->extype = F;
    e->first = stack[stack.size() - 1];
    e->second = nullptr;
    e->op = None;
    e->num=1;
    e->relation_name= nullptr;
    e->attribute_name= nullptr;
    stack.pop_back();
    stack.emplace_back(e);
}
void f_id(Extype extype) {
    assert(stack.size() >= 1);
    RelAttr *e = (RelAttr *) calloc(1, sizeof(RelAttr));
    e->extype = extype;
    e->first = stack[stack.size() - 1];
    e->second = nullptr;
    e->op = None;
    e->relation_name= nullptr;
    e->attribute_name= nullptr;
    stack.pop_back();
    stack.emplace_back(e);
}
void relation_value_append(Value *value) {
    RelAttr *e = (RelAttr *) calloc(1, sizeof(RelAttr));
    e->extype = val;
    e->value=*value;
    e->first = nullptr;
    e->second = nullptr;
    e->num=1;
    e->relation_name= nullptr;
    e->attribute_name= nullptr;
    e->op = None;
    stack.emplace_back(e);
}
void selects_append_attribute_plus(Selects *selects) {
    assert(stack.size() >= 1);
    selects->attributes[selects->attr_num++] = *stack[stack.size()-1];
    stack.pop_back();
}
void relation_attr_get(RelAttr *attr){
    assert(stack.size() >= 1);
    attr=stack[stack.size()-1];
    stack.pop_back();
}
void relation_attr_init(RelAttr *relation_attr, const char *relation_name, const char *attribute_name) {
    if (relation_name != nullptr) {
        relation_attr->relation_name = strdup(relation_name);
    } else {
        relation_attr->relation_name = nullptr;
    }
    relation_attr->aggType = Null;
    relation_attr->attribute_name = strdup(attribute_name);
    RelAttr *e = (RelAttr *) calloc(1, sizeof(RelAttr));
    e->extype=id;
    e->id=0;
    e->num=1;
    e->relation_name=relation_attr->relation_name;
    e->attribute_name=relation_attr->attribute_name;
    e->aggType=Null;
    stack.emplace_back(e);
}
void relation_agg_attr_init(RelAttr *relation_attr, const char *relation_name, const char *attribute_name, AggType aggType) {
    if (relation_name != nullptr) {
        relation_attr->relation_name = strdup(relation_name);
    } else {
        relation_attr->relation_name = nullptr;
    }
    relation_attr->extype=id;
    relation_attr->id=0;
    relation_attr->attribute_name = strdup(attribute_name);
    relation_attr->aggType = aggType;
}
void relation_attr_destroy(RelAttr *relation_attr) {
    free(relation_attr->relation_name);
    free(relation_attr->attribute_name);
    relation_attr->relation_name = nullptr;
    relation_attr->attribute_name = nullptr;
}

void value_init_integer(Value *value, int v) {
    value->type = INTS;
    value->data = calloc(1, sizeof(v));
    memcpy(value->data, &v, sizeof(v));
}
void value_init_float(Value *value, float v) {
    value->type = FLOATS;
    value->data = calloc(1, sizeof(v));
    memcpy(value->data, &v, sizeof(v));
}
void value_init_string(Value *value, const char *v) {
    value->type = CHARS;
    value->data = strdup(v);
}
void value_init_text(Value *value, const char *v) {
    value->type = TEXTS;
    value->data = strdup(v);
}
void value_init_null(Value *value) {
    value->type = UNDEFINED;
    value->null_attr = true;
    value->data = nullptr;
}
void value_destroy(Value *value) {
    value->type = UNDEFINED;
    free(value->data);
    value->data = nullptr;
}
void condition_init_ex(Condition* condition,CompOp op){
    condition->left_is_attr=-1;
    condition->right_is_attr=-1;
    condition->left_attr=*stack.at(stack.size()-2);
    condition->right_attr=*stack.at(stack.size()-1);
    condition->comp = op;
    stack.pop_back();
    stack.pop_back();
}
void condition_init(Condition *condition, CompOp comp,
                    int left_is_attr, RelAttr *left_attr, Value *left_value,
                    int right_is_attr, RelAttr *right_attr, Value *right_value) {
    condition->comp = comp;
    condition->left_is_attr = left_is_attr;
    if (left_is_attr) {
        condition->left_attr = *stack[stack.size()-1];
        stack.pop_back();
    } else {
        condition->left_value = *left_value;
    }

    condition->right_is_attr = right_is_attr;
    if (right_is_attr) {
        condition->right_attr = *stack[stack.size()-1];
        stack.pop_back();
    } else {
        condition->right_value = *right_value;
    }
}
void condition_destroy(Condition *condition) {
    if (condition->left_is_attr) {
        relation_attr_destroy(&condition->left_attr);
    } else {
        value_destroy(&condition->left_value);
    }
    if (condition->right_is_attr) {
        relation_attr_destroy(&condition->right_attr);
    } else {
        value_destroy(&condition->right_value);
    }
}

void attr_info_init(AttrInfo *attr_info, const char *name, AttrType type, size_t length, bool nullable) {
    attr_info->name = strdup(name);
    attr_info->type = type;
    attr_info->length = length;
    attr_info->nullable = nullable;
}

void attr_info_destroy(AttrInfo *attr_info) {
    free(attr_info->name);
    attr_info->name = nullptr;
}

void selects_init(Selects *selects, ...);
void selects_append_attribute(Selects *selects, RelAttr *rel_attr) {
    selects->attributes[selects->attr_num++] = *rel_attr;
}
void selects_append_orders(Selects *selects, RelAttr *rel_attr, int asc) {
    selects->orders[selects->order_num] = *rel_attr;
    selects->ascs[selects->order_num] = asc;
    selects->order_num++;
}
void selects_set_order(Selects *selects, int order) {
    selects->asc = order;
}
void selects_append_relation(Selects *selects, const char *relation_name) {
    for (size_t i = 0; i < selects->relation_num; i++) {//检查是否重复
        if (strcmp(selects->relations[i], relation_name) == 0) {
            return;
        }
    }
    selects->relations[selects->relation_num++] = strdup(relation_name);
}

void selects_append_conditions(Selects *selects, Condition conditions[], size_t condition_num) {
    assert(condition_num <= sizeof(selects->conditions) / sizeof(selects->conditions[0]));
    for (size_t i = 0; i < condition_num; i++) {
        selects->conditions[i] = conditions[i];
    }
    selects->condition_num = condition_num;
}

void selects_destroy(Selects *selects) {
    for (size_t i = 0; i < selects->attr_num; i++) {
        relation_attr_destroy(&selects->attributes[i]);
    }
    selects->attr_num = 0;

    for (size_t i = 0; i < selects->relation_num; i++) {
        free(selects->relations[i]);
        selects->relations[i] = nullptr;
    }
    selects->relation_num = 0;

    for (size_t i = 0; i < selects->condition_num; i++) {
        condition_destroy(&selects->conditions[i]);
    }
    selects->condition_num = 0;

    if (selects->sub_selection != nullptr) {
        // 有子SELECT查询，递归销毁子查询
        selects_destroy(selects->sub_selection);
    }
}

void inserts_init(Inserts *inserts, const char *relation_name, LexTuple tuples[], size_t tuple_count) {
    for (size_t i = 0; i < tuple_count; ++i) {
        assert(tuples[i].count <= sizeof(inserts->tuples) / sizeof(inserts->tuples[0]));
    }

    inserts->relation_name = strdup(relation_name);
    for (size_t i = 0; i < tuple_count; i++) {
        inserts->tuples[i] = tuples[i];
    }
    inserts->tuple_count = tuple_count;
}
void inserts_destroy(Inserts *inserts) {
    free(inserts->relation_name);
    inserts->relation_name = nullptr;

    for (size_t i = 0; i < inserts->tuple_count; i++) {
        for (size_t j = 0; j < inserts->tuples->count; ++j) {
            value_destroy(&inserts->tuples->values[j]);
        }
    }
    inserts->tuple_count = 0;
}

void deletes_init_relation(Deletes *deletes, const char *relation_name) {
    deletes->relation_name = strdup(relation_name);
}

void deletes_set_conditions(Deletes *deletes, Condition conditions[], size_t condition_num) {
    assert(condition_num <= sizeof(deletes->conditions) / sizeof(deletes->conditions[0]));
    for (size_t i = 0; i < condition_num; i++) {
        deletes->conditions[i] = conditions[i];
    }
    deletes->condition_num = condition_num;
}
void deletes_destroy(Deletes *deletes) {
    for (size_t i = 0; i < deletes->condition_num; i++) {
        condition_destroy(&deletes->conditions[i]);
    }
    deletes->condition_num = 0;
    free(deletes->relation_name);
    deletes->relation_name = nullptr;
}

void updates_init(Updates *updates, const char *relation_name, const char *attribute_name,
                  Value *value, Condition conditions[], size_t condition_num) {
    updates->relation_name = strdup(relation_name);
    updates->attribute_name = strdup(attribute_name);
    updates->value = *value;

    assert(condition_num <= sizeof(updates->conditions) / sizeof(updates->conditions[0]));
    for (size_t i = 0; i < condition_num; i++) {
        updates->conditions[i] = conditions[i];
    }
    updates->condition_num = condition_num;
}

void updates_destroy(Updates *updates) {
    free(updates->relation_name);
    free(updates->attribute_name);
    updates->relation_name = nullptr;
    updates->attribute_name = nullptr;

    value_destroy(&updates->value);

    for (size_t i = 0; i < updates->condition_num; i++) {
        condition_destroy(&updates->conditions[i]);
    }
    updates->condition_num = 0;
}

void create_table_append_attribute(CreateTable *create_table, AttrInfo *attr_info) {
    create_table->attributes[create_table->attribute_count++] = *attr_info;
}
void create_table_init_name(CreateTable *create_table, const char *relation_name) {
    create_table->relation_name = strdup(relation_name);
}
void create_table_destroy(CreateTable *create_table) {
    for (size_t i = 0; i < create_table->attribute_count; i++) {
        attr_info_destroy(&create_table->attributes[i]);
    }
    create_table->attribute_count = 0;
    free(create_table->relation_name);
    create_table->relation_name = nullptr;
}

void drop_table_init(DropTable *drop_table, const char *relation_name) {
    drop_table->relation_name = strdup(relation_name);
}
void drop_table_destroy(DropTable *drop_table) {
    free(drop_table->relation_name);
    drop_table->relation_name = nullptr;
}

void create_index_init(CreateIndex *create_index, const char *index_name,
                       const char *relation_name, const char *attr_name, bool unique_attr) {
    create_index->index_name = strdup(index_name);
    create_index->relation_name = strdup(relation_name);
    create_index->attribute_name = strdup(attr_name);
    create_index->unique_attr = unique_attr;
}
void create_index_destroy(CreateIndex *create_index) {
    free(create_index->index_name);
    free(create_index->relation_name);
    free(create_index->attribute_name);

    create_index->index_name = nullptr;
    create_index->relation_name = nullptr;
    create_index->attribute_name = nullptr;
}

void drop_index_init(DropIndex *drop_index, const char *index_name) {
    drop_index->index_name = strdup(index_name);
}
void drop_index_destroy(DropIndex *drop_index) {
    free((char *) drop_index->index_name);
    drop_index->index_name = nullptr;
}

void desc_table_init(DescTable *desc_table, const char *relation_name) {
    desc_table->relation_name = strdup(relation_name);
}

void desc_table_destroy(DescTable *desc_table) {
    free((char *) desc_table->relation_name);
    desc_table->relation_name = nullptr;
}

void load_data_init(LoadData *load_data, const char *relation_name, const char *file_name) {
    load_data->relation_name = strdup(relation_name);

    if (file_name[0] == '\'' || file_name[0] == '\"') {
        file_name++;
    }
    char *dup_file_name = strdup(file_name);
    int len = strlen(dup_file_name);
    if (dup_file_name[len - 1] == '\'' || dup_file_name[len - 1] == '\"') {
        dup_file_name[len - 1] = 0;
    }
    load_data->file_name = dup_file_name;
}

void load_data_destroy(LoadData *load_data) {
    free((char *) load_data->relation_name);
    free((char *) load_data->file_name);
    load_data->relation_name = nullptr;
    load_data->file_name = nullptr;
}

void query_init(Query *query) {
    query->flag = SCF_ERROR;
    memset(&query->sstr, 0, sizeof(query->sstr));
}

Query *query_create() {
    auto *query = (Query *) calloc(1, sizeof(Query));
    if (query == nullptr) {
        LOG_ERROR("Failed to alloc memroy for query. size=%ld", sizeof(Query));
        return nullptr;
    }

    query_init(query);
    return query;
}

void query_reset(Query *query) {
    switch (query->flag) {
        case SCF_SELECT: {
            selects_destroy(&query->sstr.selection);
        }
            break;
        case SCF_INSERT: {
            inserts_destroy(&query->sstr.insertion);
        }
            break;
        case SCF_DELETE: {
            deletes_destroy(&query->sstr.deletion);
        }
            break;
        case SCF_UPDATE: {
            updates_destroy(&query->sstr.update);
        }
            break;
        case SCF_CREATE_TABLE: {
            create_table_destroy(&query->sstr.create_table);
        }
            break;
        case SCF_DROP_TABLE: {
            drop_table_destroy(&query->sstr.drop_table);
        }
            break;
        case SCF_CREATE_INDEX: {
            create_index_destroy(&query->sstr.create_index);
        }
            break;
        case SCF_DROP_INDEX: {
            drop_index_destroy(&query->sstr.drop_index);
        }
            break;
        case SCF_SYNC: {

        }
            break;
        case SCF_SHOW_TABLES:
            break;

        case SCF_DESC_TABLE: {
            desc_table_destroy(&query->sstr.desc_table);
        }
            break;

        case SCF_LOAD_DATA: {
            load_data_destroy(&query->sstr.load_data);
        }
            break;
        case SCF_BEGIN:
        case SCF_COMMIT:
        case SCF_ROLLBACK:
        case SCF_HELP:
        case SCF_EXIT:
        case SCF_ERROR:
            break;
    }
}

void query_destroy(Query *query) {
    query_reset(query);
    free(query);
}
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////

extern "C" int sql_parse(const char *st, Query *sqls);

RC parse(const char *st, Query *sqln) {
    sql_parse(st, sqln);

    if (sqln->flag == SCF_ERROR)
        return SQL_SYNTAX;
    else
        return SUCCESS;
}