//
// Created by egork on 05.12.2022.
//

#ifndef OS_LABS_CACHE_H
#define OS_LABS_CACHE_H

#include <map>
#include <string>
#include <vector>

#include "CacheRecord.h"

namespace lab31 {

    class CacheRecord;

    class Cache {
    private:
        std::map<std::string, CacheRecord* > cache;
        bool ran_out_of_memory = false;
        std::vector<int> readyObservers;

    public:
        void deleteRecord(const std::string& url);
        Cache() = default;
        ~Cache();
        void deleteDeadRecords();
        bool isCached(const std::string &url);
        CacheRecord *subscribe(const std::string &url, int socket);
        void unsubscribe(const std::string &url, int socket);
        CacheRecord *addRecord(const std::string &url);
        bool isFullyCached(const std::string &url);
        bool ranOutOfMemory() const { return ran_out_of_memory; };
        void setRanOutOfMemory() { ran_out_of_memory = true; };
        std::vector<int> getReadyObservers();
        void clearReadyObservers();
    };
} // lab31

#endif //OS_LABS_CACHE_H
