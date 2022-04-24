#pragma once
#include "RedisCache.h"
#include <gtest/gtest.h>
#include <memory>

namespace test::AIRESTAPI {
    // The fixture for testing FileLogger class
    class RedisCacheTest : public ::testing::Test
    {
    public:
        // Connect to Redis server
        std::string configFile_{ "Config_Tests.json" };
        std::unique_ptr<rediscache::RedisCache> Redis;

    protected:
        void SetUp() override {
            Redis = std::make_unique<rediscache::RedisCache>(configFile_);
        }

        void TearDown() override {
            // Delete all caches
            rediscache::RedisCache::flushall();
        }
    };
}
