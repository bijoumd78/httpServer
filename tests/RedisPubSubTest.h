#pragma once
#include "RedisPublish.h"
#include "RedisSubscribe.h"
#include "RedisCache.h"
#include <gtest/gtest.h>

namespace test::AIRESTAPI {
    // The fixture for testing RedisPubSub classes
    class RedisPubSubTest : public ::testing::Test {
    public:
        // Connect to Redis server
        std::string_view configFile_{ "Config_Tests.json" };
        Poco::SharedPtr<redispublish::RedisPublish> RedisPub;
        Poco::SharedPtr<redissubscribe::RedisSubscribe> RedisSub;

    protected:
        void SetUp() override 
        {
            RedisPub =  new redispublish::RedisPublish(configFile_);
            RedisSub = new redissubscribe::RedisSubscribe(configFile_, "test");
        }

        void TearDown() override 
        {
            // Delete all caches
            rediscache::RedisCache::flushall();
        }
    };
}
