//
// Created by egork on 18.12.2022.
//

#include "Logger.h"

bool Logger::debugMode = false;

void Logger::log(const std::string &message, bool isDebugMessage) {
    if (!isDebugMessage) {
        std::cerr << GREEN << msg << GREEN << std::endl;
    } else if (debugMode) {
        std::cerr << YELLOW << msg << YELLOW << std::endl;
    }
}
