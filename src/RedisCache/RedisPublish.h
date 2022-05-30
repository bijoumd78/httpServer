#pragma once
#include "Poco/Redis/Client.h"
#include <Logger.h>
#include <string_view>

namespace redispublish
{
    class RedisPublish
    {
    public:
        explicit RedisPublish(std::string_view configFile);
        ~RedisPublish();

        static void publish(std::string_view topic, std::string_view message);

    private:
        // Helper function
        static void executeCommand(const Poco::Redis::Array& com);

        std::unique_ptr<Common::Logging::Configuration> pConfig_;
        std::string                                     host_{ "localhost" };
        unsigned                                        port_{ 6379 };
        static bool                                     connected_;
        static Poco::Redis::Client                      redis_;
    };
}

