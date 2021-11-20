//
// Created by jeremy on 2021/11/20.
//

#ifndef MINIDB_MULTI_BPLUS_TREE_INDEX_H
#define MINIDB_MULTI_BPLUS_TREE_INDEX_H

#include "storage/common/index.h"
#include "storage/common/bplus_tree.h"
#include "storage/common/multi_index_meta.h"

class MultiBplusTreeIndex : public Index {
public:
    MultiBplusTreeIndex() = default;
    virtual ~MultiBplusTreeIndex() noexcept;

    RC create(const char *file_name, const MultiIndexMeta &multi_index_meta);
    RC open(const char *file_name, const MultiIndexMeta &multi_index_meta);
    RC close();

    RC insert_entry(const char *record, const RID *rid) override;
    RC delete_entry(const char *record, const RID *rid) override;

    IndexScanner *create_scanner(CompOp comp_op, const char *value, bool value_is_null) override;

    RC sync() override;

private:
    bool inited_ = false;
    BplusTreeHandler index_handler_;
};
#endif //MINIDB_MULTI_BPLUS_TREE_INDEX_H
