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
// Created by Longda on 2021/4/13.
//
#ifndef __OBSERVER_STORAGE_COMMON_PAGE_MANAGER_H_
#define __OBSERVER_STORAGE_COMMON_PAGE_MANAGER_H_

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <vector>

#include "rc.h"

typedef int PageNum;

//
#define BP_INVALID_PAGE_NUM (-1)
#define BP_PAGE_SIZE (1 << 12)
#define BP_PAGE_DATA_SIZE (BP_PAGE_SIZE - sizeof(PageNum))
#define BP_FILE_SUB_HDR_SIZE (sizeof(BPFileSubHeader))
#define BP_BUFFER_SIZE 50
#define MAX_OPEN_FILE 1024

typedef struct {
  PageNum page_num;
  char data[BP_PAGE_DATA_SIZE];
} Page;
// sizeof(Page) should be equal to BP_PAGE_SIZE

typedef struct {
  PageNum page_count;
  int allocated_pages;
} BPFileSubHeader;

typedef struct {
  bool dirty;
  unsigned int pin_count;
  unsigned long acc_time;
  int file_desc;
  Page page;
} Frame;

typedef struct {
  bool open;
  Frame *frame;
} BPPageHandle;

class BPFileHandle{
public:
  BPFileHandle() {
    memset(this, 0, sizeof(*this));
  }

public:
  bool bopen;
  const char *file_name;
  int file_desc;
  Frame *hdr_frame;
  Page *hdr_page;
  char *bitmap;
  BPFileSubHeader *file_sub_header;
} ;

class BPManager {
public:
  BPManager(int size = BP_BUFFER_SIZE) {
    this->size = size;
    this->clockHand = size-1;
    frame = new Frame[size];
    allocated = new bool[size];
    ref = new bool[size];
    for (int i = 0; i < size; i++) {
      allocated[i] = false;
      frame[i].pin_count = 0;
      ref[i] = false;
    }
  }

  ~BPManager() {
    delete[] frame;
    delete[] allocated;
    delete[] ref;
    size = 0;
    frame = nullptr;
    allocated = nullptr;
    ref = nullptr;
  }

  Frame *alloc() {
    return nullptr; // TODO for test
  }

  int alloc_clock() {
      //在这里实现了时钟替换算法
      //用于记录被pinned的frame数量
      int pinnedFrameNumber = 0;
      //轮询
      begin: advanceClock();
      //如果所有的frame均被pinned则抛出异常
      if (pinnedFrameNumber>=size)
          return -1;
      if (!allocated[clockHand]) {
          //如果frame是空的就直接用
          ref[clockHand] = false;
          allocated[clockHand] = true;
          return clockHand;
      }
      else {
          if (ref[clockHand]) {
              //更新ref位
              ref[clockHand] = false;
              goto begin; //进行下一次轮询
          }
          else {
              if (frame[clockHand].pin_count>0) {
                  //如果该页正在被引用则记录数++
                  pinnedFrameNumber++;
                  goto begin;//进行下一次轮询
              }
              else {
                  //把原来的记录去除掉
                  ref[clockHand] = false;
                  //返回分配的frame
                  return clockHand;
              }
          }
      }
  }

  Frame *get(int file_desc, PageNum page_num) {
    return nullptr; // TODO for test
  }

  Frame *getFrame() { return frame; }

  bool *getAllocated() { return allocated; }

public:
  int size;
  Frame * frame = nullptr;
  bool *allocated = nullptr;
  bool *ref = nullptr;
  int clockHand = 0;

private:
    void advanceClock() {
        //将时钟轮询的指针拨向下一位
        clockHand = (clockHand+1)%(size);
    }
};

class DiskBufferPool {
public:
  /**
  * 创建一个名称为指定文件名的分页文件
  */
  RC create_file(const char *file_name);

  /**
   * 根据文件名打开一个分页文件，返回文件ID
   * 文件已经open返回success
   * 超过最大文件数返回BUFFERPOOL_OPEN_TOO_MANY_FILES
   * 打开文件失败返回IOERR_ACCESS
   * 无法分配内存返回NOMEM
   * 无法分配block则
   * @return
   */
  RC open_file(const char *file_name, int *file_id);

  /**
   * 关闭fileID对应的分页文件
   * invalid id返回BUFFERPOOL_ILLEGAL_FILE_ID
   * 写会文件失败返回IOERR_SEEK或者IOERR_WRITE
   * 关闭文件失败返回IOERR_CLOSE
   */
  RC close_file(int file_id);
/**
   * 关闭filename对应的分页文件;
   * invalid id返回BUFFERPOOL_ILLEGAL_FILE_ID;
   * 写会文件失败返回IOERR_SEEK或者IOERR_WRITE;
   * 关闭文件失败返回IOERR_CLOSE;
   */
    RC close_file(const char *file_name);
  /**
   * 根据文件ID和页号获取指定页面到缓冲区，返回页面句柄指针。
   * @return
   */
  RC get_this_page(int file_id, PageNum page_num, BPPageHandle *page_handle);

  /**
   * 在指定文件中分配一个新的页面，并将其放入缓冲区，返回页面句柄指针。
   * 分配页面时，如果文件中有空闲页，就直接分配一个空闲页；
   * 如果文件中没有空闲页，则扩展文件规模来增加新的空闲页。
   */
  RC allocate_page(int file_id, BPPageHandle *page_handle);

  /**
   * 根据页面句柄指针返回对应的页面号
   */
  RC get_page_num(BPPageHandle *page_handle, PageNum *page_num);

  /**
   * 根据页面句柄指针返回对应的数据区指针
   */
  RC get_data(BPPageHandle *page_handle, char **data);

  /**
   * 丢弃文件中编号为pageNum的页面，将其变为空闲页
   */
  RC dispose_page(int file_id, PageNum page_num);

  /**
   * 释放指定文件关联的页的内存， 如果已经脏， 则刷到磁盘，除了pinned page
   * @param file_handle
   * @param page_num 如果不指定page_num 将刷新所有页
   */
  RC force_page(int file_id, PageNum page_num);

  /**
   * 标记指定页面为“脏”页。如果修改了页面的内容，则应调用此函数，
   * 以便该页面被淘汰出缓冲区时系统将新的页面数据写入磁盘文件
   */
  RC mark_dirty(BPPageHandle *page_handle);

  /**
   * 此函数用于解除pageHandle对应页面的驻留缓冲区限制。
   * 在调用GetThisPage或AllocatePage函数将一个页面读入缓冲区后，
   * 该页面被设置为驻留缓冲区状态，以防止其在处理过程中被置换出去，
   * 因此在该页面使用完之后应调用此函数解除该限制，使得该页面此后可以正常地被淘汰出缓冲区
   */
  RC unpin_page(BPPageHandle *page_handle);

  /**
   * 获取文件的总页数
   */
  RC get_page_count(int file_id, int *page_count);

  RC flush_all_pages(int file_id);


protected:
  RC allocate_block(Frame **buf);
  RC dispose_block(Frame *buf);

  /**
   * 刷新指定文件关联的所有脏页到磁盘，除了pinned page
   * @param file_handle
   * @param page_num 如果不指定page_num 将刷新所有页
   */
  RC force_page(BPFileHandle *file_handle, PageNum page_num);
  RC force_all_pages(BPFileHandle *file_handle);
  RC check_file_id(int file_id);
  RC check_page_num(PageNum page_num, BPFileHandle *file_handle);
  RC load_page(PageNum page_num, BPFileHandle *file_handle, Frame *frame);
  RC flush_block(Frame *frame);

private:
  BPManager bp_manager_;
  BPFileHandle *open_list_[MAX_OPEN_FILE] = {nullptr};
};

DiskBufferPool *theGlobalDiskBufferPool();

#endif //__OBSERVER_STORAGE_COMMON_PAGE_MANAGER_H_
