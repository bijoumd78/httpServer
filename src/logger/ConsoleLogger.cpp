#include "ConsoleLogger.h"
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <sstream>
#include <iomanip>

namespace Common::Logging {
    ConsoleLogger::ConsoleLogger(std::string_view configFile):
        pConfig_{std::make_unique<Configuration>(configFile)},
        level_{ pConfig_->getConsoleLoggingLevel() }
    {
    }

    void ConsoleLogger::logFatal(std::string_view source, const int transaction, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::fatal)
        {
            log("[FATAL]", source, transaction, msg);
        }
    }

    void ConsoleLogger::logError(std::string_view source, const int transaction, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::error)
        {
            log("[ERROR]", source, transaction, msg);
        }
    }

    void ConsoleLogger::logWarning(std::string_view source, const int transaction, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::warning)
        {
            log("[WARNING]", source, transaction, msg);
        }
    }

    void ConsoleLogger::logInfo(std::string_view source, const int transaction, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::information)
        {
            log("[INFO]", source, transaction, msg);
        }
    }

    void ConsoleLogger::logDebug(std::string_view source, const int transaction, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::debug)
        {
            log("[DEBUG]", source, transaction, msg);
        }
    }

    void ConsoleLogger::logTrace(std::string_view source, const int transaction, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::trace)
        {
            log("[TRACE]", source, transaction, msg);
        }
    }

    void ConsoleLogger::log(std::string_view level, std::string_view source, const int transaction_id, std::string_view msg)
    {
        // Get current time
        Poco::Timestamp now;
        const auto timestamp = Poco::DateTimeFormatter::format(now, Poco::DateTimeFormat::SORTABLE_FORMAT);
        std::stringstream ss;
        ss << std::left
           << std::setw(20) << timestamp.c_str()
           << std::setw(10) << level
           << " " << source << " "
           << std::setw(4) << transaction_id
           << msg;
        puts(ss.str().c_str());
    }

}