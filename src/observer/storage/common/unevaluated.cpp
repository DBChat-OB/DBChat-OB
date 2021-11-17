//
// Created by Keuin on 2021/11/11.
// 计算Unevaluated类型的值
//


#include "unevaluated.h"
#include "sql/executor/execute_stage.h"

namespace unevaluated {
    RC eval(const Value &val, const Table &table, Trx *trx, Value &value) {
        if (val.type != UNEVALUATED)  return RC::INVALID_ARGUMENT; // can only evaluate unevaluated values
        auto *uneval = static_cast<Unevaluated *>(val.data);
        if (uneval->type == UE_SELECT) {
            // 嵌套SELECT语句，现在执行子查询语句，返回查询结果作为新的值
            auto &select = uneval->data.select;
            TupleSet tuples;
            RC err;
            if ((err = ExecuteStage::sub_select(select, tuples, trx)) != RC::SUCCESS) {
                LOG_ERROR("Failed to run sub query.");
                return err;
            }
            auto *data = new TupleSet(std::move(tuples));
            value.type = ATTR_TABLE;
            value.data = data;
            return RC::SUCCESS;
        } else {
            LOG_ERROR("Missing implementation.");
            assert(0); // 未实现的新类型，UE_*
        }
    }
}
