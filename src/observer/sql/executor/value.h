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

#ifndef __OBSERVER_SQL_EXECUTOR_VALUE_H_
#define __OBSERVER_SQL_EXECUTOR_VALUE_H_

#include <string.h>

#include <string>
#include <ostream>

class FloatValue;

//用来构成tuple的value
class TupleValue {
public:
    TupleValue() = default;

    virtual ~TupleValue() = default;

    virtual void to_string(std::ostream &os) const = 0;

    virtual int compare(const TupleValue &other) const = 0;

    virtual int getIValue()const = 0;

    virtual float getFValue()const = 0;

private:
};

class IntValue : public TupleValue {
public:
    explicit IntValue(int value) : value_(value) {
    }

    void to_string(std::ostream &os) const override {
        os << value_;
    }

    int getIValue()  const override {
        return value_;
    }
    float getFValue()  const override {
        return float(value_);
    }
    int compare(const TupleValue &other) const override {

        const IntValue &int_other = (const IntValue &) other;
        return value_ - int_other.value_;


    }

private:
    int value_;
};

class FloatValue : public TupleValue {
public:
    explicit FloatValue(float value) : value_(value) {
    }

    void to_string(std::ostream &os) const override {
        os << value_;
    }

    int getIValue()  const override {
        return value_;
    }
    float getFValue()  const override {
        return (value_);
    }

    int compare(const TupleValue &other) const override {
        const FloatValue &float_other = (const FloatValue &) other;
        float result = value_ - float_other.value_;

        if (result > 0) { // 浮点数没有考虑精度问题
            return 1;
        }
        if (result < 0) {
            return -1;
        }
        return 0;
    }

public:
    float value_;
};

class StringValue : public TupleValue {
public:
    StringValue(const char *value, int len) : value_(value, len) {
    }

    explicit StringValue(const char *value) : value_(value) {
    }

    void to_string(std::ostream &os) const override {
        os << value_;
    }
    int getIValue()  const override {
        return -1;
    }
    float getFValue()  const override {
        return -1;
    }
    int compare(const TupleValue &other) const override {
        const StringValue &string_other = (const StringValue &) other;
        return strcmp(value_.c_str(), string_other.value_.c_str());
    }

private:
    std::string value_;
};

class DateValue : public TupleValue {
public:
    explicit DateValue(unsigned int value) : value_(value) {
    }
    explicit DateValue(time_t value) {
        this->value_ = value&&0x00000000ffffffff;
    }
    void to_string(std::ostream &os) const override {
        time_t time = value_;
        struct tm * timeinfo = gmtime(&time); //使用gmt时间
        char ret [20];
        strftime(ret,20,"%Y-%m-%d",timeinfo); //格式化输出时间
        os << ret;
    }

    int getIValue()  const override {
        return value_;
    }
    float getFValue()  const override {
        return float(value_);
    }
    int compare(const TupleValue &other) const override {

        const DateValue &date_other = (const DateValue &) other;
        return value_ - date_other.value_;
    }

private:
    unsigned int value_;
};

#endif //__OBSERVER_SQL_EXECUTOR_VALUE_H_
