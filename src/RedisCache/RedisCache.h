#pragma once
#include "Poco/Redis/Redis.h"
#include "Poco/Redis/Client.h"
#include <Logger.h>

namespace rediscache {
    
    class RedisCache
    {
    public:
        explicit RedisCache(std::string_view configFile);
        ~RedisCache();

    private:
        std::unique_ptr<Common::Logging::Configuration> pConfig_;
        std::string                                     host_{"127.0.0.1"};
        unsigned                                        port_{ 6379 };
        static bool                                     connected_;
        static Poco::Redis::Client                      redis_;
    };
}
