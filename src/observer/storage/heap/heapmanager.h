//
// Created by Keuin on 2021/11/18.
//

#ifndef MINIDB_HEAPMANAGER_H
#define MINIDB_HEAPMANAGER_H

#include <cstdint>
#include <cassert>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "rc.h"

// 堆头，保存了必要的全局堆内状态
typedef struct s_heap_header {
    uint32_t next_free;
} HeapHeader;

class HeapManager {
public:
    HeapManager(const char* file_name);
    ~HeapManager();

    RC init();

    /**
     * 内存数据落盘
     */
    bool flush();

    /**
     * 将一个C风格字符串加入到堆中，返回指向该数据的堆内偏移。
     * 字符串的最大长度为4096字节（末尾的'\0'不计入此限制），超长部分将被截断。
     * 返回0表示错误。
     */
    uint32_t put(const char *value);

    /**
     * 给定一个堆内偏移，获取以该偏移为起点的只读内存指针。
     * 用户需要用free自行释放返回的指针。
     * pos为非0值。
     * 返回nullptr表示错误。
     */
    const char* get(uint32_t pos);

    std::string get_file_name();

private:
    std::string file_name;
    FILE *fp;

    uint32_t put(void *data, size_t length);
};


#endif //MINIDB_HEAPMANAGER_H
