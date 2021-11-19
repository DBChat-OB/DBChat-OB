//
// Created by jeremy on 2021/11/19.
//

#ifndef MINIDB_MULTI_INDEX_META_H
#define MINIDB_MULTI_INDEX_META_H
#include <string>
#include "rc.h"
#include <vector>

class TableMeta;
class FieldMeta;

namespace Json {
    class Value;
} // namespace Json

class MultiIndexMeta {
public:
    MultiIndexMeta() = default;

    RC init(const char *name, const std::vector<FieldMeta> field_metas);

public:

    const char *name() const;
    void fields(std::vector<FieldMeta> &fields) const;
    void desc(std::ostream &os) const;

public:
    void to_json(Json::Value &json_value) const;
    static RC from_json(const TableMeta &table, const Json::Value &json_value, MultiIndexMeta &index);

private:
    std::string       name_;
    std::vector<FieldMeta> fields_;

};
#endif //MINIDB_MULTI_INDEX_META_H
