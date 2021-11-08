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
// Created by Wangyunlai on 2021/5/14.
//

#ifndef __OBSERVER_SQL_EXECUTOR_TUPLE_H_
#define __OBSERVER_SQL_EXECUTOR_TUPLE_H_

#include <memory>
#include <vector>
#include <algorithm>

#include "sql/parser/parse.h"
#include "sql/executor/value.h"

class Table;
static std::vector<int> orders;
struct filter{
int left_table;
int left_value;
int right_table;
int right_value;
CompOp op;
bool is_same_type{true};
};
struct out_map{
    int table;
    int value;
};


//具体的一个元组，数组tuplevalue构成
class Tuple {
public:
    Tuple() = default;

    Tuple(const Tuple &other);

    ~Tuple();

    Tuple(Tuple &&other) noexcept;

    Tuple &operator=(Tuple &&other) noexcept;

    void add(TupleValue *value);
    void add(Tuple *tuple);

    void add(std::vector<std::shared_ptr<TupleValue>> other);

    void add(int value);

    void add(float value);

    void add(const char *s, int len);

    void add(unsigned int value);

    void add(time_t value);
    bool operator==(Tuple & other){
        for(int i=0;i<orders.size();i++){
            if(!values_.at(orders[i])->compare(other.get(orders[i]))){
                return false;
            }
        }
        return true;
    }
    bool operator<(Tuple &other){
        for(int i=0;i<orders.size();i++){
            int ret=values_.at(orders[i])->compare(other.get(orders[i]));
            if(ret<0){
                return true;
            } else if(ret==0){
                continue;
            }
            else{
                return false;
            }
        }
        return true;
    }
    const std::vector<std::shared_ptr<TupleValue>> &values() const {
        return values_;
    }

    int size() const {
        return values_.size();
    }

    const TupleValue &get(int index) const {
        return *values_[index];
    }

    const std::shared_ptr<TupleValue> &get_pointer(int index) const {
        return values_[index];
    }
    static void append_order_attr(int id){
        orders.push_back(id);
    }
    static void clear(){
        orders.clear();
    }
private:
    std::vector<std::shared_ptr<TupleValue>> values_;


};

//该类的数组构成tuple的模式
class TupleField {
public:
    TupleField(AttrType type, const char *table_name, const char *field_name) :
            type_(type), table_name_(table_name), field_name_(field_name) {
    }

    AttrType type() const {
        return type_;
    }

    const char *table_name() const {
        return table_name_.c_str();
    }

    const char *field_name() const {
        return field_name_.c_str();
    }

    std::string to_string() const;

private:
    AttrType type_;
    std::string table_name_;
    std::string field_name_;
};

class TupleSchema {
public:
    TupleSchema() = default;

    ~TupleSchema() = default;

    void add(AttrType type, const char *table_name, const char *field_name);
    int  get_field_size(){
        return field_num;
    }
    void add_if_not_exists(AttrType type, const char *table_name, const char *field_name);

    // void merge(const TupleSchema &other);
    void append(const TupleSchema &other);

    //去重加入
    void append_if_not_exists(const TupleSchema &other);

    const std::vector<TupleField> &fields() const {
        return fields_;
    }

    const TupleField &field(int index) const {
        return fields_[index];
    }

    int index_of_field(const char *table_name, const char *field_name) const;

    void clear() {
        fields_.clear();
    }
    void print(std::ostream &os) const;
    void print_with_table(std::ostream &os) const;

public:
    static void from_table(const Table *table, TupleSchema &schema);

private:
    std::vector<TupleField> fields_;
    int field_num{0};
};

//简单的tuple集合
class TupleSet {
public:
    TupleSet() = default;

    TupleSet(TupleSet &&other);

    explicit TupleSet(const TupleSchema &schema) : schema_(schema) {
    }

    TupleSet &operator=(TupleSet &&other);

    ~TupleSet() = default;

    void set_schema(const TupleSchema &schema);

    const TupleSchema &get_schema() const;

    void add(Tuple &&tuple);
    void add(Tuple* tuple);
    void clear();

    bool is_empty() const;

    int size() const;

    const Tuple &get(int index) const;
    const std::string &getData(int index) const;
    Tuple* get(int index);
    const std::vector<Tuple> &tuples() const;
    void print(std::ostream &os) const;
    void print_with_table(std::ostream &os) const;
    void sort(){
        std::sort(tuples_.begin(),tuples_.end());
    }
    void set_print_order(int  order){
        print_order=order;
    }

public:
    const TupleSchema &schema() const {
        return schema_;
    }

private:
    std::vector<Tuple> tuples_;
    TupleSchema schema_;
    int print_order= 1;
};

/**
 * 转换record存入set
 */
class TupleRecordConverter {
public:
    TupleRecordConverter(Table *table, TupleSet &tuple_set);

//根据meta将record解析为tuple,并加入集合
    void add_record(const char *record);

private:
    Table *table_;
    TupleSet &tuple_set_;
};

#endif //__OBSERVER_SQL_EXECUTOR_TUPLE_H_
