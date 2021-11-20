//
// Created by jeremy on 2021/11/20.
//

#include "storage/common/multi_bplus_tree_index.h"
#include "common/log/log.h"
#include "bplus_tree_index.h"
#include <vector>

MultiBplusTreeIndex::~MultiBplusTreeIndex() noexcept {
    close();
}

RC MultiBplusTreeIndex::create(const char *file_name, const MultiIndexMeta & multi_index_meta) {
    if (inited_) {
        return RC::RECORD_OPENNED;
    }

    RC rc = Index::init(multi_index_meta);
    if (rc != RC::SUCCESS) {
        return rc;
    }
    std::vector<FieldMeta> field_metas;
    multi_index_meta_.fields(field_metas);
    int attr_length = 0;
    for(auto i : field_metas) {
        attr_length += i.len();
    }
    rc = index_handler_.create(file_name, MULTI_INDEX_FIELD, attr_length);
    if (RC::SUCCESS == rc) {
        inited_ = true;
    }
    return rc;
}

RC MultiBplusTreeIndex::open(const char *file_name, const MultiIndexMeta &multi_index_meta) {
    if (inited_) {
        return RC::RECORD_OPENNED;
    }
    RC rc = Index::init(multi_index_meta);
    if (rc != RC::SUCCESS) {
        return rc;
    }

    rc = index_handler_.open(file_name);
    if (RC::SUCCESS == rc) {
        inited_ = true;
    }
    return rc;
}

RC MultiBplusTreeIndex::close() {
    if (inited_) {
        index_handler_.close();
        inited_ = false;
    }
    return RC::SUCCESS;
}

RC MultiBplusTreeIndex::insert_entry(const char *record, const RID *rid) {
    std::vector<FieldMeta> field_metas;
    multi_index_meta_.fields(field_metas); // 得到多列索引所有的fields
    int fields_num = field_metas.size(); //计算拼接后的总长度(列
    //拼接key
    char * pkey = (char *)malloc(fields_num*8);
    for(int i = 0;i<fields_num;i++) {
        memcpy(pkey+i*4, record+field_metas.at(i).offset()-4,8);
    }
    //将拼接好的key插入到树中
    RC rc = index_handler_.insert_entry(pkey,rid, false);
    free(pkey);
    return rc;
}

RC MultiBplusTreeIndex::delete_entry(const char *record, const RID *rid) {
    std::vector<FieldMeta> field_metas;
    multi_index_meta_.fields(field_metas); // 得到多列索引所有的fields
    int fields_num = field_metas.size(); //计算拼接后的总长度（列）
    //拼接key
    char * pkey = (char *)malloc(fields_num*8);
    for(int i = 0;i<fields_num;i++) {
        memcpy(pkey+i*4, record+field_metas.at(i).offset()-4,8);
    }
    //将拼接好的key传入到btree中删除
    RC rc = index_handler_.delete_entry(pkey,rid);
    free(pkey);
    return rc;
}

IndexScanner *MultiBplusTreeIndex::create_scanner(CompOp comp_op, const char *value, bool value_is_null) {
    BplusTreeScanner *bplus_tree_scanner = new BplusTreeScanner(index_handler_);
    std::vector<FieldMeta> field_metas;
    multi_index_meta_.fields(field_metas); // 得到多列索引所有的fields
    int fields_num = field_metas.size(); //计算拼接后的总长度
    char * temp_value = (char *) malloc(fields_num*4);
    memcpy(temp_value,value,4);
    memset(temp_value+4,0,fields_num*4-4);
    RC rc = bplus_tree_scanner->open(comp_op, temp_value, value_is_null);
    if (rc != RC::SUCCESS) {
        LOG_ERROR("Failed to open index scanner. rc=%d:%s", rc, strrc(rc));
        delete bplus_tree_scanner;
        free(temp_value);
        return nullptr;
    }
    BplusTreeIndexScanner *index_scanner = new BplusTreeIndexScanner(bplus_tree_scanner);
    free(temp_value);
    return index_scanner;
}

RC MultiBplusTreeIndex::sync() {
    return index_handler_.sync();
}