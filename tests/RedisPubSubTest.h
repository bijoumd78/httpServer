#pragma once
#include "RedisPublish.h"
#include "RedisSubscribe.h"
#include "RedisCache.h"
#include <gtest/gtest.h>
#include <memory>

namespace test::AIRESTAPI {
    // The fixture for testing RedisPubSub classes
    class RedisPubSubTest : public ::testing::Test {
    public:
        // Connect to Redis server
        std::string configFile_{ "Config_Tests.json" };
        std::unique_ptr<redispublish::RedisPublish> RedisPub;
        std::unique_ptr<redissubscribe::RedisSubscribe> RedisSub;

    protected:
        void SetUp() override {
            RedisPub = std::make_unique<redispublish::RedisPublish>(configFile_);
            RedisSub = std::make_unique<redissubscribe::RedisSubscribe>(configFile_);
        }

        void TearDown() override {
            // Delete all caches
            rediscache::RedisCache::flushall();
        }
    };
}
