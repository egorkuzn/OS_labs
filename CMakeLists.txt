cmake_minimum_required(VERSION 3.22.1)
project(OS_labs)

set(CMAKE_CXX_STANDARD 11)

add_executable(OS_labs
        lab-31/Main.cpp
        lab-31/Proxy.cpp lab-31/Proxy.h
        lab-31/Handles/ConnectionHandler.h
        lab-31/Cache/CacheRecord.cpp lab-31/Cache/CacheRecord.h
        lab-31/Cache/Cache.cpp lab-31/Cache/Cache.h)
