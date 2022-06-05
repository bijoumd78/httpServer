#pragma once
#include "Poco/Redis/Client.h"
#include <Poco/Runnable.h>
#include <Logger.h>

namespace redissubscribe
{
    class RedisSubscribe : public Poco::Runnable
    {
    public:
        RedisSubscribe(std::string_view configFile, std::string_view topic);
        ~RedisSubscribe();

        static void subscribe(std::string_view topic);
        void run()override;

    private:
        std::unique_ptr<Common::Logging::Configuration> pConfig_;
        std::string                                     host_{ "localhost" };
        unsigned                                        port_{ 6379 };
        std::string_view                                topic_{};
        static bool                                     connected_;
        static Poco::Redis::Client                      redis_;
    };
}

