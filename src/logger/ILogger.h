#pragma once
#include <string_view>
#include <Poco/String.h>
#include <Poco/Exception.h>

namespace Common::Logging {

    enum class Level : int
    {
        fatal = 1,   /// A fatal error. The application will most likely terminate. This is the highest priority.
        critical,    /// A critical error. The application might not be able to continue running successfully.
        error,       /// An error. An operation did not complete successfully, but the application as a whole is not affected.
        warning,     /// A warning. An operation completed with an unexpected result.
        notice,      /// A notice, which is an information with just a higher priority.
        information, /// An informational message, usually denoting the successful completion of an operation.
        debug,       /// A debugging message.
        trace        /// A tracing message. This is the lowest priority.
    };

    class ILogger {
    public:
        virtual ~ILogger() = default;

        virtual void logFatal(std::string_view source, const int transaction, std::string_view msg) = 0;
        virtual void logError(std::string_view source, const int transaction, std::string_view msg) = 0;
        virtual void logWarning(std::string_view source, const int transaction, std::string_view msg) = 0;
        virtual void logInfo(std::string_view source, const int transaction, std::string_view msg) = 0;
        virtual void logDebug(std::string_view source, const int transaction, std::string_view msg) = 0;
        virtual void logTrace(std::string_view source, const int transaction, std::string_view msg) = 0;

        static Level getLoggingLevel(const std::string& level)
        {
            if (Poco::icompare(level, "fatal") == 0)       return Level::fatal;
            if (Poco::icompare(level, "critical") == 0)    return Level::critical;
            if (Poco::icompare(level, "error") == 0)       return Level::error;
            if (Poco::icompare(level, "warning") == 0)     return Level::warning;
            if (Poco::icompare(level, "notice") == 0)      return Level::notice;
            if (Poco::icompare(level, "information") == 0) return Level::information;
            if (Poco::icompare(level, "debug") == 0)       return Level::debug;
            if (Poco::icompare(level, "trace") == 0)       return Level::trace;
            else
            {
                throw Poco::InvalidArgumentException("Unknown logging level");
            }
        }
    };

}