//
// Created by Keuin on 2021/11/18.
//

#include <common/log/log.h>
#include "heapmanager.h"


// 创建一个堆管理器
HeapManager::HeapManager(const char *file_name) {
    this->file_name.assign(file_name);
    this->fp = nullptr;
}

HeapManager::~HeapManager() {
    if (fp) fclose(fp);
}

RC HeapManager::init() {
    if (!fp) {
        fp = fopen(file_name.c_str(), "w+");
        if (!fp) return RC::BUFFERPOOL_FILEERR;
        fseek(fp, 0, SEEK_END);
        if (ftell(fp) == 0) {
            fputc('H', fp);
            fputc('E', fp);
            fputc('A', fp);
            fputc('P', fp);
            fflush(fp);
        }
    }
    return RC::SUCCESS;
}

bool HeapManager::flush() {
    if (fp) {
        return fflush(fp) == 0;
    }
    return true;
}

uint32_t HeapManager::put(const char *value) {
    // 仅追加实现的堆管理器
    assert(fp != nullptr);
    fseek(fp, 0, SEEK_END);
    uint32_t offset = ftell(fp);
    for (size_t i = 0; i < 4096; ++i) {
        char c;
        if ((c = value[i]) == '\0') break;
        fputc(c, fp);
    }
    fputc('\0', fp);
    fflush(fp);
    LOG_ERROR("heap put: %u -> '%s'.\n", offset, value);
    return offset;
}

const char *HeapManager::get(uint32_t pos) {
    fseek(fp, pos, SEEK_SET);
    char *buf = (char *) malloc(4096 + 4);
    if (fgets(buf, 4096, fp) == nullptr) {
        LOG_ERROR("fgets failed. pos=%u, errno=%d.\n", pos, errno);
        strcpy(buf, "<error>");
    }
    buf[4096] = '\0';
    LOG_ERROR("heap get: %u -> '%s'.\n", pos, buf);
    return buf;
}

std::string HeapManager::get_file_name() {
    return this->file_name;
}
