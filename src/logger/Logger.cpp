#include "Logger.h"
#include "ILogger.h"
#include <Poco/File.h>
#include <Poco/RegularExpression.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/Path.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/StringTokenizer.h>
#include <fstream>

namespace Common::Logging {
    // Initialize static data members
    std::vector<ILogger*> Logger::channels_{};
    Poco::FastMutex Logger::mtx_;

    void Logger::addChannel(ILogger* channelPtr)
    {
        Poco::FastMutex::ScopedLock lock(mtx_);
        channels_.push_back(channelPtr);
    }

    template<typename T>
    void Logger::removeChannel()
    {
        Poco::FastMutex::ScopedLock lock(mtx_);
        if (!channels_.empty())
        {
            auto itr = std::find_if(std::begin(channels_), std::end(channels_),
                [](Common::Logging::ILogger* logger) { return typeid(*logger) == typeid(T); });

            if (itr != channels_.end())
            {
                channels_.erase(itr);
            }
        }
    }

    void Logger::removeAllChannel()
    {
        Poco::FastMutex::ScopedLock lock(mtx_);
        if (!channels_.empty())
        {
            channels_.clear();
        }
    }

    template<typename T>
    std::vector<std::string> Logger::search(const std::string& pattern)
    {
        Poco::FastMutex::ScopedLock lock(mtx_);
        if (!channels_.empty())
        {
            auto itr = std::find_if(std::begin(channels_), std::end(channels_),
                [](Common::Logging::ILogger* logger) { return typeid(*logger) == typeid(T); });

            if (itr != channels_.end())
            {
                return T::search(pattern);
            }
        }
        return std::vector<std::string>{};
    }

    void Logger::log(std::string_view level, std::string_view source, int transaction, std::string_view msg)
    {
        Poco::FastMutex::ScopedLock lock(mtx_);

        if (!channels_.empty())
        {
            for (const auto& e : channels_)
            {
                switch (ILogger::getLoggingLevel(level.data()))
                {
                case Level::fatal:
                    e->logFatal(source, transaction, msg);
                    break;
                case Level::error:
                    e->logError(source, transaction, msg);
                    break;
                case Level::warning:
                    e->logWarning(source, transaction, msg);
                    break;
                case Level::information:
                    e->logInfo(source, transaction, msg);
                    break;
                case Level::debug:
                    e->logDebug(source, transaction, msg);
                    break;
                case Level::trace:
                    e->logTrace(source, transaction, msg);
                    break;
                default:
                    throw Poco::InvalidArgumentException("Unknown logging level");
                }
            }
        }
    }
}
