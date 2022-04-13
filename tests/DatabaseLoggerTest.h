#pragma once
#include "Logger.h"
#include <gtest/gtest.h>
#include <memory>

namespace test::AIRESTAPI {

    // The fixture for testing DatabaseLogger class
    class DatabaseLoggerTest : public ::testing::Test
    {
    public:
        std::unique_ptr<Common::Logging::DatabaseLogger> pDb_;
        std::string configFile_{ "Config_Tests.json" };

    protected:
        ~DatabaseLoggerTest() override = default;

        void SetUp() override {
            // Create table 
            pDb_ = std::make_unique< Common::Logging::DatabaseLogger>(configFile_);
        }

        void TearDown() override {
            // Drop table
            pDb_->executeQuery("DROP TABLE IF EXISTS " + Common::Logging::DatabaseLogger::getTableName() + ";");
        }
    };
}