#include "RedisPublish.h"
#include <Logger.h>
#include <chrono>

namespace redispublish
{
    using namespace Poco::Redis;

    RedisPublish::RedisPublish(std::string_view configFile) :
        pConfig_{ std::make_unique<Common::Logging::Configuration>(configFile) }
    {
        host_ = pConfig_->getRedisHost();
        port_ = pConfig_->getRedisPort();

        std::stringstream ss;

        if (!connected_)
        {
            try
            {
                Poco::Timespan t(10, 0); // Connect within 10 seconds
                redis_.connect(host_, port_, t);
                connected_ = true;
                ss << "Connected to [" << host_ << ':' << port_ << ']';
                Common::Logging::Logger::log("information", "RedisPublish", -1, ss.str());
            }
            catch (const Poco::Exception& e)
            {
                ss << "Couldn't connect to [" << host_ << ':' << port_ << ']' << e.message() << ". ";
                Common::Logging::Logger::log("error", "RedisPublish", -1, ss.str());
            }
        }
    }

    RedisPublish::~RedisPublish()
    {
        if (connected_)
        {
            redis_.disconnect();
            connected_ = false;
            std::stringstream ss;
            ss << "Disconnected from [" << host_ << ':' << port_ << ']';
            Common::Logging::Logger::log("information", "RedisPublish", -1, ss.str());
        }
    }

    void RedisPublish::executeCommand(const Poco::Redis::Array & com)
    {
        try {
            redis_.execute<void>(com);
            redis_.flush();
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisPublish", -1, e.message());
        }
        catch (const Poco::BadCastException& e)
        {
            Common::Logging::Logger::log("error", "RedisPublish", -1, e.message());
        }
    }

    void RedisPublish::publish(std::string_view topic, std::string_view message)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisPublish", -1, "Not connected to Redis server.");
            return;
        }

        Array publish;
        publish.add("PUBLISH").add(topic.data());
        publish.add(message.data());
        executeCommand(publish);
        publish.clear();
    }
}