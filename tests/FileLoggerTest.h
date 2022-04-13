#pragma once
#include "Logger.h"
#include <gtest/gtest.h>
#include <Poco/AutoPtr.h>
#include <mutex>

namespace test::AIRESTAPI {
    // The fixture for testing FileLogger class
    class FileLoggerTest : public ::testing::Test
    {
    public:
        Common::Logging::FileLogger* pFile_;
        std::string configFile_{ "Config_Tests.json" };

    protected:
        ~FileLoggerTest() override = default;

        void SetUp() override {
            // Create log directory
            pFile_ = new Common::Logging::FileLogger(configFile_);
        }

        void TearDown() override {
#if 0
            // Delete log directory
            using namespace std::filesystem;
            if (const auto& dir = Common::Logging::FileLogger::getLogDirectory(); exists(dir))
            {
                const auto n = remove_all(dir);
                std::cout << "Deleted " << n << " files or directories.\n";
            }
            delete pFile_;
#endif
        }
    };
}
