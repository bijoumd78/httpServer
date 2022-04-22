#pragma once
#include "RedisCache.h"
#include <gtest/gtest.h>
#include <memory>

namespace test::AIRESTAPI {
    // The fixture for testing FileLogger class
    class RedisCacheTest : public ::testing::Test
    {
    protected:
        void SetUp() override {
           // Connect to Redis server
            std::string configFile_{ "Config_Tests.json" };
            rediscache::RedisCache Redis(configFile_);
        }

        void TearDown() override {
            // Delete all caches
            rediscache::RedisCache::flushall();
        }
    };
}
