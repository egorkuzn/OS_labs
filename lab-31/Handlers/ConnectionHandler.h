//
// Created by egork on 05.12.2022.
//

#ifndef OS_LABS_CONNECTIONHANDLER_H
#define OS_LABS_CONNECTIONHANDLER_H

namespace lab31 {
    class ConnectionHandler {
    public:
        virtual ~ConnectionHandler() = default;

        virtual void deleteCache() = 0;

        virtual CacheRecord* getCacheRecord() = 0;

        virtual bool handle(int event) = 0;

        virtual size_t getReadElements() = 0;
    private:
    };
}

#endif //OS_LABS_CONNECTIONHANDLER_H
