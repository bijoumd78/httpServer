#pragma once
#include "ILogger.h"
#include "Configuration.h"
#include <Poco/Data/Session.h>
#include <libpq-fe.h>
#include <vector>
#include <array>

namespace Common::Logging {
    using tableData_t = std::array<std::string_view, 5>;
    using multipleRows_t = std::vector<tableData_t>;

    struct Row {
        Row()
        {
            transactionId_ = -1;
        }

        Row(const std::string& timestamp, const std::string& level,
            const std::string& source, int transactionId, const std::string& message) :
            timestamp_{ timestamp },
            level_{ level },
            source_{ source },
            transactionId_{ transactionId },
            message_{ message }
        {}

        bool operator==(const Row& other) const
        {
            return timestamp_ == other.timestamp_      &&
                level_ == other.level_                 &&
                source_ == other.source_               &&
                transactionId_ == other.transactionId_ &&
                message_ == other.message_;
        }

        bool operator<(const Row& other)
        {
            if (timestamp_ < other.timestamp_)
                return true;
            if (level_ < other.level_)
                return true;
            if (source_ < other.source_)
                return true;
            if (transactionId_ < other.transactionId_)
                return true;
            if (message_ < other.message_)
                return true;
        }

        std::string timestamp_;
        std::string level_;
        std::string source_;
        int         transactionId_;
        std::string message_;
    };

    class DatabaseLogger final : public ILogger
    {
    public:
        explicit DatabaseLogger(std::string_view configFile);
        ~DatabaseLogger()override;

        // Prevent copy construction and assignment operation.
        DatabaseLogger(const DatabaseLogger&) = delete;
        DatabaseLogger& operator=(const DatabaseLogger&) = delete;

        // Allow move construction and assignment operation.
        DatabaseLogger(DatabaseLogger&&) = default;
        DatabaseLogger& operator=(DatabaseLogger&&) = default;

        // Search log files
        static std::vector<std::string> search(const std::string& pattern);
        void insertSingleRow(Row& row);
        Row  selectSingleRow(const std::string& token);
        static void insertMultipleRows(multipleRows_t& rows);
        void executeQuery(const std::string& query);
        static std::string getTableName();

        void logFatal(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logError(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logWarning(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logInfo(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logDebug(std::string_view source, const int transaction_id, std::string_view msg) override;
        void logTrace(std::string_view source, const int transaction_id, std::string_view msg) override;

    private:
        void log(std::string_view level, std::string_view source, const int transaction_id, std::string_view msg);
        std::string checkJsonFromat(std::string_view msg)const;

        std::unique_ptr<Configuration> pConfig_;
        static std::unique_ptr<Poco::Data::Session> pSession_;
        std::string        user_{ "postgres" };
        std::string        password_{ "password" };
        std::string        hostaddr_{ "127.0.0.1" };
        std::string        port_{ "5432" };
        std::string        level_{ "information" };
        static std::string tableName_;
    };

}

