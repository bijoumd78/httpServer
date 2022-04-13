#pragma once
#include "ConsoleLogger.h"
#include "FileLogger.h"
#include "DatabaseLogger.h"
#include <Poco/Mutex.h>
#include <string_view>
#include <vector>
#include <memory>

namespace Common::Logging {

    // Forward enum declaration
    enum class Level;
    class ILogger;

    class Logger final
    {
    public:
        static void addChannel(ILogger* channelPtr);

        template<typename T>
        static void removeChannel();

        static void removeAllChannel();

        template<typename T>
        static std::vector<std::string> search(const std::string& pattern);

        static void log(std::string_view level, std::string_view source, int transaction, std::string_view message);

    private:
        static std::vector<ILogger*> channels_;
        static Poco::FastMutex mtx_;
    };

    // Explicit template instantiation
    template void Logger::removeChannel<ConsoleLogger>();
    template void Logger::removeChannel<FileLogger>();
    template void Logger::removeChannel<DatabaseLogger>();


    // Explicit template instantiation
    template std::vector<std::string> Logger::search<FileLogger>(const std::string& pattern);
    template std::vector<std::string> Logger::search<DatabaseLogger>(const std::string& pattern);

}

