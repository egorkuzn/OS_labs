//
// Created by egork on 05.12.2022.
//

#ifndef OS_LABS_PROXY_H
#define OS_LABS_PROXY_H

namespace lab31 {

    class Proxy {
    private:
        sockaddr_in address{};
        int socketDesc;     // сокет сервера
        std::vector<pollfd> connections{};
        std::map<int, ConnectionHandler *> handlers;
        bool proxyOn = false;
        Cache *cache;

        void disconnectHandler(int _sock);

    public:
        Cache *getCache() { return cache; };

        void addNewHandler(int fd, ConnectionHandler *handler);

        explicit Proxy(int port);

        ConnectionHandler *getHandlerByFd(int fd);

        ~Proxy();

        int createProxySocket(int port);

        bool initProxySocket();

        void addNewConnection(int socket, short event);

        void run();

        void stopProxy();

        void addEvent(int fd, short event);

        void deleteEvent(int fd, short event);

        void makeNewServer(const std::vector<int> &observers);

        void setPollOutEventToObservers(const std::vector<int> &observers);
    };

} // lab31

#endif //OS_LABS_PROXY_H
