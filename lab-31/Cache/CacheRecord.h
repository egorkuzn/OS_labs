//
// Created by egork on 05.12.2022.
//

#ifndef OS_LABS_CACHERECORD_H
#define OS_LABS_CACHERECORD_H

#include <vector>
#include <string>
#include <iostream>

#include "Cache.h"

namespace lab31 {

    class Cache;

// запрос и ответ на него
    class CacheRecord {
    private:
        std::vector<int> observers; // подписчики, по идеи, если их нет, то можно удалять запись
        Cache *cache;
        bool is_fully_cached = false;
        std::string data;
        bool is_broken = false;
        bool deleteAfterUse = false;
        bool readyForRead = false;
        const size_t DEFAULT_ITERATION_ALIVE = 100;
        size_t iterationsAlive = DEFAULT_ITERATION_ALIVE;

    public:
        void setDeleteAfterUse(){ deleteAfterUse = true; }
        bool getDeleteAfterUse(){ return deleteAfterUse; }
        void deleteRecord(const std::string& url);
        size_t getIterationsAlive(){ return iterationsAlive;}
        void Iteration(){ iterationsAlive--; }
        void recoverIterationsAlive(){ iterationsAlive = DEFAULT_ITERATION_ALIVE;}
        ~CacheRecord();
        CacheRecord(Cache *cache);
        void setBroken() { is_broken = true; };
        bool isBroken() const { return is_broken; };
        std::string read(size_t start, size_t length) const;
        void write(const std::string &);
        void addObserver(int socket);
        void removeObserver(int socket);
        void setFullyCached();
        bool isFullyCached() const { return is_fully_cached; };
        size_t getDataSize() { return data.size(); };
        int getObserverCount();
        void setReadyForRead() { readyForRead = true; };
        bool isReadyForRead() const { return readyForRead; };
        std::vector<int> &getObservers() { return observers; };
    };

} // lab31

#endif //OS_LABS_CACHERECORD_H
