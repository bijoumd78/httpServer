#pragma once
#include "Poco/Redis/Client.h"
#include <Logger.h>

namespace redissubscribe
{
    class RedisSubscribe
    {
    public:
        explicit RedisSubscribe(std::string_view configFile);
        ~RedisSubscribe();

        static void subscribe(std::string_view topic);
        static std::vector<std::string> getMessages();

    private:
        std::unique_ptr<Common::Logging::Configuration> pConfig_;
        std::string                                     host_{ "localhost" };
        unsigned                                        port_{ 6379 };
        static bool                                     connected_;
        static Poco::Redis::Client                      redis_;
        static std::vector<std::string>                 receivedMessages_;

    };
}

