//
// Created by egork on 05.12.2022.
//

#ifndef OS_LABS_SERVERHANDLER_H
#define OS_LABS_SERVERHANDLER_H

#include <poll.h>
#include <csignal>
#include <sys/socket.h>

#include "ConnectionHandler.h"

namespace lab31 {

    class ServerHandler : public ConnectionHandler {
    private:
        int serverSocket;
        CacheRecord *cacheRecord = nullptr;
    public:
        void deleteCache() override{};
        CacheRecord* getCacheRecord() override { return  cacheRecord;}
        ServerHandler(int socket, CacheRecord *record);
        explicit ServerHandler(int socket);
        void setCacheRecord(CacheRecord *record);
        bool handle(int event) override;
        int getSocket() const { return serverSocket; };
        ~ServerHandler() override;
        size_t getReadElements() override {return 0; };
        bool receive();
    };
} // lab31

#endif //OS_LABS_SERVERHANDLER_H
