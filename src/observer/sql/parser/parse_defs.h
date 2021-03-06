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
// Created by wangyunlai.wyl on 2021/6/7.
//

#ifndef __OBSERVER_SQL_PARSER_PARSE_DEFS_H__
#define __OBSERVER_SQL_PARSER_PARSE_DEFS_H__

#include <stddef.h>
#include <stdbool.h>

#define MAX_NUM 20
#define MAX_REL_NAME 20
#define MAX_ATTR_NAME 20
#define MAX_ERROR_MESSAGE 20
#define MAX_DATA 50
typedef enum {
    Null, Max, Min, Count, Avg
} AggType;

typedef enum {
    EQUAL_TO,     //"="     0
    LESS_EQUAL,   //"<="    1
    NOT_EQUAL,    //"<>"    2
    LESS_THAN,    //"<"     3
    GREAT_EQUAL,  //">="    4
    GREAT_THAN,   //">"     5
    CONTAINED_BY, // "in"   6
    NOT_CONTAINED_BY, // "in"   7
    IS_CompOP,
    IS_NOT_CompOP,
    NO_OP
} CompOp;

//属性值类型
typedef enum {
    UNDEFINED, CHARS, INTS, FLOATS, DATE, TEXTS,
    UNEVALUATED, // 未被求值的一个抽象值或值表达式，如子SQL语句
    ATTR_TABLE, // 行优先的二维线性表，是由有限个值构成的有序线性数据结构。SQL SELECT从句的执行结果就是一个TABLE，只有一列的TABLE退化为列表
    MULTI_INDEX_FIELD //用于在B树中表示这个key是多列复合的
} AttrType;

//属性值
typedef struct _Value {
    AttrType type;  // type of value
    void *data;     // value
    bool null_attr;
} Value;

#define MAX_TUPLES_ONE_INSERTION 30

// 元组，属性值的集合，即一行数据
typedef struct s_tuple {
    size_t count; // 值的个数
    Value values[MAX_NUM];
} LexTuple;
//表达式类型
typedef enum {
    E,T,F,val,id
}Extype;
//运算符号类型
typedef enum {
    Sub, Add, Mul, Div,None
} CalOp;
//属性结构体
typedef struct _relattr{
    Extype extype;
    CalOp op;
    Value value;
    int num;//正负号
    struct _relattr* first;
    struct _relattr* second;
    int id;
    AggType aggType;
    char *relation_name;   // relation name (may be NULL) 表名
    char *attribute_name;  // attribute name              属性名
} RelAttr;
typedef struct _Condition {
    int left_is_attr;    // TRUE if left-hand side is an attribute
    // 1时，操作符左边是属性名，0时，是属性值
    Value left_value;    // left-hand side value if left_is_attr = FALSE
    RelAttr left_attr;   // left-hand side attribute
    CompOp comp;         // comparison operator
    int right_is_attr;   // TRUE if right-hand side is an attribute
    // 1时，操作符右边是属性名，0时，是属性值
    RelAttr right_attr;  // right-hand side attribute if right_is_attr = TRUE 右边的属性
    Value right_value;   // right-hand side value if right_is_attr = FALSE
} Condition;

// struct of select
typedef struct s_sql_query Selects;
typedef struct s_sql_query {
    size_t attr_num;               // Length of attrs in Select clause
    RelAttr attributes[MAX_NUM];    // attrs in Select clause
    size_t relation_num;           // Length of relations in Fro clause
    char *relations[MAX_NUM];     // relations in From clause
    size_t condition_num;          // Length of conditions in Where clause
    Condition conditions[MAX_NUM];    // conditions in Where clause
    size_t order_num;
    RelAttr orders[MAX_NUM]; // 排序的属性
    size_t ascs[MAX_NUM];//升降序
    int asc;//排序升降
    // 子查询。如果为NULL，表示没有子查询语句。在parser遇到子查询语句时创建，在selects_destroy过程中销毁
    // 仅仅在解析表达式时有用到，SQL执行时这里总是NULL，子查询语句被放到了conditions列表里，作为一个UNEVALUATED值
    Selects *sub_selection;
} Selects;

typedef enum {
    UE_SELECT // 是一个SQL SELECT语句
    // 后续如算术表达式等未求值的属性值，可以在这里继续扩充
} UE_TYPE;

// 未被求出字面值的属性值，对应于AttrType::UNEVALUATED
// 此时Value结构体的data字段即指向一个该结构体的实例
// 在参与比较或运算时，需要先对其求值以获取实际值
typedef struct {
    UE_TYPE type;
    union {
        Selects select;
        // 其他类型可以在此继续扩充，如算术表达式的信息，可以把语法树或者字符串存在这里
    } data;
} Unevaluated;

// struct of insert
typedef struct {
    char *relation_name;    // Relation to insert into
    size_t tuple_count; // how many tuples to insert
    LexTuple tuples[MAX_TUPLES_ONE_INSERTION]; // tuples to be inserted
} Inserts;

// struct of delete
typedef struct {
    char *relation_name;            // Relation to delete from
    size_t condition_num;           // Length of conditions in Where clause
    Condition conditions[MAX_NUM];  // conditions in Where clause
} Deletes;

// struct of update
typedef struct {
    char *relation_name;            // Relation to update
    char *attribute_name;           // Attribute to update
    Value value;                    // update value
    size_t condition_num;           // Length of conditions in Where clause
    Condition conditions[MAX_NUM];  // conditions in Where clause
} Updates;

typedef struct {
    char *name;     // Attribute name
    AttrType type;  // Type of attribute
    size_t length;  // Length of attribute
    bool nullable;
} AttrInfo;

// struct of craete_table
typedef struct {
    char *relation_name;           // Relation name
    size_t attribute_count;        // Length of attribute
    AttrInfo attributes[MAX_NUM];  // attributes
} CreateTable;

// struct of drop_table
typedef struct {
    char *relation_name;  // Relation name
} DropTable;

// struct of create_index
typedef struct {
    char *index_name; // 索引名
    char *relation_name; // 表名
    unsigned int attribute_count; // 包含的属性个数
    char *attribute_names[MAX_NUM]; // 属性名
    bool unique_attr; // 是否为UNIQUE索引
} CreateIndex;

// struct of  drop_index
typedef struct {
    const char *index_name;  // Index name
} DropIndex;

typedef struct {
    const char *relation_name;
} DescTable;

typedef struct {
    const char *relation_name;
    const char *file_name;
} LoadData;

union Queries {
    Selects selection;
    Inserts insertion;
    Deletes deletion;
    Updates update;
    CreateTable create_table;
    DropTable drop_table;
    CreateIndex create_index;
    DropIndex drop_index;
    DescTable desc_table;
    LoadData load_data;
    char *errors;
};

// 修改yacc中相关数字编码为宏定义
enum SqlCommandFlag {
    SCF_ERROR = 0,
    SCF_SELECT,
    SCF_INSERT,
    SCF_UPDATE,
    SCF_DELETE,
    SCF_CREATE_TABLE,
    SCF_DROP_TABLE,
    SCF_CREATE_INDEX,
    SCF_DROP_INDEX,
    SCF_SYNC,
    SCF_SHOW_TABLES,
    SCF_DESC_TABLE,
    SCF_BEGIN,
    SCF_COMMIT,
    SCF_ROLLBACK,
    SCF_LOAD_DATA,
    SCF_HELP,
    SCF_EXIT
};
// struct of flag and sql_struct
typedef struct Query {
    enum SqlCommandFlag flag;
    union Queries sstr;
} Query;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
void e_e_t(CalOp op);
void e_t();
void t_t_f(CalOp op);
void t_f();
void f_e();
void relation_value_append(Value *value,int num);
void selects_append_attribute_plus(Selects *selects);
void relation_attr_get(RelAttr *attr);
void condition_init_ex(Condition* condition,CompOp op);
void relation_attr_init(RelAttr *relation_attr, const char *relation_name, const char *attribute_name);
void relation_agg_attr_init(RelAttr *relation_attr, const char *relation_name, const char *attribute_name,AggType aggType);
void relation_attr_destroy(RelAttr *relation_attr);

void value_init_integer(Value *value, int v,int signal);
void value_init_float(Value *value, float v,int signal);
void value_init_string(Value *value, const char *v);
void value_init_null(Value *value);
void value_destroy(Value *value);

void condition_init(Condition *condition, CompOp comp, int left_is_attr, RelAttr *left_attr, Value *left_value,
                    int right_is_attr, RelAttr *right_attr, Value *right_value);
void condition_destroy(Condition *condition);

void attr_info_init(AttrInfo *attr_info, const char *name, AttrType type, size_t length, bool nullable);
void attr_info_destroy(AttrInfo *attr_info);

void selects_init(Selects *selects, ...);
void selects_append_attribute(Selects *selects, RelAttr *rel_attr);
void selects_append_orders(Selects * selects,RelAttr * rel_attr,int asc);
void selects_append_relation(Selects *selects, const char *relation_name);
void selects_set_order(Selects* selects,int order);
void selects_append_conditions(Selects *selects, Condition conditions[], size_t condition_num);
void selects_destroy(Selects *selects);

void inserts_init(Inserts *inserts, const char *relation_name, LexTuple tuples[], size_t tuple_count);
void inserts_destroy(Inserts *inserts);

void deletes_init_relation(Deletes *deletes, const char *relation_name);
void deletes_set_conditions(Deletes *deletes, Condition conditions[], size_t condition_num);
void deletes_destroy(Deletes *deletes);

void updates_init(Updates *updates, const char *relation_name, const char *attribute_name, Value *value,
                  Condition conditions[], size_t condition_num);
void updates_destroy(Updates *updates);

void create_table_append_attribute(CreateTable *create_table, AttrInfo *attr_info);
void create_table_init_name(CreateTable *create_table, const char *relation_name);
void create_table_destroy(CreateTable *create_table);

void drop_table_init(DropTable *drop_table, const char *relation_name);
void drop_table_destroy(DropTable *drop_table);

void create_index_init(CreateIndex *obj, const char *index_name,
                       const char *relation_name, const char *attr_name, bool unique_attr);
void create_index_add_attribute(CreateIndex *obj, const char *attr_name);
void create_index_destroy(CreateIndex *obj);

void drop_index_init(DropIndex *drop_index, const char *index_name);
void drop_index_destroy(DropIndex *drop_index);

void desc_table_init(DescTable *desc_table, const char *relation_name);
void desc_table_destroy(DescTable *desc_table);

void load_data_init(LoadData *load_data, const char *relation_name, const char *file_name);
void load_data_destroy(LoadData *load_data);

void query_init(Query *query);
Query *query_create();  // create and init
void query_reset(Query *query);
void query_destroy(Query *query);  // reset and delete
void set_sub();
#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __OBSERVER_SQL_PARSER_PARSE_DEFS_H__