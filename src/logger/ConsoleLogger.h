#pragma once
#include "ILogger.h"
#include "Configuration.h"
#include <Poco/Mutex.h>

namespace Common::Logging {

    class ConsoleLogger final : public ILogger
    {
    public:
        ConsoleLogger() = delete;
        explicit ConsoleLogger(std::string_view configFile);
        ~ConsoleLogger() override= default;

        // Prevent copy construction and assignment operation.
        ConsoleLogger(const ConsoleLogger&) = delete;
        ConsoleLogger& operator=(const ConsoleLogger&) = delete;

        // Allow move construction and assignment operation.
        ConsoleLogger(ConsoleLogger&&) = default;
        ConsoleLogger& operator=(ConsoleLogger&&) = default;

        void logFatal(std::string_view source, const int transaction, std::string_view msgconst, const char* fileName , const int lineNumber) override;
        void logError(std::string_view source, const int transaction, std::string_view msgconst, const char* fileName, const int lineNumber) override;
        void logWarning(std::string_view source, const int transaction, std::string_view msg) override;
        void logInfo(std::string_view source, const int transaction, std::string_view msg) override;
        void logDebug(std::string_view source, const int transaction, std::string_view msg) override;
        void logTrace(std::string_view source, const int transaction, std::string_view msg) override;

    private:
        void log( std::string_view level, std::string_view source, const int transaction_id, std::string_view msg, const char* fileName, const int lineNumber);
        void log( std::string_view level, std::string_view source, const int transaction_id, std::string_view msg);
        std::unique_ptr<Configuration> pConfig_;
        std::string level_{ "information" };
        std::string times_{ "UTC" };
    };

}

