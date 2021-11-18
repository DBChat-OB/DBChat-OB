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

#include <sstream>
#include "sql/executor/tuple.h"
#include "storage/common/table.h"
#include "common/log/log.h"
struct filter_map {
    int left_table;
    int right_table;
    int right_value;
    int left_value;
    CompOp op;
};
bool do_filter(Tuple **tuples, struct filter_map *filters, int num) {
    for (int i = 0; i < num; i++) {
        int ret;
        ret = tuples[filters[i].left_table]->get(filters[i].left_value).compare(
                tuples[filters[i].right_table]->get(filters[i].right_value));
        switch (filters[i].op) {
            case CompOp::EQUAL_TO: {
                if (ret != 0) {
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
            case CompOp::GREAT_THAN: {
                if (ret <= 0) {
                    return false;
                }
                break;
            }
            case CompOp::LESS_EQUAL: {
                if (ret > 0) {
                    return false;
                }
                break;
            }
            case CompOp::LESS_THAN: {
                if (ret >= 0) {
                    return false;
                }
                break;
            }
            case CompOp::NOT_EQUAL: {
                if (ret == 0) {
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
Tuple::Tuple(const Tuple &other) {
    LOG_PANIC("Copy constructor of tuple is not supported");
    exit(1);
}

Tuple::Tuple(Tuple &&other) noexcept: values_(std::move(other.values_)) {
}

Tuple &Tuple::operator=(Tuple &&other) noexcept {
    if (&other == this) {
        return *this;
    }

    values_.clear();
    values_.swap(other.values_);
    return *this;
}

Tuple::~Tuple() {
}

// add (Value && value)
void Tuple::add(TupleValue *value) {
    values_.emplace_back(value);
}

void Tuple::add(std::vector<std::shared_ptr<TupleValue>> other) {
    for (std::shared_ptr<TupleValue> value: other) {
        values_.emplace_back(value);
    }
}

void Tuple::add(Tuple *tuple) {
    add(tuple->values_);

}

void Tuple::add(int value, bool null_attr) {
    add(new IntValue(value,null_attr));
}

void Tuple::add(float value, bool null_attr) {
    add(new FloatValue(value,null_attr));
}

void Tuple::add(const char *s, int len, bool null_attr) {
    add(new StringValue(s, len, null_attr));
}

void Tuple::add(unsigned int value, bool null_attr) {
    add(new DateValue(value, null_attr));
}

void Tuple::add(time_t value, bool null_attr) {
    add(new DateValue(value, null_attr));
}

// Create a Tuple with only one element of given type of data.
Tuple::Tuple(AttrType type_, void *data) {
    switch (type_) {
            case CHARS:
                add((char*)data, (int)strlen((char*)data), false);
                break;
            case INTS:
                add(*(int*)data, false);
                break;
            case FLOATS:
                add(*(float*)data, false);
                break;
            case DATE:
                add(*(time_t*)data, false);
                break;
            case UNDEFINED:
            case UNEVALUATED:
            case ATTR_TABLE:
                LOG_ERROR("Programming error: Trying to add value of unsupported type into Tuple.");
                assert(0);
                break;
    }
}

////////////////////////////////////////////////////////////////////////////////

std::string TupleField::to_string() const {
    return std::string(table_name_) + "." + field_name_ + std::to_string(type_);
}

////////////////////////////////////////////////////////////////////////////////
void TupleSchema::from_table(const Table *table, TupleSchema &schema) {
    const char *table_name = table->name();
    const TableMeta &table_meta = table->table_meta();
    const int field_num = table_meta.field_num();
    for (int i = 0; i < field_num; i++) {
        const FieldMeta *field_meta = table_meta.field(i);
        if (field_meta->visible()) {
            schema.add(field_meta->type(), table_name, field_meta->name(), field_meta->nullable());
        }
    }
}

void TupleSchema::add(AttrType type, const char *table_name, const char *field_name, bool nullable) {
    AggType aggType = Null;
    fields_.emplace_back(type, table_name, field_name, nullable, aggType);
    field_num++;
}

void TupleSchema::add_agg(AttrType type, const char *table_name, const char *field_name, bool nullable, AggType aggType) {
    fields_.emplace_back(type, table_name, field_name,nullable,aggType);
    field_num++;
}

void TupleSchema::add_if_not_exists(AttrType type, const char *table_name, const char *field_name, bool nullable) {
    for (const auto &field: fields_) {
        if (0 == strcmp(field.table_name(), table_name) &&
            0 == strcmp(field.field_name(), field_name)) {
            return;
        }
    }
    add(type, table_name, field_name, nullable);
}

void TupleSchema::append(const TupleSchema &other) {
    fields_.reserve(fields_.size() + other.fields_.size());
    for (const auto &field: other.fields_) {
        fields_.emplace_back(field);
    }
    field_num += other.field_num;
}

void TupleSchema::append_if_not_exists(const TupleSchema &other) {
    for (const auto &field: other.fields()) {
        add_if_not_exists(field.type(), field.table_name(), field.field_name(), field.nullable());
    }
}

int TupleSchema::index_of_field(const char *table_name, const char *field_name) const {
    const int size = fields_.size();
    for (int i = 0; i < size; i++) {
        const TupleField &field = fields_[i];
        if (0 == strcmp(field.table_name(), table_name) && 0 == strcmp(field.field_name(), field_name)) {
            return i;
        }
    }
    return -1;
}
void print_agg(TupleField tupleField,std::ostream &os){
    switch (tupleField.agg_type()) {
        case Min:{
            os<<"min(";
        }
            break;
        case Max:{
            os<<"max(";
        }
            break;
        case Avg:{
            os<<"avg(";
        }
            break;
        case Count:{
            os<<"count(";
        }
            break;
    }
}
void TupleSchema::print_with_table(std::ostream &os) const {
    if (fields_.empty()) {
        os << "No schema";
        return;
    }
    for (std::vector<TupleField>::const_iterator iter = fields_.begin(), end = --fields_.end();
         iter != end; ++iter) {
        if((*iter).agg_type()==Null){
            os << iter->table_name() << ".";
            os << iter->field_name() << " | ";
        } else{
            print_agg(*iter,os);
            os << iter->table_name() << ".";
            os << iter->field_name() <<")"<< " | ";
        }
    }
    if(fields_.back().agg_type()==Null){
        os << fields_.back().table_name() << ".";
        os << fields_.back().field_name() << std::endl;
    } else{
        print_agg(fields_.back(),os);
        os << fields_.back().table_name() << ".";
        os << fields_.back().field_name() <<")"<< std::endl;
    }

}

void TupleSchema::print(std::ostream &os) const {
    if (fields_.empty()) {
        os << "No schema";
        return;
    }

    // 判断有多张表还是只有一张表
    std::set<std::string> table_names;
    for (const auto &field: fields_) {
        table_names.insert(field.table_name());
    }
    for (std::vector<TupleField>::const_iterator iter = fields_.begin(), end = --fields_.end();
         iter != end; ++iter) {
        if((*iter).agg_type()==Null){
            os << iter->field_name() << " | ";
        } else{
            print_agg(*iter,os);
            os << iter->field_name() <<")"<< " | ";
        }
    }
    if(fields_.back().agg_type()==Null){
        os << fields_.back().field_name() << std::endl;
    } else{
        print_agg(fields_.back(),os);
        os << fields_.back().field_name()<<")" << std::endl;
    }
}


/////////////////////////////////////////////////////////////////////////////
TupleSet::TupleSet(TupleSet &&other) : tuples_(std::move(other.tuples_)), schema_(other.schema_) {
    other.schema_.clear();
}

TupleSet &TupleSet::operator=(TupleSet &&other) {
    if (this == &other) {
        return *this;
    }

    schema_.clear();
    schema_.append(other.schema_);
    other.schema_.clear();

    tuples_.clear();
    tuples_.swap(other.tuples_);
    return *this;
}

void TupleSet::add(Tuple &&tuple) {
    tuples_.emplace_back(std::move(tuple));
}

void TupleSet::join(TupleSet&other,TupleSet&ret,std::vector<Condition>&conditions)  {

    struct filter_map filters[MAX_NUM];
    int filter_size = 0;
    TupleSet *tupleSets[2];
    tupleSets[0] = this;
    tupleSets[1] = &other;
    //先选出位于这两表的属性 建立filter映射
    for (int i = 0; i < conditions.size(); i++) {
        Condition condition = conditions[i];
        int left_id;
        int right_id;
        if ((left_id = this->get_schema().index_of_field(condition.left_attr.relation_name,
                                                         condition.left_attr.attribute_name)) != -1 &&
            (right_id = other.get_schema().index_of_field(condition.right_attr.relation_name,
                                                          condition.right_attr.attribute_name)) != -1) {
            filters[filter_size].left_table = 0;
            filters[filter_size].right_table = 1;
            filters[filter_size].left_value = left_id;
            filters[filter_size].right_value = right_id;
            filters[filter_size].op = condition.comp;
            filter_size++;
        } else if ((left_id = other.get_schema().index_of_field(condition.left_attr.relation_name,
                                                                condition.left_attr.attribute_name)) != -1 &&
                   (right_id = this->get_schema().index_of_field(condition.right_attr.relation_name,
                                                                 condition.right_attr.attribute_name)) != -1) {
            filters[filter_size].left_table = 1;
            filters[filter_size].right_table = 0;
            filters[filter_size].left_value = left_id;
            filters[filter_size].right_value = right_id;
            filters[filter_size].op = condition.comp;
            filter_size++;
        }
    }
    //对每个元组进行过滤
    for (int i = 0; i < this->size(); i++) {
        for (int j = 0; j < other.size(); j++) {
            Tuple *tuples[2];
            tuples[0] = this->get(i);
            tuples[1] = other.get(j);
            bool add = do_filter(tuples, filters, filter_size);
            if(add){
                Tuple tuple;
                tuple.add(tuples[0]);
                tuple.add(tuples[1]);
                ret.add(std::move(tuple));
            }
        }
    }
}
void TupleSet::clear() {
    tuples_.clear();
    schema_.clear();
}

void TupleSet::print_with_table(std::ostream &os) const {
    if (schema_.fields().empty()) {
        LOG_WARN("Got empty schema");
        return;
    }
    schema_.print_with_table(os);

    for (const Tuple &item: tuples_) {
        const std::vector<std::shared_ptr<TupleValue>> &values = item.values();
        for (std::vector<std::shared_ptr<TupleValue>>::const_iterator iter = values.begin(), end = --values.end();
             iter != end; ++iter) {
            (*iter)->to_string(os);
            os << " | ";
        }
        values.back()->to_string(os);
        os << std::endl;
    }


}

void TupleSet::print(std::ostream &os) const {
    if (schema_.fields().empty()) {
        LOG_WARN("Got empty schema");
        return;
    }

    schema_.print(os);

    for (const Tuple &item: tuples_) {
        const std::vector<std::shared_ptr<TupleValue>> &values = item.values();
        for (std::vector<std::shared_ptr<TupleValue>>::const_iterator iter = values.begin(), end = --values.end();
             iter != end; ++iter) {
            (*iter)->to_string(os);
            os << " | ";
        }
        values.back()->to_string(os);
        os << std::endl;
    }

}

void TupleSet::set_schema(const TupleSchema &schema) {
    schema_ = schema;
}

const TupleSchema &TupleSet::get_schema() const {
    return schema_;
}

bool TupleSet::is_empty() const {
    return tuples_.empty();
}

int TupleSet::size() const {
    return tuples_.size();
}

const Tuple &TupleSet::get(int index) const {
    return tuples_[index];
}

Tuple *TupleSet::get(int index) {
    return &tuples_[index];
}


const std::vector<Tuple> &TupleSet::tuples() const {
    return tuples_;
}

bool TupleSet::flatten(Value &ret) const {
    if (this->size() != 1)  return false;
    auto &t = this->get(0);
    if (t.size() != 1)  return false;
    auto &v = t.get(0);
    ret.type = v.get_type();
    v.get_data(ret.data);
    return true;
}


/////////////////////////////////////////////////////////////////////////////
TupleRecordConverter::TupleRecordConverter(Table *table, TupleSet &tuple_set) :
        table_(table), tuple_set_(tuple_set) {
}

//加上字符串拼接
void TupleRecordConverter::add_record(const char *record) {
    const TupleSchema &schema = tuple_set_.schema();
    Tuple tuple;
    const TableMeta &table_meta = table_->table_meta();
    int length = 0;


    int p = 0;
    for (const TupleField &field: schema.fields()) {
        const FieldMeta *field_meta = table_meta.field(field.field_name());
        assert(field_meta != nullptr);
        bool null_attr = ((*(unsigned *) (record + field_meta->offset()-4))&0x00000001==0x00000001);
        switch (field_meta->type()) {
            case INTS: {
                int value = *(int *) (record + field_meta->offset());
                tuple.add(value,null_attr);
            }
                break;
            case FLOATS: {
                float value = *(float *) (record + field_meta->offset());
                tuple.add(value,null_attr);
            }
                break;
            case CHARS: {
                const char *s = record + field_meta->offset();  // 现在当做Cstring来处理
                tuple.add(s, strlen(s),null_attr);
            }
                break;
            case DATE: {
                unsigned int value = *(unsigned int *) (record + field_meta->offset());
                tuple.add(value,null_attr);
            }
                break;
            default: {
                LOG_PANIC("Unsupported field type. type=%d", field_meta->type());
            }
        }
    }

    tuple_set_.add(std::move(tuple));

}


