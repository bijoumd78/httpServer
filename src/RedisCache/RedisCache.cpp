#include "RedisCache.h"


namespace rediscache
{
    using namespace Poco::Redis;

    bool RedisCache::connected_{ false };
    Poco::Redis::Client RedisCache::redis_;

    RedisCache::RedisCache(std::string_view configFile):
        pConfig_{ std::make_unique<Common::Logging::Configuration>(configFile) }
    {
        host_ = pConfig_->getRedisHost();
        port_ = pConfig_->getRedisPort();

        if (!connected_)
        {
            std::stringstream ss;
            try
            {
                // Connect within 10 seconds
                Poco::Timespan t(10, 0); 
                redis_.connect(host_, port_, t);
                connected_ = true;
                ss << "Connected to [" << host_ << ':' << port_ << ']';
                Common::Logging::Logger::log("information", "Cache", -1, ss.str());
            }
            catch (const Poco::Exception& e)
            {
                ss.str(std::string{});
                ss << "Couldn't connect to [" << host_ << ':' << port_ << ']' << e.message() << ".";
                Common::Logging::Logger::log("error", "Cache", -1, ss.str());
            }
        }
    }

    RedisCache::~RedisCache()
    {
        if (connected_)
        {
            redis_.disconnect();
            connected_ = false;
            std::stringstream ss;
            ss << "Disconnected from [" << host_ << ':' << port_ << ']';
            Common::Logging::Logger::log("information", "Cache", -1, ss.str());
        }
    }
}