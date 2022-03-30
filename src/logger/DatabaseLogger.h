#pragma once
#include "ILogger.h"
#include "Configuration.h"
#include <vector>
#include <array>

namespace Common::Logging {
    using tableData_t = std::array<std::string_view, 5>;
    using multipleRows_t = std::vector<tableData_t>;

    class DatabaseLogger final : public ILogger
    {
    public:
        explicit DatabaseLogger(std::string_view configFile);
        ~DatabaseLogger() override = default;

        // Prevent copy construction and assignment operation.
        DatabaseLogger(const DatabaseLogger&) = delete;
        DatabaseLogger& operator=(const DatabaseLogger&) = delete;
       

        // Allow move construction and assignment operation.
        DatabaseLogger(DatabaseLogger&&) = default;
        DatabaseLogger& operator=(DatabaseLogger&&) = default;

        void logFatal(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logError(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logWarning(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logInfo(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logDebug(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logTrace(std::string_view source, const int transaction_id, std::string_view msg) override;

        void BulkInsert(const multipleRows_t& rows);

    private:
        void log(std::string_view level, std::string_view source, const int transaction_id, std::string_view msg);
        void executeSQLQuery(std::string_view query);
        std::string checkJsonFromat(std::string_view msg)const;

        std::unique_ptr<Configuration> pConfig_;
        std::string       databaseName_{"Dev"};
        std::string       user_{"postgres"};
        std::string       password_{"password"};
        std::string       hostaddr_{"127.0.0.1"};
        std::string       port_{"5432"};
        std::string       level_{"information"};
        std::string       sql_create_{ "CREATE TABLE LOGS("                      \
                                       "TIMESTAMP        TIMESTAMPTZ,"           \
                                       "LEVEL            TEXT         NOT NULL," \
                                       "SOURCE           TEXT         NOT NULL," \
                                       "TRANSACTION_ID   INT          NOT NULL," \
                                       "MESSAGE          JSONB               );" };
    };

}

