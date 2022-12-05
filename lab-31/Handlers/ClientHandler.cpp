//
// Created by egork on 05.12.2022.
//

#include "ClientHandler.h"

namespace lab31 {
    ClientHandler::ClientHandler(int sock, Proxy *clientProxy){
        this -> clientSocket = sock;
        this -> proxy = clientProxy;
        this -> server = nullptr;
        this -> url = "";
        this -> host = "";
        this -> record = nullptr;
    }

    void ClientHandler::deleteCache(){
        std::cout << "deleteCache";
        record -> deleteRecord(url);
    }

//POLLIN - есть данные для чтения.
//POLLHUP - Устройство было отключено, или канал или FIFO были закрыты последним процессом, который открыл его для записи.
//POLLOUT - есть данные для отправки.
//POLLERR - Произошла ошибка
// смотрим со стороны сервера( у сервера есть данные на отправку, у сервера есть данные для чтения)
    bool ClientHandler::handle(int event) {

        if (event == (POLLHUP | POLLIN)) {
            return false;
        }

        if(event == (POLLHUP | POLLERR | POLLIN)){
            return false;
        }

        if (event & POLLIN) {
            if (!receive()) {
                return false;
            }
            //proxy->addEvent(clientSocket, POLLOUT);
            return true;
        }

        if (event & POLLOUT) {
            if (!initialized || record -> isBroken()) {
                return false;
            }

            if (readPointer <= record -> getDataSize()) {
                //std::cout << "proxy send data" << "\n";
                //std::cout << "start read " << '\n';
                std::string buffer = record -> read(readPointer, BUFSIZ);
                ssize_t ret = send(clientSocket, buffer.data(), buffer.size(), 0);
                //std::cout << "end read " << '\n';
                if (ret == -1) {
                    perror("send failed");
                } else {
                    readPointer += buffer.size();
                }

                if (buffer.empty()){
                    //std::cout << "asdasdas" << '\n';
                    proxy -> deleteEvent(clientSocket, POLLOUT);
                    //std::cout << "close POLLOUT" << std::endl;
                }
            }

        }
        return true;

    }

// "GET http://lib.pushkinskijdom.ru/Default.aspx?tabid=10183 HTTP/1.1\r\nHost: lib.pushkinskijdom.ru\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:103.0) Gecko/20100101 Firefox/103.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate\r\nReferer: http://lib.pushkinskijdom.ru/Default.aspx?tabid=2018\r\nConnection: keep-alive\r\nCookie: .ASPXANONYMOUS=uLMKm8gi2QEkAAAAOGEwYjJhYzItNzVmMS00OTRjLTlmZjMtYjAwNzQ4MTZkYTk40; __utma=261296784.1672019940.1667117300.1667190460.1667197894.4; __utmz=261296784.1667117300.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none); __utmc=261296784; language=ru-RU\r\nUpgrade-Insecure-Requests: 1\r\n\r\n"
    size_t findFirstSpChar(std::string in) {
        std::vector<size_t> indexes = {};
        indexes.push_back(in.find('\n'));
        indexes.push_back(in.find('\r'));
        indexes.push_back(in.find(' '));
        std::sort(indexes.begin(), indexes.end());

        return indexes[0];
    }

    std::string ClientHandler::getPrVersion(std::string in){
        std::string req = std::move(in);
        size_t start = req.find("HTTP/");

        return req.substr(start + 5, 3);
    }

    std::string getHost(std::string in){
        std::string req = std::move(in);
        std::string afterHostString = req.substr(req.find("Host:") + 6);
        size_t end = findFirstSpChar(afterHostString);
        return afterHostString.substr(0, end);
    }

    std::string ClientHandler::getUrl(std::string in){
        std::string req = std::move(in);
        size_t start = req.find(' ');
        size_t end = req.find("HTTP");
        if(start == std::string::npos || end == std::string::npos) {
            std::cerr <<"parse URL error" << std::endl;
            return "";
        }
        return req.substr(start + 1 , end - start - 2);
    }

    std::string ClientHandler::getMethod(std::string in){
        std::string req = std::move(in);
        size_t end = req.find(' ');

        if(end == std::string::npos) {
            std::cerr <<"parse URL error" << std::endl;
            return "";
        }

        return req.substr(0, end);
    }

    bool ClientHandler::RequestParser(){
        prVersion = getPrVersion(request);
        std::cout << prVersion << std::endl;

        if (prVersion!="1.1" && prVersion!="1.0") {
            char NOT_ALLOWED[71] = "HTTP/1.0 505 HTTP VERSION NOT SUPPORTED\r\n\r\n HTTP Version Not Supported";
            write(clientSocket, NOT_ALLOWED, 71);
            return false;
        }
        std::string HTTPMethod = getMethod(request);
        std::cout << HTTPMethod << std::endl;
        if (HTTPMethod != "GET" && HTTPMethod != "POST") {
            char NOT_ALLOWED[59] = "HTTP/1.0 405 METHOD NOT ALLOWED\r\n\r\n Method Not Allowed";
            write(clientSocket, NOT_ALLOWED, 59);
            return false;
        }

        host = getHost(request);
        int place = host.find(':');
        port = host.substr(place+1, host.size()-place-1);
        host = host.substr(0,host.find(':'));
        url = getUrl(request);
        return true;
    }

    bool ClientHandler::initConnectionToDest(){
        if(!becomeFirstWriter()){
            return false;
        }

        initialized = true;
        return true; // подключение к серверу, отправка запроса на него
    }

//"GET http://lib.pushkinskijdom.ru/Default.aspx?tabid=2018 HTTP/1.1\r\nHost: lib.pushkinskijdom.ru\r\n
    bool ClientHandler::receive() {
        char buffer[BUFSIZ];

        ssize_t len = recv(clientSocket, buffer, BUFSIZ, 0);
        if (len < 0) {
            std::cerr << "Failed to read data from clientSocket";
            return false;
        }

        if (len == 0) {
            proxy -> getCache() -> unsubscribe(url, clientSocket);
            return false;
        }

        request.append(buffer, len);

        if(len < BUFSIZ) {
            if(record != nullptr){
                record -> setFullyCached();
                proxy -> getCache() -> unsubscribe(url, clientSocket);
            }
            if(!RequestParser()){       // перенести парсинг и создание record сюда для протокола 1.1
                return false;
            }
            if (!proxy -> getCache() -> isFullyCached(url) ) {
                if (!initialized) {
                    if (!initConnectionToDest()) {
                        return false;
                    }
                } else {
                    writeToServer(request); // last
                }
                record = proxy -> getCache() -> addRecord(url);
                if (record == nullptr) {
                    std::cerr << "Failed to allocate new cache record for " + url;
                    proxy->stopProxy();
                    return false;
                }
                server -> setCacheRecord(record);
                proxy -> getCache() -> subscribe(url, clientSocket);
            }else{
                std::cout << "loaded from cache " << url << '\n';
                record = proxy -> getCache() -> subscribe(url, clientSocket);
                initialized = true;
            }
            readPointer = 0;
            request.clear();
        }
        return true;
    }

    bool ClientHandler::tryMakeFirstWriter() {
        return becomeFirstWriter();
    }

    bool ClientHandler::becomeFirstWriter() {
        firstWriter = true;
        readPointer = 0;
        int serverSocket = connectToServer(host);

        if (serverSocket == -1) {
            std::cerr << "Cannot connect to: " + host + " closing connection.";
            return false;
        } else {
            createServer(serverSocket);
        }

        if (!writeToServer(request)) {
            return false;
        }
        return true;
    }

    void ClientHandler::setURL(const char *at, size_t len) {
        url.append(at, len);
    }

    std::string ClientHandler::getLastField() {
        return lastField;
    }

    void ClientHandler::setLastField(const char *at, size_t len) {
        lastField.append(at, len);
    }

    int ClientHandler::connectToServer(const std::string& host) {
        struct addrinfo hints, *res, *result;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags |= AI_CANONNAME;

        int errcode = getaddrinfo((host+"\0").c_str(),(port+"\0").c_str(), &hints, &result);
        if (errcode != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
            exit(EXIT_FAILURE);
        }
        res = result;
        int fd = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);

        while (res != nullptr) {
            if (connect(fd, result -> ai_addr, result -> ai_addrlen) == 0) {
                return fd;
            }

            res = res -> ai_next;
        }

        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    void ClientHandler::resetLastField() {
        lastField = "";
    }

    void ClientHandler::createServer(int socket) {
        server = new ServerHandler(socket);
        proxy -> addNewConnection(socket, (POLLIN | POLLHUP));
        proxy -> addNewHandler(socket, server);
    }

    bool ClientHandler::writeToServer(const std::string &msg) {
        ssize_t len = send(server -> getSocket(), msg.data(), msg.size(), 0);

        if (len == -1) {
            std::cerr << "Failed to send data to server";
            record -> setBroken();
            return false;
        }
        //std::cout << "req" << '\n' << msg << '\n';
        return true;
    }

    void ClientHandler::setHost(const char *at, size_t len) {
        host.append(at, len);
    }

    size_t ClientHandler::getReadElements() {
        return readPointer;
    }

    ClientHandler::~ClientHandler() {
        proxy -> getCache() -> unsubscribe(url, clientSocket);
    }
} // lab31