//
// Created by egork on 05.12.2022.
//

#include "Cache.h"

namespace lab31 {
    bool Cache::isCached(const std::string &url) {
        return cache.find(url) != cache.end();
    }

    bool Cache::isFullyCached(const std::string &url) {
        return (isCached(url) && cache.find(url) -> second -> isFullyCached() && cache.find(url) -> second -> getIterationsAlive()!=0);
    }


    CacheRecord* Cache::addRecord(const std::string &url) {
        //std::cout << "Adding new record for " + url << std::endl;
        auto record = new CacheRecord(this);
        cache.insert(std::make_pair(url, record));
        return record;
    }

    CacheRecord *Cache::subscribe(const std::string &url, int socket) {
        if (isCached(url)) {
            //std::cerr <<"Subscribing client #" + std::to_string(socket) + " to " + url;
            CacheRecord* cacheRecord= cache.find(url) -> second;
            cacheRecord -> addObserver(socket);
            cacheRecord -> recoverIterationsAlive();
            return cacheRecord;
        } else {
            //std::cerr << url + " is not cached.";
            return nullptr;
        }
    }

    void Cache::unsubscribe(const std::string &url, int socket) {
        if (!isCached(url)) {
            return;
        } else {
            cache.find(url) -> second -> removeObserver(socket);
        }
    }

    Cache::~Cache() {
        for (const auto& record : cache){
            delete record.second;
        }

        std::cerr << "Cache deleted" << std::endl;
    }

    std::vector<int> Cache::getReadyObservers() {
        for (auto record : cache){
            if (record.second -> isReadyForRead()){
                readyObservers.insert(readyObservers.end(), record.second -> getObservers().begin(), record.second -> getObservers().end());
            }
        }
        return readyObservers;
    }

    void Cache::clearReadyObservers(){
        readyObservers.clear();
    }

    void Cache::deleteDeadRecords() {
        std::string cacheForDelete[cache.size()];
        int i = 0;
        for (auto record : cache){
            if (record.second -> getObservers().empty() &&
                (isFullyCached(record.first) || record.second -> getDeleteAfterUse())
                || record.second -> isBroken()){
                if(record.second -> getIterationsAlive() == 0){
                    std::cout << record.second -> getObservers().size() << '\n';
                    cacheForDelete[i] = record.first;
                    i++;
                }
                record.second -> Iteration();
            }
        }
        for(int j = 0; j<i;j++){
            std::cout << "deleted " << cacheForDelete[j] << '\n';
            deleteRecord(cacheForDelete[j]);
        }
    }

    void Cache::deleteRecord(const std::string& url){
        cache.erase(cache.find(url));
    }
} // lab31