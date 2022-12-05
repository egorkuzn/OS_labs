//
// Created by egork on 05.12.2022.
//

#ifndef OS_LABS_CLIENTHANDLER_H
#define OS_LABS_CLIENTHANDLER_H


#include <string>
#include <poll.h>
#include <netdb.h>
#include <cstring>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../Proxy.h"
#include "ServerHandler.h"
#include "ConnectionHandler.h"
#include "../Cache/CacheRecord.h"

namespace lab31 {

    class Proxy;
    class http_parser;

    class ClientHandler: public ConnectionHandler{
    public:
        ~ClientHandler() override{proxy->getCache()->unsubscribe(url, clientSocket);};
        explicit ClientHandler(int socket, Proxy *proxy);
        int connectToServer(const std::string& host);
        bool handle(int event) override;
        void setURL(const char *at, size_t len);
        void setHost(const char *at, size_t len);
        int getSocket() const { return clientSocket; }
        std::string getLastField();
        void setLastField(const char *at, size_t len);
        void resetLastField();
        void createServer(int socket);
        bool writeToServer(const std::string& msg);
        bool tryMakeFirstWriter();
        bool RequestParser();
        bool initConnectionToDest();
        static std::string getUrl(std::string in);
        static std::string getMethod(std::string in);
        static std::string getPrVersion(std::string in);
        void deleteCache() override;
        size_t getReadElements() override;
        CacheRecord* getCacheRecord() override { return record;}
    private:
        int timeAlive = 100;
        int clientSocket;
        Proxy *proxy;
        std::string request;
        std::string lastField;
        std::string prVersion = "1.0";
        std::string url;
        std::string host;
        std::string port;
        ServerHandler *server;
        CacheRecord *record;
        size_t readPointer = 0;
        bool cachingInParallel = false;
        bool firstWriter = false;
        bool receive();
        bool becomeFirstWriter();
        bool initialized = false;
    };

} // lab31

#endif //OS_LABS_CLIENTHANDLER_H
