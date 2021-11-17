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
#define EPSILON 1e-6

#include <iostream>
#include <iomanip>
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

    virtual int getIValue() const = 0;

    virtual float getFValue() const = 0;

    virtual void get_data(void *&ptr) const = 0;

    void set_null(bool null_attr);

    virtual bool is_null()const = 0;

protected:
    bool null_attr_;

private:
};

class IntValue : public TupleValue {
public:
    explicit IntValue(int value, bool null_attr) : value_(value) {
        null_attr_ = null_attr;
    }

    void to_string(std::ostream &os) const override {
        if(null_attr_) {
            os << "null";
        }
        else {
            os << value_;
        }
    }

    int getIValue() const override {
        return value_;
    }

    float getFValue() const override {
        return float(value_);
    }

    int compare(const TupleValue &other) const override {
        const IntValue &int_other = (const IntValue &) other;
        return value_ - int_other.value_;
    }

    void get_data(void *&ptr) const override {
        ptr = (void *) &value_;
    }

    void set_null(bool null_attr) {
        null_attr_ = null_attr;
    }

    bool is_null() const override{
        return null_attr_;
    }

private:
    int value_;
};

class FloatValue : public TupleValue {
public:
    explicit FloatValue(float value, bool null_attr) : value_(value) {
        null_attr_ = null_attr;
    }

    void to_string(std::ostream &os) const override {
        if(null_attr_){
            os << "null" ;
        }
        else{
            float a = value_;
            bool flag = true;
            int z[3];
            z[0] = int(a);
            a = a - (int) a;
            for (int i = 0; i < 2; i++) {
                a *= 10;
                z[i + 1] = int(a);
                if (a - (int) a == 0) {
                    flag = false;
                }
                a = a - (int) a;
            }
            if (flag) {//如何输出两位
                if (a * 10 >= 5) {
                    z[2] += 1;
                }
                float ret = z[0] * 1.0 + z[1] * 0.1 + z[2] * 0.01;
                os << ret;
            } else {
                os << value_;
            }
        }

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

        if (result > EPSILON) {
            return 1;
        }
        if (result < -EPSILON) {
            return -1;
        }
        return 0;
    }

    void  get_data (void * &ptr) const  override {
        ptr = (void *)&value_;
    }

    void set_null(bool null_attr) {
        null_attr_ = null_attr;
    }

    bool is_null() const override{
        return null_attr_;
    }

public:
    float value_;
};

class StringValue : public TupleValue {
public:
    StringValue(const char *value, int len, bool null_attr) : value_(value, len) {
        memset(value_c_str,0,20);
        strcpy(value_c_str,value);
        null_attr_ = null_attr;
    }

    explicit StringValue(const char *value) : value_(value) {
    }

    void to_string(std::ostream &os) const override {
        if(null_attr_) {
            os << "null";
        }
        else {
            os << value_;
        }
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

    void  get_data (void * &ptr) const  override {
        ptr = (void *)&value_c_str;
    }

    void set_null(bool null_attr) {
        null_attr_ = null_attr;
    }

    bool is_null() const override{
        return null_attr_;
    }

private:
    std::string value_;
    char  value_c_str[20];
};

class DateValue : public TupleValue {
public:
    explicit DateValue(unsigned int value, bool null_attr) : value_(value) {
        null_attr_ = null_attr;
    }
    explicit DateValue(time_t value) {
        this->value_ = value&&0x00000000ffffffff;
    }
    void to_string(std::ostream &os) const override {
        if(null_attr_) {
            os << "null";
        }
        else {
            time_t time = value_;
            struct tm *timeinfo = gmtime(&time); //使用gmt时间
            char ret[20];
            strftime(ret, 20, "%Y-%m-%d", timeinfo); //格式化输出时间
            os << ret;
        }
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

    void  get_data (void * &ptr) const  override {
        ptr = (void *)&value_;
    }

    void set_null(bool null_attr){
        null_attr_ = null_attr;
    }

    bool is_null() const override{
        return null_attr_;
    }

private:
    unsigned int value_;
};

#endif //__OBSERVER_SQL_EXECUTOR_VALUE_H_
