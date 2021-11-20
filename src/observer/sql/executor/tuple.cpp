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
/**
 * success=-1 则有值没有被找到
 * @param tuple
 * @param schema
 * @param attr
 * @param success 返回0则是简单值，返回1则是表达式值
 * @param tupleValue
 * @return
 */
double getvalue(Tuple *tuple,TupleSchema *schema,RelAttr &attr,int *success,std::shared_ptr<TupleValue>& tupleValue){
    double left;
    double right;
    double value;
    switch (attr.extype) {
        case E:{
            if(attr.second== nullptr){
                value=getvalue(tuple,schema,*attr.first,success,tupleValue);
            } else{
                left=getvalue(tuple,schema,*attr.first,success,tupleValue);
                if(*success==-1){
                    return 0;
                }
                right=getvalue(tuple,schema,*attr.second,success,tupleValue);
                if(*success==-1){
                    return 0;
                }
                *success=1;
                if(attr.op==Add){
                    value= left+right;
                } else{
                    value= left-right;
                }
            }
            break;
        }
        case T:{
            if(attr.second== nullptr){
                value=getvalue(tuple,schema,*attr.first,success,tupleValue);
            } else{
                left=getvalue(tuple,schema,*attr.first,success,tupleValue);
                if(*success==-1){
                    return 0;
                }
                right=getvalue(tuple,schema,*attr.second,success,tupleValue);
                if(*success==-1){
                    return 0;
                }
                *success=1;
                if(attr.op==Mul){
                    value= left*right;
                } else{
                    value= left/right;
                }
            }
            break;
        }
        case F:{
            value= getvalue(tuple,schema,*attr.first,success,tupleValue);
            if(*success==-1){
                return 0;
            }
            *success=1;
            break;
        }
        case id:{
            if(attr.num==-1){
                *success=1;
            } else{
                *success=0;
            }
            attr.id=schema->index_of_field(attr.relation_name,attr.attribute_name)+1;
            if(attr.id==0){
                *success=-1;
            }
            if(attr.id>0){
                TupleField field=schema->field(attr.id-1);
                if(field.type()==INTS||field.type()==FLOATS){
                        value=tuple->get(attr.id-1).getFValue();
                }
                tupleValue=tuple->get_pointer(attr.id-1);
            }


            break;
        }
        case val:{
            if(attr.value.type==INTS){
                value=*(int *)attr.value.data*1.0;
            } else{
                value=*(float *)attr.value.data;
            }
            *success=1;
            break;
        }
    }
    return value*attr.num;
}
bool do_filters(Tuple *tuple,std::vector<Condition> conditions,TupleSchema* schema){
    for(int i=0;i<conditions.size();i++){
        int success1;
        int success2;
        Condition  condition=conditions[i];
        std::shared_ptr<TupleValue> left_value;
        std::shared_ptr<TupleValue> right_value;
        double ret;
        double left= getvalue(tuple,schema,condition.left_attr,&success1,left_value);
        double right= getvalue(tuple,schema,condition.right_attr,&success2,right_value);
        if(success1==-1||success2==-1){
            continue;
        }
        if(success1||success2){//是表达式
            ret=left-right;
            if(ret>0){
                ret=1;
            } else if(ret<0){
                ret =-1;
            }
        } else{
            ret=left_value->compare(*right_value);
        }

        CompOp op=condition.comp;
        switch (op) {
            case CompOp::EQUAL_TO: {
                if (ret != 0) {
                    return false;
                }
                break;
            }
            case CompOp::GREAT_EQUAL: {
                if (ret!=0&&ret!=1) {
                    return false;
                }
                break;
            }
            case CompOp::GREAT_THAN: {
                if (ret !=1) {
                    return false;
                }
                break;
            }
            case CompOp::LESS_EQUAL: {
                if (ret!=0&&ret!=-1) {
                    return false;
                }
                break;
            }
            case CompOp::LESS_THAN: {
                if (ret!=-1) {
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
void print_attr_with_table(RelAttr attr,std::ostream &os){
    int value;
    switch (attr.extype) {
        case E:{
            if(attr.second== nullptr){
                print_attr_with_table(*attr.first,os);
            } else{
                if(attr.op==Add){
                    print_attr_with_table(*attr.first,os);
                    os<<"+";
                    print_attr_with_table(*attr.second,os);
                } else{
                    print_attr_with_table(*attr.first,os);
                    os<<"-";
                    print_attr_with_table(*attr.second,os);
                }
            }
            break;
        }
        case T:{
            if(attr.second== nullptr){
                print_attr_with_table(*attr.first,os);
            } else{
                if(attr.op==Mul){
                    print_attr_with_table(*attr.first,os);
                    os<<"*";
                    print_attr_with_table(*attr.second,os);
                } else{
                    print_attr_with_table(*attr.first,os);
                    os<<"/";
                    print_attr_with_table(*attr.second,os);
                }
            }
            break;
        }
        case F:{
            if(attr.num==-1){
                os<<"-";
            }
            os<<"(";
            print_attr_with_table(*attr.first,os);
            os<<")";
            break;
        }
        case id:{
            if(attr.num==-1){
                os<<"-";
            }
            os<<attr.relation_name<<"."<<attr.attribute_name;
            break;
        }
        case val:{
            if(attr.value.type==INTS){
                value=*(int *)attr.value.data*1.0;
            } else{
                value=*(float *)attr.value.data;
            }
            os<<value;
            break;
        }
    }
}
void print_attr(RelAttr attr,std::ostream &os){
    int value;
    switch (attr.extype) {
        case E:{
            if(attr.second== nullptr){
                print_attr(*attr.first,os);
            } else{
                if(attr.op==Add){
                    print_attr(*attr.first,os);
                    os<<"+";
                    print_attr(*attr.second,os);
                } else{
                    print_attr(*attr.first,os);
                    os<<"-";
                    print_attr(*attr.second,os);
                }
            }
            break;
        }
        case T:{
            if(attr.second== nullptr){
                print_attr(*attr.first,os);
            } else{
                if(attr.op==Mul){
                    print_attr(*attr.first,os);
                    os<<"*";
                    print_attr(*attr.second,os);
                } else{
                    print_attr(*attr.first,os);
                    os<<"/";
                    print_attr(*attr.second,os);
                }
            }
            break;
        }
        case F:{
            if(attr.num==-1){
                os<<"-";
            }
            os<<"(";
            print_attr(*attr.first,os);
            os<<")";
            break;
        }
        case id:{
            if(attr.num==-1){
                os<<"-";
            }
            os<<attr.attribute_name;
            break;
        }
        case val:{
            if(attr.value.type==INTS){
                value=*(int *)attr.value.data*1.0;
            } else{
                value=*(float *)attr.value.data;
            }
            os<<value;
            break;
        }
    }
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

void Tuple::add(const char *s, int len, bool null_attr, bool is_text = false) {
    if (!is_text) {
        // CHARS
        add(new StringValue(s, len, null_attr));
    } else {
        add(new TextValue(s, len, null_attr));
    }
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
            case TEXTS:
                add((char*)data, (int)strlen((char*)data), false, true);
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
            default:
                LOG_ERROR("Programming error: Trying to add value of unsupported type (%u) into Tuple.", type_);
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
int TupleSchema::index_of_rel(const char *table_name, const char *field_name) const {
    const int size = relattrs.size();
    for (int i = 0; i < size; i++) {
        const RelAttr &relAttr = relattrs[i];
        if (0 == strcmp(relAttr.relation_name, table_name) && 0 == strcmp(relAttr.attribute_name, field_name)) {
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
    if (get_col_number()==0) {
        os << "No schema";
        return;
    }
    if(fields_.size()!=0){
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
    } else{
        for(int i=0;i<relattrs.size()-1;i++){
            print_attr_with_table(relattrs[i],os);
            os<<" | ";
        }
        print_attr_with_table(relattrs[relattrs.size()-1],os);
        os<<std::endl;
    }

}

void TupleSchema::print(std::ostream &os) const {
    if (get_col_number()==0) {
        os << "No schema";
        return;
    }
    if(!fields_.empty()){
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
    } else{
        for(int i=0;i<relattrs.size()-1;i++){
            print_attr(relattrs[i],os);
            os<<" | ";
        }
        print_attr(relattrs[relattrs.size()-1],os);
        os<<std::endl;
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
    TupleSchema schema;
    schema.append(this->schema_);
    schema.append(other.schema_);
    //对每个元组进行过滤
    for (int i = 0; i < this->size(); i++) {
        for (int j = 0; j < other.size(); j++) {
            Tuple *tuples[2];
            tuples[0] = this->get(i);
            tuples[1] = other.get(j);
            Tuple tuple;
            tuple.add(tuples[0]);
            tuple.add(tuples[1]);
            bool add = do_filters(&tuple,conditions,&schema);
            if(add){
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
    if (schema_.get_col_number()==0) {
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
    if (schema_.get_col_number()==0) {
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

    // 把value的data复制一份，使得其不是const的
    auto sz = v.data_length();
    void *buf = malloc(sz);
    const void *src;
    v.get_data(src);
    memcpy(buf, src, sz);
    ret.data = buf;
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
            case TEXTS: {
                // 从堆中获取实际值
                uint32_t ref = *(uint32_t*)(record + field_meta->offset());
                if (!null_attr) {
                    auto s = table_->heap_manager->get(ref);
                    tuple.add(s, (int)strlen(s), null_attr, true);
                    free((void*)s);
                } else {
                    tuple.add("\0", 0, null_attr, true);
                }
            }
                break;
            default: {
                LOG_PANIC("Unsupported field type. type=%d", field_meta->type());
            }
        }
    }

    tuple_set_.add(std::move(tuple));

}


