cmake_minimum_required(VERSION 3.2.1)
project(Lab31)
set(CMAKE_CXX_STANDARD 14)
add_executable(Lab31
        Main.cpp
        Proxy.cpp Proxy.h
        Cache/Cache.cpp Cache/Cache.h
        Cache/CacheRecord.cpp Cache/CacheRecord.h
        Handlers/ClientHandler.cpp Handlers/ClientHandler.h
        Handlers/ConnectionHandler.h
        Handlers/ServerHandler.cpp Handlers/ServerHandler.h)