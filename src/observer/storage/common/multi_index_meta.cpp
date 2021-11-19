//
// Created by jeremy on 2021/11/19.
//
#include "storage/common/multi_index_meta.h"
#include "storage/common/field_meta.h"
#include "storage/common/table_meta.h"
#include "common/lang/string.h"
#include "common/log/log.h"
#include "rc.h"
#include "json/json.h"
#include <vector>

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_FIELDS("fields");

RC MultiIndexMeta::init(const char *name, const std::vector <FieldMeta> field_metas) {
    if (nullptr == name || common::is_blank(name)) {
        return RC::INVALID_ARGUMENT;
    }
    name_ = name;
    fields_ = field_metas;
    return RC::SUCCESS;
}

void MultiIndexMeta::to_json(Json::Value &json_value) const {
    json_value[FIELD_NAME] = name_;
    Json::Value fields_value;
    for (const FieldMeta & field : fields_) {
        Json::Value field_value;
        field.to_json(field_value);
        fields_value.append(std::move(field_value));
    }
    json_value[FIELD_FIELDS] = std::move(fields_value);
}

RC MultiIndexMeta::from_json(const TableMeta &table, const Json::Value &json_value, MultiIndexMeta &index) {
    const Json::Value &name_value = json_value[FIELD_NAME];
    if (!name_value.isString()) {
        LOG_ERROR("Index name is not a string. json value=%s", name_value.toStyledString().c_str());
        return RC::GENERIC_ERROR;
    }
    const Json::Value &fields_value = json_value[FIELD_FIELDS];
    if (!fields_value.isArray() || fields_value.size() <= 0) {
        LOG_ERROR("Invalid multi-index meta. fields is not array, json value=%s", fields_value.toStyledString().c_str());
        return RC::GENERIC_ERROR;
    }

    RC rc = RC::SUCCESS;
    int field_num = fields_value.size();
    std::vector<FieldMeta> fields(field_num);
    for (int i = 0; i < field_num; i++) {
        FieldMeta &field = fields[i];

        const Json::Value &field_value = fields_value[i];
        rc = FieldMeta::from_json(field_value, field);
        if (rc != RC::SUCCESS) {
            LOG_ERROR("Failed to deserialize multi-index meta. index name =%s", name_value.asCString());
            return RC::GENERIC_ERROR;
        }
    }

    return index.init(name_value.asCString(), fields);
}

const char *MultiIndexMeta::name() const {
    return name_.c_str();
}

void MultiIndexMeta::fields(std::vector<FieldMeta> &fields) const {
    fields.assign(fields_.begin(),fields_.end());
}

void MultiIndexMeta::desc(std::ostream &os) const {
    os << "index name=" << name_;
    os << "fields:";
    for(const FieldMeta & field : fields_) {
        os<<field.name()<<"\t";
    }
}
