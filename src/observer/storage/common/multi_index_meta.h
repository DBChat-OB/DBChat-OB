//
// Created by jeremy on 2021/11/19.
//

#ifndef MINIDB_MULTI_INDEX_META_H
#define MINIDB_MULTI_INDEX_META_H
#include <string>
#include "rc.h"

class TableMeta;
class FieldMeta;

namespace Json {
    class Value;
} // namespace Json

class MultiIndexMeta {
public:
    MultiIndexMeta() = default;

    RC init(const char *name, );

public:
    const char *name() const;
    const char *field() const;
    void desc(std::ostream &os) const;
    bool is_unique() const;
public:
    void to_json(Json::Value &json_value) const;
    static RC from_json(const TableMeta &table, const Json::Value &json_value, IndexMeta &index);

private:
    std::string       name_;
    std::string       field_;
    bool              unique_;
};
#endif //MINIDB_MULTI_INDEX_META_H
