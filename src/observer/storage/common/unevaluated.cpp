//
// Created by Keuin on 2021/11/11.
// 计算Unevaluated类型的值
//


#include "unevaluated.h"

namespace unevaluated {
    Value eval(const Value &val, const Table &table) {
        assert(val.type == UNEVALUATED);
        auto *uneval = static_cast<Unevaluated *>(val.data);
        if (uneval->type == UE_SELECT) {
            // 嵌套SELECT语句，现在执行子查询语句，返回查询结果作为新的值
            auto &select = uneval->data.select;

        } else {
            assert(0); // 未实现的新类型，UE_*
        }
    }
}
