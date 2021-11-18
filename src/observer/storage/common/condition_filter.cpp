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
// Created by Wangyunlai on 2021/5/7.
//

#include <stddef.h>
#include <assert.h>
#include <cmath>
#include "unevaluated.h"
#include "condition_filter.h"
#include "record_manager.h"
#include "common/log/log.h"
#include "storage/common/table.h"
#include "storage/common/mytime.cpp"
#include "sql/executor/tuple.h"

using namespace common;

ConditionFilter::~ConditionFilter() {}

DefaultConditionFilter::DefaultConditionFilter() {
    left_.is_attr = false;
    left_.attr_length = 0;
    left_.attr_offset = 0;
    left_.value = nullptr;

    right_.is_attr = false;
    right_.attr_length = 0;
    right_.attr_offset = 0;
    right_.value = nullptr;
}

DefaultConditionFilter::~DefaultConditionFilter() {
    // unevaluated类型会计算出一个临时的实际值，在此需要销毁
    if (left_.is_ue)  free(left_.value); // 参考value_destroy
    if (right_.is_ue)  free(right_.value); // 参考value_destroy
}

RC DefaultConditionFilter::init(const ConDesc &left, const ConDesc &right, AttrType attr_type, CompOp comp_op) {
    // TODO fix this
    if (attr_type < UNDEFINED || attr_type > ATTR_TABLE) {
        LOG_ERROR("Invalid condition with unsupported attribute type: %d", attr_type);
        return RC::INVALID_ARGUMENT;
    }

    if (comp_op < EQUAL_TO || comp_op >= NO_OP) {
        LOG_ERROR("Invalid condition with unsupported compare operation: %d", comp_op);
        return RC::INVALID_ARGUMENT;
    }

    left_ = left;
    right_ = right;
    attr_type_ = attr_type;
    comp_op_ = comp_op;
    return RC::SUCCESS;
}

RC DefaultConditionFilter::init(Table &table, const Condition &condition, Trx *trx) {
    const TableMeta &table_meta = table.table_meta();
    ConDesc left;
    ConDesc right;

    AttrType type_left = UNDEFINED;
    AttrType type_right = UNDEFINED;

    if (1 == condition.left_is_attr) {
        left.is_attr = true;
        const FieldMeta *field_left = table_meta.field(condition.left_attr.attribute_name);
        if (nullptr == field_left) {
            LOG_WARN("No such field in condition. %s.%s", table.name(), condition.left_attr.attribute_name);
            return RC::SCHEMA_FIELD_MISSING;
        }
        left.attr_length = field_left->len();
        left.attr_offset = field_left->offset();

        left.value = nullptr;

        type_left = field_left->type();
    } else {
        left.is_attr = false;
        left.attr_length = 0;
        left.attr_offset = 0;
        if (condition.left_value.type != UNEVALUATED) {
            // 左值已经是实际值，不需要求值
            left.is_ue = false;
            left.is_null = condition.left_value.null_attr;
            left.value = condition.left_value.data;  // 校验type 或者转换类型
            type_left = condition.left_value.type;
        } else {
            // 左值未被求出
            // 未求值的抽象值需要先计算出实际值才能被filter使用
            left.is_ue = true;
            Value concrete_value;
            RC err;
            if ((err = unevaluated::eval(condition.left_value, table, trx, concrete_value)) != RC::SUCCESS) {
                LOG_ERROR("Failed to evaluate value referenced in filter condition.");
                return err;
            }
            assert(concrete_value.type != UNEVALUATED);
            left.value = concrete_value.data;
            type_left = concrete_value.type;
        }
    }

    if (1 == condition.right_is_attr) {
        right.is_attr = true;
        const FieldMeta *field_right = table_meta.field(condition.right_attr.attribute_name);
        if (nullptr == field_right) {
            LOG_WARN("No such field in condition. %s.%s", table.name(), condition.right_attr.attribute_name);
            return RC::SCHEMA_FIELD_MISSING;
        }
        right.attr_length = field_right->len();
        right.attr_offset = field_right->offset();
        type_right = field_right->type();

    right.value = nullptr;
    if(type_right==DATE&&!left.is_attr&&type_left==CHARS){
        time_t time_value;
        if(mytime::chars_to_date((char*)left.value,time_value)){
            unsigned int time_int = time_value;
            memcpy(left.value, &time_int, 4);
        }
        else{
            return RC::SCHEMA_FIELD_TYPE_MISMATCH;
        }
    }
  } else {
    right.is_attr = false;
    right.attr_length = 0;
    right.attr_offset = 0;
    if (condition.right_value.type != UNEVALUATED) {
        // 右值已经是实际值，不需要求值
        right.is_ue = false;
        right.is_null = condition.right_value.null_attr;
        right.value = condition.right_value.data;
        type_right = condition.right_value.type;
    } else {
        // 右值未被求出
        // 未求值的抽象值需要先计算出实际值才能被filter使用
        right.is_ue = true;
        Value concrete_value;
        RC err;
        if ((err = unevaluated::eval(condition.right_value, table, trx, concrete_value)) != RC::SUCCESS) {
            LOG_ERROR("Failed to evaluate value referenced in filter condition.");
            return err;
        }
        assert(concrete_value.type != UNEVALUATED);
        right.value = concrete_value.data;
        type_right = concrete_value.type;
    }

    if (type_right == CHARS && left.is_attr && type_left == DATE) {
        time_t time_value;
        if (mytime::chars_to_date((char *) right.value, time_value)) {
            unsigned int time_int = time_value;
            memcpy(right.value, &time_int, 4);
            type_right = DATE;
        } else {
            return RC::SCHEMA_FIELD_TYPE_MISMATCH;
        }
    }
  }

    // 校验和转换
    if (!field_type_compare_compatible_table[type_left][type_right]) {
        // 不能比较的两个字段， 要把信息传给客户端
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
    // TODO：这里没有实现不同类型的数据比较，比如整数跟浮点数之间的对比
    // 但是选手们还是要实现。这个功能在预选赛中会出现
//  if (type_left != type_right) {
//    return RC::SCHEMA_FIELD_TYPE_MISMATCH;
//  }

    // 比较时要转换成什么类型
    // 设置为ATTR_TABLE来表示至少有一边是TABLE。
    auto cast_type = type_left;
    if (type_left == ATTR_TABLE || type_right == ATTR_TABLE) {
        cast_type = ATTR_TABLE;
    }

    this->type_left = type_left;
    this->type_right = type_right;

    return init(left, right, cast_type, condition.comp);
}

static double scalar_to_double(const Value v) {
    switch (v.type) {
        case CHARS: {
            char *end;
            double d = strtod((char*)v.data, &end);
            if (end > v.data && *(end - 1) == '\0') {
                // conversion is success
                return d;
            } else {
                // conversion failed
                return NAN;
            }
        }
        case INTS:
            return *(int*)(v.data);
        case FLOATS:
            return *(float*)(v.data);
        case DATE: {
            // 这里date已经被转换为uint32了，按照uint32处理
            return *(unsigned int*)(v.data);
        }
        case UNEVALUATED:
        case UNDEFINED:
        case ATTR_TABLE:
            return NAN;
    }
    assert(0);
}

// 比较两个SQL标量，返回1表示大于，0表示等于，-1表示小于，其他值表示无法比较。
static int scalar_compare(const Value &v1, const Value &v2) {
    // 两个字符串相比时，按照字符串比较处理
    if (v1.type == CHARS && v2.type == CHARS) {
        int cmp = strcmp((char*)v1.data, (char*)v2.data);
        if (cmp > 0)  return 1;
        if (cmp == 0)  return 0;
        if (cmp < 0)  return -1;
    }

    // 否则，将按照C类型提升规则进行比较
    // 在当前情况里，所有值均可被float64表示，因此全部转换为double再比较即可
    double d1, d2;
    d1 = scalar_to_double(v1);
    d2 = scalar_to_double(v2);
    if (std::isnan(d1) || std::isnan(d2)) return 0xFF;
    if (d1 - d2 > -1e-20 && d1 - d2 < 1e-20) {
        return 0;
    } else if (d1 > d2) {
        return 1;
    } else if (d1 < d2) {
        return -1;
    }
    return 0xFE;
}

// 比较两个SQL矢量，返回1表示大于，0表示等于，-1表示小于，其他值表示无法比较。
static int vector_compare(const TupleSet &v1, const TupleSet &v2, CompOp op) {
    // 两个表的列不兼容，直接返回错误
    if (v1.get_schema().fields().size() > 1 || v2.get_schema().fields().size() > 1) {
        // illegal according to SQL specification
        return 0xFB;
    }
    // 有一个矢量是空的，按null处理
    if (op == CompOp::NO_OP)  return 0;
    if (v1.size() == 0 || v2.size() == 0) {
        if (op == CONTAINED_BY || op == NOT_CONTAINED_BY) {
            // 根据mariadb，两边只要出现一个空就是假的，这里直接返回了
            return 0;
        }
        // 无法比较，直接返回比较失败
        return 0xFC;
    }

    // 两边都是平凡矢量，即都是标量，按照标量比较规则进行比较
    if (v1.size() == 1 && v2.size() == 1) {
        auto &t1 = v1.get(0), &t2 = v2.get(0);
        if (t1.size() != 1 || t2.size() != 1) {
            // 列数超过1的矢量无法参与比较
            return 0xFB;
        }
        // 两边都是1x1的二维矢量
        // 退化为标量比较
        // 除了IN之外的任意运算符作用到两个平凡矢量上，等价于作用到他们对应的标量上
        // 如果是IN，那么此时标量相等<=>矢量包含
        Value flatten_v1, flatten_v2;
        if (v1.flatten(flatten_v1) && v2.flatten(flatten_v2)) {
            int compare_result = scalar_compare(flatten_v1, flatten_v2);
            if (op == CONTAINED_BY || op == NOT_CONTAINED_BY) {
                bool flag_compare = compare_result == 0;
//                if (op == NOT_CONTAINED_BY) {
//                    flag_compare = !flag_compare; // 了转反
//                }
                if (flag_compare) {
                    return 1;
                } else {
                    return 0;
                }
            }
            return compare_result;
        }
    }

    // 两边都不是平凡矢量，按照SQL规则，无法比较
    if (v1.size() > 1 && v2.size() > 1) {
        return 0xFB;
    }

    // 一个是平凡矢量，一个不是平凡矢量，部分情况下可以比较
    // 由于CONTAINED_BY运算符的特殊性，需要与其他运算符区分开，因此这里需要知道具体的运算符
    int result = 0;
    switch (op) {
        case NOT_CONTAINED_BY: // 比较结果不反转，在filter里面再反转
        case CONTAINED_BY: {
            // WHERE x IN y, y must be a table, x must be a scalar or an ordinary 1-dim vector
            result = 1; // initially as true (right contains left)
            for (auto &ele : v1.tuples()) {
                const auto &vec = v2.tuples();

                Tuple::clear();
                Tuple::append_order_attr(0,0);
                if (std::find(vec.begin(), vec.end(), ele) == vec.end()) {
                    // right does not contain this value
                    // set to false
                    result = 0;
                    break;
                }
            }
        }
        break;
        case EQUAL_TO:
        case LESS_EQUAL:
        case NOT_EQUAL:
        case LESS_THAN:
        case GREAT_EQUAL:
        case GREAT_THAN:
        case NO_OP:
            // 非平凡矢量无法被这些运算符作用，违反比较规则
            result = 0xFB;
    }
    return result;
}

bool DefaultConditionFilter::filter(const Record &rec, RC &err) const
{
    err = RC::SUCCESS; // 如果后续不设置为failure，默认为成功
  char *left_value = nullptr;
  char *right_value = nullptr;
  bool left_is_null = false;
  bool right_is_null = false;
  if (left_.is_attr) {  // value
      left_is_null = ((*(unsigned *) (rec.data + left_.attr_offset-4))&0x00000001==0x00000001);
    left_value = (char *)(rec.data + left_.attr_offset);
  } else {
      left_is_null = left_.is_null;
      left_value = (char *)left_.value;
  }

  if (right_.is_attr) {
      right_is_null = ((*(unsigned *) (rec.data + right_.attr_offset-4))&0x00000001==0x00000001);
    right_value = (char *)(rec.data + right_.attr_offset);
  } else {
      right_is_null = right_.is_null;
    right_value = (char *)right_.value;
  }

    //null的比较逻辑
    if (left_is_null||right_is_null) {
        if (comp_op_==IS_CompOP||comp_op_==IS_NOT_CompOP) {
            switch (comp_op_) {
                case IS_CompOP:
                    if (left_is_null&&right_is_null)
                        return true;
                    else
                        return false;
                    break;
                case IS_NOT_CompOP:
                    if(left_is_null&&right_is_null)
                        return false;
                    else if(left_is_null&&!right_is_null)
                        return false;
                    else if(!left_is_null&&right_is_null)
                        return true;
                    else
                        return false;
                    break;
                default:
                    return false;
                    break;
            }
        }
        else {
            //用其他符号对null进行比较一律返回false
            return false;
        }
    }

    // 直接把两边的数据都转成矢量，按照矢量比较规则进行比较
    TupleSet *tuples_left = nullptr, *tuples_right = nullptr;
    bool free_left = false, free_right = false; // 是否需要free

    if (type_left == ATTR_TABLE) {
        tuples_left = (TupleSet*)left_value;
    } else {
        // left is a scalar value
        free_left = true;
        tuples_left = new TupleSet();
        tuples_left->add(Tuple(type_left, left_value));
        TupleSchema schema;
        schema.add(type_left, "<anonymous table>", "<anonymous field>", false);
        tuples_left->set_schema(schema);
    }

    if (type_right == ATTR_TABLE) {
        tuples_right = (TupleSet*)right_value;
    } else {
        // right is a scalar value
        free_right = true;
        tuples_right = new TupleSet();
        tuples_right->add(Tuple(type_right, right_value));
        TupleSchema schema;
        schema.add(type_right, "<anonymous table>", "<anonymous field>", false);
        tuples_left->set_schema(schema);
    }



    int compare_result = vector_compare(*tuples_left, *tuples_right, comp_op_);

    if (free_left) {
        delete tuples_left;
        tuples_left = nullptr;
    }
    if (free_right) {
        delete tuples_right;
        tuples_right = nullptr;
    }

    if (compare_result == 0xFB)  err = RC::INVALID_ARGUMENT; // 根据SQL规范，这个比较是错误的，需要将查询返回失败
    if (compare_result < -1 || compare_result > 1)  return false; // 比较时出现错误，直接跳过
    switch (comp_op_) {
        case EQUAL_TO:
            return compare_result == 0;
        case LESS_EQUAL:
            return compare_result <= 0;
        case NOT_EQUAL:
            return compare_result != 0;
        case LESS_THAN:
            return compare_result < 0;
        case GREAT_EQUAL:
            return compare_result >= 0;
        case GREAT_THAN:
            return compare_result > 0;
        case CONTAINED_BY:
            return compare_result != 0;
        case NOT_CONTAINED_BY:
            return compare_result == 0;
        case NO_OP:
            return false;
    }
    return false;
}

CompositeConditionFilter::~CompositeConditionFilter() {
    if (memory_owner_) {
        delete[] filters_;
        filters_ = nullptr;
    }
}

RC CompositeConditionFilter::init(const ConditionFilter *filters[], int filter_num, bool own_memory) {
    filters_ = filters;
    filter_num_ = filter_num;
    memory_owner_ = own_memory;
    return RC::SUCCESS;
}

RC CompositeConditionFilter::init(const ConditionFilter *filters[], int filter_num) {
    return init(filters, filter_num, false);
}

RC CompositeConditionFilter::init(Table &table, const Condition *conditions, int condition_num, Trx *trx) {
    if (condition_num == 0) {
        return RC::SUCCESS;
    }
    if (conditions == nullptr) {
        return RC::INVALID_ARGUMENT;
    }

    RC rc = RC::SUCCESS;
    ConditionFilter **condition_filters = new ConditionFilter *[condition_num];
    for (int i = 0; i < condition_num; i++) {
        DefaultConditionFilter *default_condition_filter = new DefaultConditionFilter();
        rc = default_condition_filter->init(table, conditions[i], trx);
        if (rc != RC::SUCCESS) {
            delete default_condition_filter;
            for (int j = i - 1; j >= 0; j--) {
                delete condition_filters[j];
                condition_filters[j] = nullptr;
            }
            delete[] condition_filters;
            condition_filters = nullptr;
            return rc;
        }
        condition_filters[i] = default_condition_filter;
    }
    return init((const ConditionFilter **) condition_filters, condition_num, true);
}

bool CompositeConditionFilter::filter(const Record &rec, RC &err) const {
    for (int i = 0; i < filter_num_; i++) {
        if (!filters_[i]->filter(rec, err)) {
            return false;
        }
    }
    return true;
}
