//
// Created by egork on 05.12.2022.
//

#include <iostream>
#include <csignal>
#include "Proxy.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Please, enter port number and try again." << std::endl;
        return -1;
    }

    sigset(SIGPIPE, SIG_IGN);
    sigset(SIGSTOP, SIG_IGN);

    struct sigaction act{};
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, nullptr);

    std::cout << "Starting proxy on port " << argv[1]  << " ..."<< std::endl;

    int port = atoi(argv[1]);
    auto* proxy = new Proxy(port);

    proxy->run();

    delete proxy;

    std::cout << "Application finished" << std::endl;

    return 0;
}
