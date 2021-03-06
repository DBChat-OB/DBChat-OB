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
// Created by Wangyunlai on 2021/5/13.
//

#include <limits.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <sql/executor/tuple.h>

#include "storage/common/table.h"
#include "storage/common/table_meta.h"
#include "common/log/log.h"
#include "common/lang/string.h"
#include "storage/default/disk_buffer_pool.h"
#include "storage/common/record_manager.h"
#include "storage/common/condition_filter.h"
#include "storage/common/meta_util.h"
#include "storage/common/index.h"
#include "storage/common/bplus_tree_index.h"
#include "storage/trx/trx.h"
#include "storage/common/mytime.cpp"

#include "sql//executor/execute_stage.h"
Table::Table() :
    data_buffer_pool_(nullptr),
    file_id_(-1),
    record_handler_(nullptr),
    is_opened(false),
    heap_manager(nullptr) {
}

Table::~Table() {
  delete record_handler_;
  record_handler_ = nullptr;

  if (data_buffer_pool_ != nullptr && file_id_ >= 0) {
    data_buffer_pool_->close_file(file_id_);
    data_buffer_pool_ = nullptr;
    delete this->heap_manager;
    this->heap_manager = nullptr;
  }

  LOG_INFO("Table has been closed: %s", name());
}

RC Table::create(const char *path, const char *name, const char *base_dir, int attribute_count, const AttrInfo attributes[]) {

  if (nullptr == name || common::is_blank(name)) {
    LOG_WARN("Name cannot be empty");
    return RC::INVALID_ARGUMENT;
  }
  LOG_INFO("Begin to create table %s:%s", base_dir, name);

  if (attribute_count <= 0 || nullptr == attributes) {
    LOG_WARN("Invalid arguments. table_name=%s, attribute_count=%d, attributes=%p",
        name, attribute_count, attributes);
    return RC::INVALID_ARGUMENT;
  }

  RC rc = RC::SUCCESS;

  // 使用 table_name.table记录一个表的元数据
  // 判断表文件是否已经存在

  int fd = ::open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
  if (-1 == fd) {
    if (EEXIST == errno) {
      LOG_ERROR("Failed to create table file, it has been created. %s, EEXIST, %s",
                path, strerror(errno));
      return RC::SCHEMA_TABLE_EXIST;
    }
    LOG_ERROR("Create table file failed. filename=%s, errmsg=%d:%s", 
       path, errno, strerror(errno));
    return RC::IOERR;
  }

  close(fd);

  // 创建文件
  if ((rc = table_meta_.init(name, attribute_count, attributes)) != RC::SUCCESS) {
    LOG_ERROR("Failed to init table meta. name:%s, ret:%d", name, rc);
    return rc; // delete table file
  }

  std::fstream fs;
  fs.open(path, std::ios_base::out | std::ios_base::binary);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open file for write. file name=%s, errmsg=%s", path, strerror(errno));
    return RC::IOERR;
  }

  // 记录元数据到文件中
  table_meta_.serialize(fs);
  fs.close();

  std::string data_file = std::string(base_dir) + "/" + name + TABLE_DATA_SUFFIX;
  data_buffer_pool_ = theGlobalDiskBufferPool();
  rc = data_buffer_pool_->create_file(data_file.c_str());
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to create disk buffer pool of data file. file name=%s", data_file.c_str());
    return rc;
  }

  rc = init_record_handler(base_dir);

  base_dir_ = base_dir;
  LOG_INFO("Successfully create table %s:%s", base_dir, name);
  return rc;
}

RC Table::open(const char *meta_file, const char *base_dir) {
  // 加载元数据文件
  std::fstream fs;
  std::string meta_file_path = std::string(base_dir) + "/" + meta_file;
  fs.open(meta_file_path, std::ios_base::in | std::ios_base::binary);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open meta file for read. file name=%s, errmsg=%s", meta_file, strerror(errno));
    return RC::IOERR;
  }
  if (table_meta_.deserialize(fs) < 0) {
    LOG_ERROR("Failed to deserialize table meta. file name=%s", meta_file);
    return RC::GENERIC_ERROR;
  }
  fs.close();

  // 加载数据文件
  RC rc = init_record_handler(base_dir);

  base_dir_ = base_dir;

  const int index_num = table_meta_.index_num();
  for (int i = 0; i < index_num; i++) {
    const IndexMeta *index_meta = table_meta_.index(i);
    const FieldMeta *field_meta = table_meta_.field(index_meta->field());
    if (field_meta == nullptr) {
      LOG_PANIC("Found invalid index meta info which has a non-exists field. table=%s, index=%s, field=%s",
                name(), index_meta->name(), index_meta->field());
      return RC::GENERIC_ERROR;
    }

    BplusTreeIndex *index = new BplusTreeIndex();
    std::string index_file = index_data_file(base_dir, name(), index_meta->name());
    rc = index->open(index_file.c_str(), *index_meta, *field_meta);
    if (rc != RC::SUCCESS) {
      delete index;
      LOG_ERROR("Failed to open index. table=%s, index=%s, file=%s, rc=%d:%s",
                name(), index_meta->name(), index_file.c_str(), rc, strrc(rc));
      return rc;
    }
    indexes_.push_back(index);
  }
    //加载多列索引
    const int multi_index_num = table_meta_.multi_index_num();
    for (int i = 0; i < multi_index_num; i++) {
        const MultiIndexMeta *multi_index_meta = table_meta_.multi_index(i);

        MultiBplusTreeIndex *multi_index = new MultiBplusTreeIndex();
        std::string index_file = index_data_file(base_dir, name(), multi_index_meta->name());
        rc = multi_index->open(index_file.c_str(), *multi_index_meta);
        if (rc != RC::SUCCESS) {
            delete multi_index;
            LOG_ERROR("Failed to open index. table=%s, index=%s, file=%s, rc=%d:%s",
                      name(), multi_index_meta->name(), index_file.c_str(), rc, strrc(rc));
            return rc;
        }
        indexes_.push_back(multi_index);
    }
  // 加载TEXT堆
  rc = open_heap_manager();

  is_opened = (rc == RC::SUCCESS);
  return rc;
}

RC Table::open_heap_manager() {
    RC rc;
    std::string heap_file_path(base_dir_);
    heap_file_path.append("/");
    heap_file_path.append(name());
    heap_file_path.append(".heap");
    this->heap_manager = new HeapManager(heap_file_path.c_str());
    if ((rc = this->heap_manager->init()) != RC::SUCCESS) {
        LOG_INFO("Failed to initialize heap manager. table=%s, file=%s, rc=%d:%s.",
                  name(), heap_file_path.c_str(), rc, strrc(rc));
        return rc;
    }
    LOG_INFO("Heap manager is opened. table=%s", name());
    return rc;
}

RC Table::commit_insert(Trx *trx, const RID &rid) {
  Record record;
  RC rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return trx->commit_insert(this, record);
}

RC Table::rollback_insert(Trx *trx, const RID &rid) {

  Record record;
  RC rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  // remove all indexes
  rc = delete_entry_of_indexes(record.data, rid, false);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to delete indexes of record(rid=%d.%d) while rollback insert, rc=%d:%s",
              rid.page_num, rid.slot_num, rc, strrc(rc));
  } else {
    rc = record_handler_->delete_record(&rid);
  }
  return rc;
}

RC Table::insert_record(Trx *trx, Record *record) {
  RC rc = RC::SUCCESS;

  if (trx != nullptr) {
    trx->init_trx_info(this, *record);
  }
  rc = record_handler_->insert_record(record->data, table_meta_.record_size(), &record->rid);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Insert record failed. table name=%s, rc=%d:%s", table_meta_.name(), rc, strrc(rc));
    return rc;
  }

  if (trx != nullptr) {
    rc = trx->insert_record(this, record);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to log operation(insertion) to trx");

      RC rc2 = record_handler_->delete_record(&record->rid);
      if (rc2 != RC::SUCCESS) {
        LOG_PANIC("Failed to rollback record data when insert index entries failed. table name=%s, rc=%d:%s",
                  name(), rc2, strrc(rc2));
      }
      return rc;
    }
  }

  rc = insert_entry_of_indexes(record->data, record->rid);
  if (rc != RC::SUCCESS) {
    RC rc2 = delete_entry_of_indexes(record->data, record->rid, true);
    if (rc2 != RC::SUCCESS) {
      LOG_PANIC("Failed to rollback index data when insert index entries failed. table name=%s, rc=%d:%s",
                name(), rc2, strrc(rc2));
    }
    rc2 = record_handler_->delete_record(&record->rid);
    if (rc2 != RC::SUCCESS) {
      LOG_PANIC("Failed to rollback record data when insert index entries failed. table name=%s, rc=%d:%s",
                name(), rc2, strrc(rc2));
    }
    return rc;
  }
  return rc;
}
RC Table::insert_record(Trx *trx, int value_num, const Value *values) {
  if (value_num <= 0 || nullptr == values ) {
    LOG_ERROR("Invalid argument. value num=%d, values=%p", value_num, values);
    return RC::INVALID_ARGUMENT;
  }

  char *record_data;
  RC rc = make_record(value_num, values, record_data);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to create a record. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  Record record;
  record.data = record_data;
  // record.valid = true;
  rc = insert_record(trx, &record);
  delete[] record_data;
  return rc;
}

const char *Table::name() const {
  return table_meta_.name();
}

const TableMeta &Table::table_meta() const {
  return table_meta_;
}

RC Table::make_record(int value_num, const Value *values, char * &record_out) {
  // 检查字段类型是否一致
  if (value_num + table_meta_.sys_field_num() != table_meta_.field_num()) {
    return RC::SCHEMA_FIELD_MISSING;
  }

  const int normal_field_start_index = table_meta_.sys_field_num();
  for (int i = 0; i < value_num; i++) {
    const FieldMeta *field = table_meta_.field(i + normal_field_start_index);
    const Value &value = values[i];
    if (!field->nullable()&&value.null_attr) {
        return RC::CONSTRAINT_NOTNULL;
    }
    if (field->type() != value.type) {
        if(value.null_attr)
            continue;
        if(field->type()==DATE&&value.type==CHARS){
            continue;
        }
        if (field->type() == TEXTS && value.type == CHARS) {
            // 超长字符串在SQL解析时和普通字符串的类型是一样的，只不过截断长度不一样、存储方式不一样（需要一个间接寻址）
            continue;
        }
      LOG_ERROR("Invalid value type. field name=%s, type=%d, but given=%d",
        field->name(), field->type(), value.type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  }

  // 复制所有字段的值
  int record_size = table_meta_.record_size();
  char *record = new char [record_size];
  unsigned int null_attr_true = 0x00000001;
  unsigned int null_attr_false = 0x00000000;
  for (int i = 0; i < value_num; i++) {
    const FieldMeta *field = table_meta_.field(i + normal_field_start_index);
    const Value &value = values[i];
    if (value.null_attr) {
        memcpy(record + field->offset()-4, & null_attr_true, 4);
        continue;
    }
    else {
        memcpy(record + field->offset()-4, & null_attr_false, 4);
    }
    if(field->type()==DATE&&value.type==CHARS){
        time_t time_value;
        if(mytime::chars_to_date((char*)value.data, time_value))
        {
            unsigned int time_int = time_value;
            memcpy(record + field->offset(), & time_int, field->len());
        }
        else{
            return RC::SCHEMA_FIELD_TYPE_MISMATCH;
        }
        continue;
    }
    if (field->type() == TEXTS) {
        if (value.type != CHARS)  return RC::SCHEMA_FIELD_TYPE_MISMATCH;
        // 对于TEXT类型，实际值存放在堆里，数据库里只存储一个4字节的堆内偏移
        assert(heap_manager != nullptr);
        auto heap_offset = heap_manager->put((char*)value.data);
        if (heap_offset == 0) {
            // error
            return RC::IOERR;
        }
        *(uint32_t*)(record + field->offset()) = heap_offset;
    } else {
        memcpy(record + field->offset(), value.data, field->len());
    }
  }

  record_out = record;
  return RC::SUCCESS;
}

RC Table::init_record_handler(const char *base_dir) {
  std::string data_file = std::string(base_dir) + "/" + table_meta_.name() + TABLE_DATA_SUFFIX;
  if (nullptr == data_buffer_pool_) {
    data_buffer_pool_ = theGlobalDiskBufferPool();
  }

  int data_buffer_pool_file_id;
  RC rc = data_buffer_pool_->open_file(data_file.c_str(), &data_buffer_pool_file_id);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to open disk buffer pool for file:%s. rc=%d:%s",
              data_file.c_str(), rc, strrc(rc));
    return rc;
  }

  record_handler_ = new RecordFileHandler();
  rc = record_handler_->init(*data_buffer_pool_, data_buffer_pool_file_id);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to init record handler. rc=%d:%s", rc, strrc(rc));
    return rc;
  }

  file_id_ = data_buffer_pool_file_id;
  return rc;
}

/**
 * 为了不把Record暴露出去，封装一下
 */
class RecordReaderScanAdapter {
public:
  explicit RecordReaderScanAdapter(void (*record_reader)(const char *data, void *context), void *context)
      : record_reader_(record_reader), context_(context){
  }

  void consume(const Record *record) {
    record_reader_(record->data, context_);
  }
private:
  void (*record_reader_)(const char *, void *);
  void *context_;
};
static RC scan_record_reader_adapter(Record *record, void *context) {
  RecordReaderScanAdapter &adapter = *(RecordReaderScanAdapter *)context;
  adapter.consume(record);
  return RC::SUCCESS;
}

RC Table::scan_record(Trx *trx, ConditionFilter *filter, int limit, void *context, void (*record_reader)(const char *data, void *context)) {
  RecordReaderScanAdapter adapter(record_reader, context);
  return scan_record(trx, filter, limit, (void *)&adapter, scan_record_reader_adapter);
}

RC Table::scan_record(Trx *trx, ConditionFilter *filter, int limit, void *context, RC (*record_reader)(Record *record, void *context)) {
  if (nullptr == record_reader) {
    return RC::INVALID_ARGUMENT;
  }

  if (0 == limit) {
    return RC::SUCCESS;
  }

  if (limit < 0) {
    limit = INT_MAX;
  }

  IndexScanner *index_scanner = find_index_for_scan(filter);
  if (index_scanner != nullptr) {
    return scan_record_by_index(trx, index_scanner, filter, limit, context, record_reader);
  }

  RC rc = RC::SUCCESS;
  RecordFileScanner scanner;
  rc = scanner.open_scan(*data_buffer_pool_, file_id_, filter);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("failed to open scanner. file id=%d. rc=%d:%s", file_id_, rc, strrc(rc));
    return rc;
  }

  int record_count = 0;
  Record record;
  rc = scanner.get_first_record(&record);
  for ( ; RC::SUCCESS == rc && record_count < limit; rc = scanner.get_next_record(&record)) {
    if (trx == nullptr || trx->is_visible(this, &record)) {
      rc = record_reader(&record, context);
      if (rc != RC::SUCCESS) {
        break;
      }
      record_count++;
    }
  }

  if (RC::RECORD_EOF == rc) {
    rc = RC::SUCCESS;
  } else {
    LOG_ERROR("failed to scan record. file id=%d, rc=%d:%s", file_id_, rc, strrc(rc));
  }
  scanner.close_scan();
  return rc;
}

RC Table::scan_record_by_index(Trx *trx, IndexScanner *scanner, ConditionFilter *filter, int limit, void *context,
                               RC (*record_reader)(Record *, void *)) {
  RC rc = RC::SUCCESS;
  RID rid;
  Record record;
  int record_count = 0;
  while (record_count < limit) {
    rc = scanner->next_entry(&rid);
    if (rc != RC::SUCCESS) {
      if (RC::RECORD_EOF == rc) {
        rc = RC::SUCCESS;
        break;
      }
      LOG_ERROR("Failed to scan table by index. rc=%d:%s", rc, strrc(rc));
      break;
    }

    rc = record_handler_->get_record(&rid, &record);
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to fetch record of rid=%d:%d, rc=%d:%s", rid.page_num, rid.slot_num, rc, strrc(rc));
      break;
    }

    if ((trx == nullptr || trx->is_visible(this, &record)) && (filter == nullptr || filter->filter(record, rc))) {
      if (rc != RC::SUCCESS) {
        LOG_ERROR("filter comparison failed.");
        break;
      }
      rc = record_reader(&record, context);
      if (rc != RC::SUCCESS) {
        LOG_TRACE("Record reader break the table scanning. rc=%d:%s", rc, strrc(rc));
        break;
      }
    }
    if (rc != RC::SUCCESS) {
      LOG_ERROR("filter comparison failed.");
      break;
    }
    record_count++;
  }

  scanner->destroy();
  return rc;
}

class IndexInserter {
public:
  explicit IndexInserter(Index *index) : index_(index) {
  }

  RC insert_index(const Record *record) {
    return index_->insert_entry(record->data, &record->rid);
  }
private:
  Index * index_;
};

static RC insert_index_record_reader_adapter(Record *record, void *context) {
  IndexInserter &inserter = *(IndexInserter *)context;
  return inserter.insert_index(record);
}

RC Table::create_index(Trx *trx, const char *index_name, const char *attribute_name, bool unique_attr) {
  if (index_name == nullptr || common::is_blank(index_name) ||
      attribute_name == nullptr || common::is_blank(attribute_name)) {
    return RC::INVALID_ARGUMENT;
  }
  if (table_meta_.index(index_name) != nullptr ||
      table_meta_.find_index_by_field((attribute_name))) {
    return RC::SCHEMA_INDEX_EXIST;
  }

  const FieldMeta *field_meta = table_meta_.field(attribute_name);
  if (!field_meta) {
    return RC::SCHEMA_FIELD_MISSING;
  }

  IndexMeta new_index_meta;
  RC rc = new_index_meta.init(index_name, *field_meta, unique_attr);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  // 创建索引相关数据
  BplusTreeIndex *index = new BplusTreeIndex();
  std::string index_file = index_data_file(base_dir_.c_str(), name(), index_name);
  rc = index->create(index_file.c_str(), new_index_meta, *field_meta);
  if (rc != RC::SUCCESS) {
    delete index;
    LOG_ERROR("Failed to create bplus tree index. file name=%s, rc=%d:%s", index_file.c_str(), rc, strrc(rc));
    return rc;
  }

  // 遍历当前的所有数据，插入这个索引
  IndexInserter index_inserter(index);
  rc = scan_record(trx, nullptr, -1, &index_inserter, insert_index_record_reader_adapter);
  if (rc != RC::SUCCESS) {
    // rollback
    delete index;
    LOG_ERROR("Failed to insert index to all records. table=%s, rc=%d:%s", name(), rc, strrc(rc));
    return rc;
  }
  indexes_.push_back(index);

  TableMeta new_table_meta(table_meta_);
  rc = new_table_meta.add_index(new_index_meta);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to add index (%s) on table (%s). error=%d:%s", index_name, name(), rc, strrc(rc));
    return rc;
  }
  // 创建元数据临时文件
  std::string tmp_file = table_meta_file(base_dir_.c_str(), name()) + ".tmp";
  std::fstream fs;
  fs.open(tmp_file, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!fs.is_open()) {
    LOG_ERROR("Failed to open file for write. file name=%s, errmsg=%s", tmp_file.c_str(), strerror(errno));
    return RC::IOERR; // 创建索引中途出错，要做还原操作
  }
  if (new_table_meta.serialize(fs) < 0) {
    LOG_ERROR("Failed to dump new table meta to file: %s. sys err=%d:%s", tmp_file.c_str(), errno, strerror(errno));
    return RC::IOERR;
  }
  fs.close();

  // 覆盖原始元数据文件
  std::string meta_file = table_meta_file(base_dir_.c_str(), name());
  int ret = rename(tmp_file.c_str(), meta_file.c_str());
  if (ret != 0) {
    LOG_ERROR("Failed to rename tmp meta file (%s) to normal meta file (%s) while creating index (%s) on table (%s). " \
              "system error=%d:%s", tmp_file.c_str(), meta_file.c_str(), index_name, name(), errno, strerror(errno));
    return RC::IOERR;
  }

  table_meta_.swap(new_table_meta);

  LOG_INFO("add a new index (%s) on the table (%s)", index_name, name());

  return rc;
}

RC Table::create_multi_index(Trx *trx, const char *index_name, const char **attribute_names, size_t attribute_count) {
    if (index_name == nullptr || common::is_blank(index_name)) {
        return RC::INVALID_ARGUMENT;
    }
    for (int i = 0;i<attribute_count;i++) {
        if (attribute_names[i]== nullptr||common::is_blank(attribute_names[i]))
            return RC::INVALID_ARGUMENT;
    }
    if (table_meta_.multi_index(index_name) != nullptr) {
        return RC::SCHEMA_INDEX_EXIST;
    }
    std::vector<FieldMeta> field_metas;
    for(int i = 0; i<attribute_count;i++) {
        const FieldMeta *field_meta = table_meta_.field(attribute_names[i]);
        if (!field_meta) {
            return RC::SCHEMA_FIELD_MISSING;
        }
        field_metas.push_back(*field_meta);
    }

    MultiIndexMeta new_multi_index_meta;
    RC rc = new_multi_index_meta.init(index_name, field_metas);
    if (rc != RC::SUCCESS) {
        return rc;
    }

    // 创建索引相关数据
    MultiBplusTreeIndex *multi_index = new MultiBplusTreeIndex();
    std::string index_file = index_data_file(base_dir_.c_str(), name(), index_name);
    rc = multi_index->create(index_file.c_str(), new_multi_index_meta);
    if (rc != RC::SUCCESS) {
        delete multi_index;
        LOG_ERROR("Failed to create bplus tree index. file name=%s, rc=%d:%s", index_file.c_str(), rc, strrc(rc));
        return rc;
    }
    // 遍历当前的所有数据，插入这个索引
    IndexInserter index_inserter(multi_index);
    rc = scan_record(trx, nullptr, -1, &index_inserter, insert_index_record_reader_adapter);
    if (rc != RC::SUCCESS) {
        // rollback
        delete multi_index;
        LOG_ERROR("Failed to insert index to all records. table=%s, rc=%d:%s", name(), rc, strrc(rc));
        return rc;
    }
    indexes_.push_back(multi_index);

    TableMeta new_table_meta(table_meta_);
    rc = new_table_meta.add_multi_index(new_multi_index_meta);
    if (rc != RC::SUCCESS) {
        LOG_ERROR("Failed to add index (%s) on table (%s). error=%d:%s", index_name, name(), rc, strrc(rc));
        return rc;
    }
    // 创建元数据临时文件
    std::string tmp_file = table_meta_file(base_dir_.c_str(), name()) + ".tmp";
    std::fstream fs;
    fs.open(tmp_file, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    if (!fs.is_open()) {
        LOG_ERROR("Failed to open file for write. file name=%s, errmsg=%s", tmp_file.c_str(), strerror(errno));
        return RC::IOERR; // 创建索引中途出错，要做还原操作
    }
    if (new_table_meta.serialize(fs) < 0) {
        LOG_ERROR("Failed to dump new table meta to file: %s. sys err=%d:%s", tmp_file.c_str(), errno, strerror(errno));
        return RC::IOERR;
    }
    fs.close();

    // 覆盖原始元数据文件
    std::string meta_file = table_meta_file(base_dir_.c_str(), name());
    int ret = rename(tmp_file.c_str(), meta_file.c_str());
    if (ret != 0) {
        LOG_ERROR("Failed to rename tmp meta file (%s) to normal meta file (%s) while creating index (%s) on table (%s). " \
              "system error=%d:%s", tmp_file.c_str(), meta_file.c_str(), index_name, name(), errno, strerror(errno));
        return RC::IOERR;
    }

    table_meta_.swap(new_table_meta);

    LOG_INFO("add a new index (%s) on the table (%s)", index_name, name());

    return rc;
}


void record_reader_table(const char *data, void *context) {
    TupleRecordConverter *converter = (TupleRecordConverter *)context;
    converter->add_record(data);
}
RC Table::update_record(Trx *trx, const char *attribute_name, const Value *value, int condition_num, const Condition conditions[], int *updated_count) {
    //初始化返回状态
    RC rc = RC::SUCCESS;
    //初始化可能更新的日期
    time_t time_value;
    bool is_date = false, is_text = false; // 是否发生date的、text的隐式类型转换
    unsigned int time_int = 0;
    //初始化表的schema
    TupleSchema tuple_schema;
    TupleSchema::from_table(this,tuple_schema);
    int value_num = tuple_schema.get_field_size();
    //查看要upset的列的列名是否在表的schema中,如过在得到下标，不在直接返回错误
    int field_index = tuple_schema.index_of_field(name(),attribute_name);
    if (field_index==-1)
        return RC::SCHEMA_FIELD_NOT_EXIST;
    TupleField tuple_field = tuple_schema.field(field_index);
    //判断nullable是否一致
    if (!tuple_field.nullable()&&value->null_attr) {
        return RC::CONSTRAINT_NOTNULL;
    }
    //判断更新的数据类型和对应列的数据类型是否一致
    if (tuple_field.type()!=value->type) {
        // 判断是否更新为空，如果不是的话，判断能否进行隐式类型转换，如果都不能，则该更新非法，拒绝更新
        if (value->null_attr) {
            // 更新为空，这里暂时不处理，后面处理
        } else if (value->type == CHARS && tuple_field.type() == DATE) {
            //确实是用chars更新date
            is_date = true;
            //chars更新date要看日期是否合法。
            if(mytime::chars_to_date((char*)value->data,time_value)){
                //合法就保存time值
                time_int = time_value;
            }
            else{
                //不合法
                rc = RC::SCHEMA_FIELD_TYPE_MISMATCH;
                return rc;
            }
        } else if (value->type == CHARS && tuple_field.type() == TEXTS) {
            // 字符串可以被隐式转换为超长字段
            is_text = true;
        } else {
            // 非法更新，类型不匹配
            rc = RC::SCHEMA_FIELD_TYPE_MISMATCH;
            return rc;
        }
    }
    //为每个比较的condition构建一个比较器filter
    std::vector<DefaultConditionFilter *> condition_filters;
    for (int i = 0; i < condition_num; i++) {
        const Condition &condition = conditions[i];
        std::vector<RelAttr> lefts;
        std::vector<RelAttr> rights;
        bool is_simple = is_simple_ex(condition.left_attr, lefts) && is_simple_ex(condition.right_attr, rights);
        if (is_simple) {
            RelAttr left_attr = lefts[0];
            RelAttr right_attr = rights[0];
            if ((left_attr.extype == val && right_attr.extype == val) || // 两边都是值
            (left_attr.extype == id && right_attr.extype == val)||  // 左边是属性右边是值
            (left_attr.extype == val && right_attr.extype == id)  ||  // 左边是值，右边是属性名
            (left_attr.extype == id && right_attr.extype == id )
            // 左右都是属性名，并且表名都符合
            ) {
                DefaultConditionFilter *condition_filter = new DefaultConditionFilter();
                Condition condition1;
                condition1.comp = condition.comp;
                if ((left_attr.extype == val && right_attr.extype == val)) {
                    condition1.right_is_attr = 0;
                    condition1.left_is_attr = 0;
                    condition1.right_value = right_attr.value;
                    condition1.left_value = left_attr.value;

                }
                if ((left_attr.extype == id && right_attr.extype == val)) {
                    condition1.right_is_attr = 0;
                    condition1.left_is_attr = 1;
                    condition1.right_value = right_attr.value;
                    condition1.left_attr = left_attr;
                }
                if ((left_attr.extype == val && right_attr.extype == id)) {
                    condition1.right_is_attr = 1;
                    condition1.left_is_attr = 0;
                    condition1.left_value = left_attr.value;
                    condition1.right_attr = right_attr;
                }
                if (left_attr.extype == id && right_attr.extype == id) {
                    condition1.right_is_attr = 1;
                    condition1.left_is_attr = 1;
                    condition1.left_attr = left_attr;
                    condition1.right_attr = right_attr;
                }
                RC rc = condition_filter->init(*this, condition1, trx);
                if (rc != RC::SUCCESS) {
                    delete condition_filter;
                    for (DefaultConditionFilter *&filter: condition_filters) {
                        delete filter;
                    }
                    return rc;
                }
                condition_filters.push_back(condition_filter);
            }
        }
    }
    //将多个比较器组合成为多值过滤比较器
    CompositeConditionFilter condition_filter;
    condition_filter.init((const ConditionFilter **)condition_filters.data(),condition_filters.size());
    //初始化过滤出来的tupleSet
    // 读取出所有要更新的行
    TupleSet tuple_set;
    tuple_set.clear();
    tuple_set.set_schema(tuple_schema);
    TupleRecordConverter converter(this,tuple_set);
    rc = scan_record(trx, &condition_filter, -1, (void *)&converter, record_reader_table);
    if (rc!=RC::SUCCESS)
        return rc;
    // 删除原来表中的所有要更新的行
    int delete_num;
    rc = delete_record(nullptr,&condition_filter,&delete_num);
    if (rc!=RC::SUCCESS)
        return rc;
    //更改过滤出来的每个tuple的属性值,并且生成record的同时一条条插入table
//    std::vector<Tuple> tuples;
    int tuple_size = tuple_set.size();
//    for(int i = 0 ;i <tuple_size;i ++) {
//        tuples.push_back(tuple_set.tuples().at(i));
//    }
    const int normal_field_start_index = table_meta_.sys_field_num();
    unsigned int null_attr_true = 0x00000001;
    unsigned int null_attr_false = 0x00000000;
    for (int i = 0; i<tuple_size; i++){
        //遍历要update的每一行，将每一行都转换成record之后插入
        int record_size = table_meta_.record_size();
        char *record_char = new char [record_size];
        for (int j = 0; j < value_num; ++j) {
            const FieldMeta *field = table_meta_.field(j + normal_field_start_index);
            if (j==field_index) {
                if(value->null_attr){
                    memcpy(record_char + field->offset()-4, &null_attr_true, 4);
                    continue;
                }
                else {
                    memcpy(record_char + field->offset()-4, &null_attr_false, 4);
                    if (is_text) {
                        uint32_t text_offset = heap_manager->put((char*)value->data);
                        memcpy(record_char + field->offset(), &text_offset, field->len());
                    } else if (is_date) {
                        memcpy(record_char + field->offset(), &time_int, field->len());
                    }
                    else {
                        memcpy(record_char + field->offset(), value->data, field->len());
                    }
                }
                //要更改的列就将新值复制
            }
            else{
                //不是要更改的列，直接将原来的值复制
                auto ptr_tuple = (tuple_set.tuples().at(i)).get_pointer(j);
                const void * ptr_data;
                uint32_t text_offset;
                ptr_tuple->get_data(ptr_data);
                if (ptr_tuple->get_type() == AttrType::TEXTS) {
                    // 如果是TEXT类型，需要再写入堆内，因为当前UPDATE的实现是有复制语义的（删除了再创建新的），旧的值已经被删掉了
                    text_offset = heap_manager->put((char*)ptr_data);
                    ptr_data = &text_offset;
                }
                memcpy(record_char + field->offset(), ptr_data, field->len());
                if((tuple_set.tuples().at(i)).get_pointer(j)->is_null()) {
                    memcpy(record_char + field->offset()-4, &null_attr_true, 4);
                }
                else {
                    memcpy(record_char + field->offset()-4, &null_attr_false, 4);
                }
            }
        }
        Record record;
        record.data = record_char;
        rc = insert_record(trx,&record);
        if (rc!=RC::SUCCESS)
            return rc;
        *updated_count++;
    }
    return RC::SUCCESS;
}

class RecordDeleter {
public:
  RecordDeleter(Table &table, Trx *trx) : table_(table), trx_(trx) {
  }

  RC delete_record(Record *record) {
    RC rc = RC::SUCCESS;
    rc = table_.delete_record(trx_, record);
    if (rc == RC::SUCCESS) {
      deleted_count_++;
    }
    return rc;
  }

  int deleted_count() const {
    return deleted_count_;
  }

private:
  Table & table_;
  Trx *trx_;
  int deleted_count_ = 0;
};

static RC record_reader_delete_adapter(Record *record, void *context) {
  RecordDeleter &record_deleter = *(RecordDeleter *)context;
  return record_deleter.delete_record(record);
}

RC Table::delete_record(Trx *trx, ConditionFilter *filter, int *deleted_count) {
  RecordDeleter deleter(*this, trx);
  RC rc = scan_record(trx, filter, -1, &deleter, record_reader_delete_adapter);
  if (deleted_count != nullptr) {
    *deleted_count = deleter.deleted_count();
  }
  return rc;
}

RC Table::delete_record(Trx *trx, Record *record) {
  RC rc = RC::SUCCESS;
  if (trx != nullptr) {
    rc = trx->delete_record(this, record);
  } else {
    rc = delete_entry_of_indexes(record->data, record->rid, false);// 重复代码 refer to commit_delete
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to delete indexes of record (rid=%d.%d). rc=%d:%s",
                record->rid.page_num, record->rid.slot_num, rc, strrc(rc));
    } else {
      rc = record_handler_->delete_record(&record->rid);
    }
  }
  return rc;
}

RC Table::commit_delete(Trx *trx, const RID &rid) {
  RC rc = RC::SUCCESS;
  Record record;
  rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  rc = delete_entry_of_indexes(record.data, record.rid, false);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to delete indexes of record(rid=%d.%d). rc=%d:%s",
              rid.page_num, rid.slot_num, rc, strrc(rc));// panic?
  }

  rc = record_handler_->delete_record(&rid);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return rc;
}

RC Table::rollback_delete(Trx *trx, const RID &rid) {
  RC rc = RC::SUCCESS;
  Record record;
  rc = record_handler_->get_record(&rid, &record);
  if (rc != RC::SUCCESS) {
    return rc;
  }

  return trx->rollback_delete(this, record); // update record in place
}

RC Table::insert_entry_of_indexes(const char *record, const RID &rid) {
  RC rc = RC::SUCCESS;
  for (Index *index : indexes_) {
    rc = index->insert_entry(record, &rid);
    if (rc != RC::SUCCESS) {
      break;
    }
  }
  return rc;
}

RC Table::delete_entry_of_indexes(const char *record, const RID &rid, bool error_on_not_exists) {
  RC rc = RC::SUCCESS;
  for (Index *index : indexes_) {
    rc = index->delete_entry(record, &rid);
    if (rc != RC::SUCCESS) {
      if (rc != RC::RECORD_INVALID_KEY || !error_on_not_exists) {
        break;
      }
    }
  }
  return rc;
}

Index *Table::find_index(const char *index_name) const {
  for (Index *index: indexes_) {
    if (!index->is_multi()&&0 == strcmp(index->index_meta().name(), index_name)) {
      return index;
    }
  }
  return nullptr;
}

MultiBplusTreeIndex * Table::find_multi_index(const MultiIndexMeta * multi_index_meta) const{
    for (Index *index: indexes_) {
        if (index->is_multi()&&index->multi_index_meta()==*multi_index_meta) {
            return dynamic_cast<MultiBplusTreeIndex *>(index);
        }
    }
    return nullptr;
}
IndexScanner *Table::find_index_for_scan(const DefaultConditionFilter &filter) {
  const ConDesc *field_cond_desc = nullptr;
  const ConDesc *value_cond_desc = nullptr;
  if (filter.left().is_attr && !filter.right().is_attr) {
    field_cond_desc = &filter.left();
    value_cond_desc = &filter.right();
  } else if (filter.right().is_attr && !filter.left().is_attr) {
    field_cond_desc = &filter.right();
    value_cond_desc = &filter.left();
  }
  if (field_cond_desc == nullptr || value_cond_desc == nullptr) {
    return nullptr;
  }

  const FieldMeta *field_meta = table_meta_.find_field_by_offset(field_cond_desc->attr_offset);
  if (nullptr == field_meta) {
    LOG_PANIC("Cannot find field by offset %d. table=%s",
              field_cond_desc->attr_offset, name());
    return nullptr;
  }

  const IndexMeta *index_meta = table_meta_.find_index_by_field(field_meta->name());
  if (nullptr == index_meta) {
      //未找到单列索引，就继续尝试找多列索引
      const MultiIndexMeta *multi_index_meta = table_meta_.find_multi_index_by_field(field_meta->name());
      if (nullptr!=multi_index_meta){
          MultiBplusTreeIndex *multi_index = find_multi_index(multi_index_meta);
          if(multi_index== nullptr)
              return nullptr;
              else {
                  //成功找到多列索引
                return multi_index->create_scanner(filter.comp_op(), (const char *)value_cond_desc->value, value_cond_desc->is_null);
              }
      }
    return nullptr;
  }

  Index *index = find_index(index_meta->name());
  if (nullptr == index) {
      return nullptr;
  }

  return index->create_scanner(filter.comp_op(), (const char *)value_cond_desc->value, value_cond_desc->is_null);
}

IndexScanner *Table::find_index_for_scan(const ConditionFilter *filter) {
  if (nullptr == filter) {
    return nullptr;
  }

  // remove dynamic_cast
  const DefaultConditionFilter *default_condition_filter = dynamic_cast<const DefaultConditionFilter *>(filter);
  if (default_condition_filter != nullptr) {
    return find_index_for_scan(*default_condition_filter);
  }

  const CompositeConditionFilter *composite_condition_filter = dynamic_cast<const CompositeConditionFilter *>(filter);
  if (composite_condition_filter != nullptr) {
    int filter_num = composite_condition_filter->filter_num();
    for (int i = 0; i < filter_num; i++) {
      IndexScanner *scanner= find_index_for_scan(&composite_condition_filter->filter(i));
      if (scanner != nullptr) {
        return scanner; // 可以找到一个最优的，比如比较符号是=
      }
    }
  }
  return nullptr;
}

RC Table::sync() {
  RC rc = data_buffer_pool_->flush_all_pages(file_id_);
  if (rc != RC::SUCCESS) {
    LOG_ERROR("Failed to flush table's data pages. table=%s, rc=%d:%s", name(), rc, strrc(rc));
    return rc;
  }

  for (Index *index: indexes_) {
    rc = index->sync();
    if (rc != RC::SUCCESS) {
      LOG_ERROR("Failed to flush index's pages. table=%s, index=%s, rc=%d:%s",
                name(), index->index_meta().name(), rc, strrc(rc));
      return rc;
    }
  }

  if (!heap_manager->flush()) {
      LOG_ERROR("Failed to flush TEXT heap. table=%s.", name());
      return RC::BUFFERPOOL_FILEERR;
  }

  LOG_INFO("Sync table over. table=%s", name());
  return rc;
}
