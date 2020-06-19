#pragma once

#include <LoggerTarget.h>

class DisplayLoggerTarget : public LoggerTarget
{
    public:
        DisplayLoggerTarget(const char* loggerName, int logLevel);
        virtual void log(const char* logLevelText, const char* tag, const char* message);
        
    private:
        long id = 0;
};