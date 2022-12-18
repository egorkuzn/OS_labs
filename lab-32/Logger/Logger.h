//
// Created by egork on 18.12.2022.
//

#ifndef LAB32_LOGGER_H
#define LAB32_LOGGER_H

#include <iostream>
#include <string>

#define GREEN "\033[32m"
#define YELLOW "\033[33m"

class Logger {
public:
    static bool debugMode;
    Logger() = default;
    ~Logger() = default;
    static void log(const std::string& message, bool isDebugMessage);
};


#endif //LAB32_LOGGER_H
