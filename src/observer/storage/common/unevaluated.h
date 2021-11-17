//
// Created by Keuin on 2021/11/11.
//

#ifndef MINIDB_UNEVALUATED_H
#define MINIDB_UNEVALUATED_H

#include <sql/parser/parse_defs.h>
#include <cassert>
#include "table.h"

namespace unevaluated {
    Value eval(const Value &val, const Table &table);
}

#endif //MINIDB_UNEVALUATED_H
