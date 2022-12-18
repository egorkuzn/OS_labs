
#include "ClientHandler.h"

ClientHandler::ClientHandler(int sock) {
    this->clientSocket = sock;
    this->url = "";
    this->host = "";
    this->headers = "";
    this->record = nullptr;
}

void ClientHandler::deleteEvent(pollfd* conn, short event) {
    conn->events &= ~event;
}

bool ClientHandler::readFromCache(pollfd* connection) {

    pthread_testcancel();

    if (record->isBroken()) {
        return false; // надо чтобы возвращал что то серьёзное
    }

    int state;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
    std::string buffer = record->read(readPointer, BUFSIZ);
    pthread_setcancelstate(state, nullptr);
    ssize_t ret = write(clientSocket, buffer.data(), buffer.size());
    //std::cout << buffer.data() << '\n';
    if (ret == -1) {
        perror("write failed");
        return false;
    } else {
        if (buffer.empty()){
            //std::cout << "delete read ability" << '\n';
            deleteEvent(connection, POLLOUT);
        }
        readPointer += buffer.size();
    }
    return true;
}

bool ClientHandler::receive(ClientHandler* client){

    char buffer[BUFSIZ];
    ssize_t len;
    request.erase();
    if(record != nullptr && record->isBroken()){
        return false;
    }

    len = read(clientSocket, buffer, BUFSIZ);
    if (len < 0) {
        std::cerr << "Failed to readFromCache data from client's socket" << '\n';
        return false;
    }

    /* if (len == 0) {
         std::cout << "Client #" + std::to_string(clientSocket) + " done writing. Closing connection" << '\n';
         return false;
     }*/
    request.append(buffer, len);
    memset(buffer, 0, BUFSIZ);

    if(len == 0 || len < BUFSIZ){
        //std::cout << request << '\n';
        //auto url = url;

        if(!RequestParser()){
            return false;
        }

        // криво работает смена кэша
        if(record != nullptr){
            //record->changeRecordPage(url);
            record -> decrementObserversCount();
        }

        if (!cache->isCached(url)) {
            record = cache->addRecord(url);
            if (!createServer(client, record, host, request)) {
                std::cerr << "Failed to create new server" << '\n';
                return false;
                //pthread_exit((void *) -1);
            }
            sendRequest(request);
            std::cout << url + " is not cached yet." << '\n';
        } else {
            std::cout << url + " is cached." << '\n';
            record = cache->getRecord(url);
        }
        record->incrementObserversCount();
        readPointer=0;
        request.clear();
    }
    return true;
}

size_t ClientHandler::findFirstSpChar(std::string in) {
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

std::string ClientHandler::getHost(std::string in){
    std::string req = std::move(in);
    std::string afterHostString = req.substr(req.find("Host:") + 6);
    size_t end = findFirstSpChar(afterHostString);
    return afterHostString.substr(0, end);
}

std::string ClientHandler::getServerMethodPath(std::string in, std::string HTTPMethod) {
    std::string req = std::move(in);
    req = req.substr(req.find(HTTPMethod) + HTTPMethod.length());
    req = req.substr(req.find('/'));
    int start = (req[1] == '/') ? 1 : 0;
    return req.substr(start, findFirstSpChar(req));
}

std::string ClientHandler::getUrl(std::string host, std::string serverMethodPath){
    return "http://" + host + serverMethodPath;
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

void ClientHandler::buildRequest(std::string& HTTPMethod,
                                 std::string& serverMethodPath) {
    request.clear();
    request = HTTPMethod + " " + serverMethodPath + " HTTP/" + prVersion + "\r\n";
    request += "Accept: text/html\r\n";
    request += "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: keep-alive\r\n\r\n";
}

bool ClientHandler::isOneLineRequest() {
    return request.find_first_of('\n') == request.find_last_of('\n');
}

bool ClientHandler::RequestParser(){
    if (request.size() <= 8) return false;

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
    std::string serverMethodPath = getServerMethodPath(request, HTTPMethod);
    url = getUrl(host, serverMethodPath);
    size_t place = host.find(':');

    if (place != std::string::npos) {
        port = host.substr(place + 1, host.size() - place - 1);
        host = host.substr(0, host.find(':'));
    } else {
        port = "80";
    }

    std::cout << serverMethodPath << std::endl;
    std::cout << url << std::endl;

    if (isOneLineRequest()) {
        std::cout << "One line mode detected" << std::endl;
        buildRequest(HTTPMethod, serverMethodPath);
    }

    return true;
}

void cleanupClient(void *arg) {

    std::cout << "Client # " + std::to_string(pthread_self()) + " freeing" << '\n';
    std::cout << "======" << '\n';
    auto *client = (ClientHandler *) arg;

    close(client->getSocket());
    my_delete(client)
    std::cout << "Client # " + std::to_string(pthread_self()) + " freed resources" << '\n';
    //std::cout << "cleaned Client" << '\n';
}

pollfd addNewConnection(int socket, short event) {
    pollfd fd{};
    fd.fd = socket;
    fd.events = event;
    fd.revents = 0;
    return fd;
}

void * ClientHandler::clientHandlerRoutine(void *args) {

    auto *arguments = (std::tuple<int, Cache *> *) args;
    ClientHandler *client = nullptr;
    int selectedDescNum;
    std::cout << "Client hi" << '\n';

    pthread_cleanup_push(cleanupClient, client);
        client = new ClientHandler(std::get<0>(*arguments));
        client->setCache(std::get<1>(*arguments));
        //client->addNewConnection(POLLIN | POLLHUP);
        pollfd connection = addNewConnection(std::get<0>(*arguments), POLLIN | POLLHUP);

        while(true) {
            client->returnRead(&connection);
            pthread_testcancel();
            //std::cout << "thread " << client->clientSocket << " events declared " << connection.events << '\n';

            if ((selectedDescNum = poll(&connection, 1, timeOut)) == -1) {
                std::cerr << "Proxy: run poll internal error" << '\n';
                break;
            }

            //selectedDescNum += client->returnRead(client->connection);

            if(selectedDescNum == 0){
                continue;
            }

            auto activity = connection.revents;

            //std::cout <<"thread " << client->clientSocket << " event " << activity << '\n';

            if (activity & POLLHUP ) {
                if(client->record!= nullptr){
                    client->record->decrementObserversCount();
                }
                break;
            }

            if(activity & POLLERR){
                if(client->record!= nullptr){
                    client->record->decrementObserversCount();
                }
                break;
            }

            if(activity & POLLIN){
                //std::cout <<"thread " << client->clientSocket << " read from client" << '\n';
                if (client->receive(client)) {
                } else {
                    std::cerr << "send request Error:" << client->getErrorMsg() << '\n';
                    break;
                }
            }

            if (activity & POLLOUT) {
                //std::cout << "thread " << client->clientSocket<< " read from server" << '\n';
                if(!client->readFromCache(&connection)) {
                    break;
                }
            }
            connection.revents = 0;
        }
    pthread_cleanup_pop(0);

    close(client->getSocket());
    Proxy::setHandlerDone(pthread_self());
    my_delete(client)
    //std::cout << "Client #" + std::to_string(pthread_self()) + " done and freed resources" << '\n';
    return nullptr;
}

bool ClientHandler::createServer(ClientHandler* client, CacheRecord *record, const std::string &host, const std::string &request) {

    //std::cout << "createServer :: Record observer count " + std::to_string(record->getObserversCount()) << '\n';

    int serverSocket = connectToServer(client);
    if (serverSocket == -1) {
        std::cerr << "Cannot connect to: " + host + " closing connection." << '\n';
        return false;
    }
    client->serverSocket= serverSocket;
    //std::cout << "Connected to server " + host << '\n';

    auto args = new std::tuple<int, CacheRecord *, const std::string>(serverSocket, record, request);
    //serverArgs.push_back(args);
    pthread_t serverThread;
    if (pthread_create(&serverThread, nullptr, ServerHandler::serverHandlerRoutine, (void *) args) != 0) {
        std::cerr << "Proxy :: Failed to create server thread" << '\n';
        return false;
    }

    Proxy::addHandler(serverThread);
    //std::cout << "Created server thread";
    client->setServerThread(serverThread);
    return true;
}

// переделать на getaddrinfo
int ClientHandler::connectToServer(ClientHandler* client) {
    int serverSocket;
    if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Failed to open new socket";
        return -1;
    }

    struct hostent *hostinfo = gethostbyname(client->host.data());
    if (hostinfo == nullptr) {
        std::cerr << "Unknown host" + client->host;
        close(serverSocket);
        return -1;
    }

    struct sockaddr_in sockaddrIn{};
    sockaddrIn.sin_family = AF_INET;
    int port = std::atoi(client->port.c_str());
    sockaddrIn.sin_port = htons(port);
    sockaddrIn.sin_addr = *((struct in_addr *) hostinfo->h_addr);

    if ((connect(serverSocket, (struct sockaddr *) &sockaddrIn, sizeof(sockaddrIn))) == -1) {
        std::cerr << "Cannot connect to" + client->host;
        close(serverSocket);
        return -1;
    }
    return serverSocket;
}

void ClientHandler::setServerThread(pthread_t serverThread) {
    ClientHandler::serverThread = serverThread;
}

void ClientHandler::setCache(Cache *cache) {
    ClientHandler::cache = cache;
}

bool ClientHandler::sendRequest(const std::string& request) const {
    ssize_t len = write(serverSocket, request.data(), request.size());

    if (len == -1) {
        std::cerr << "Failed to send data to server" << '\n';
        return false;
    }
    return true;
}

short ClientHandler::returnRead(pollfd* conn) {
    if(record != nullptr && readPointer < record->getDataSize() && !(conn->events & POLLOUT)) {
        //std::cout << "return read ability" << '\n';
        conn->events |= POLLOUT;
        return 1;
    }
    return 0;
}
