cmake_minimum_required(VERSION 3.2.1)
project(Lab32)
set(CMAKE_CXX_STANDARD 14)
add_executable(Lab32
        Main.cpp
        Proxy.cpp Proxy.h
        Cache/Cache.cpp Cache/Cache.h
        Cache/CacheRecord.cpp Cache/CacheRecord.h
        Handlers/ClientHandler.cpp Handlers/ClientHandler.h
        Handlers/ConnectionHandler.h
        Handlers/ServerHandler.cpp Handlers/ServerHandler.h Logger/Logger.h Logger/Logger.cpp)