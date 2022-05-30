#pragma once
#include "RedisPublish.h"
#include "RedisSubscribe.h"
#include <gtest/gtest.h>
#include <memory>

namespace test::AIRESTAPI {
    // The fixture for testing RedisPubSub classes
    class RedisPubSubTest : public ::testing::Test {
    public:
        // Connect to Redis server
        std::string configFile_{ "Config_Tests.json" };
        redispublish::RedisPublish* RedisPub;
        redissubscribe::RedisSubscribe* RedisSub;

    protected:
        void SetUp() override {
            RedisPub = new redispublish::RedisPublish(configFile_);
            RedisSub = new redissubscribe::RedisSubscribe(configFile_);
        }

        void TearDown() override {
            // Delete all caches
            delete RedisPub;
            delete RedisSub;
        }
    };
}
