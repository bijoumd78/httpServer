#include "DatabaseLogger.h"
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/JSON/Parser.h>
#include <Poco/String.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>
#include <Poco/Data/PostgreSQL/PostgreSQL.h>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/DataException.h>
#include <sstream>
#include <algorithm>


namespace Poco {
    namespace Data {

        template <>
        class TypeHandler<Row>
        {
        public:
            static void bind(std::size_t pos, const Common::Logging::Row& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
            {
                // the table is defined as logs (timestamp TIMESTAMPTZ, level TEXT, source TEXT, transaction_id INT, message JSONB/TEXT)
                poco_assert_dbg(!pBinder.isNull());
                pBinder->bind(pos++, obj.timestamp_, dir);
                pBinder->bind(pos++, obj.level_, dir);
                pBinder->bind(pos++, obj.source_, dir);
                pBinder->bind(pos++, obj.transactionId_, dir);
                pBinder->bind(pos++, obj.message_, dir);
            }

            static void prepare(std::size_t pos, const Common::Logging::Row& obj, AbstractPreparator::Ptr pPrepare)
            {
                // the table is defined as logs (timestamp TIMESTAMPTZ, level TEXT, source TEXT, transaction_id INT, message JSONB/TEXT)
                poco_assert_dbg(!pPrepare.isNull());
                pPrepare->prepare(pos++, obj.timestamp_);
                pPrepare->prepare(pos++, obj.level_);
                pPrepare->prepare(pos++, obj.source_);
                pPrepare->prepare(pos++, obj.transactionId_);
                pPrepare->prepare(pos++, obj.message_);
            }

            static std::size_t size()
            {
                return 5;
            }

            static void extract(std::size_t pos, Common::Logging::Row& obj, const Common::Logging::Row& defVal, AbstractExtractor::Ptr pExt)
            {
                poco_assert_dbg(!pExt.isNull());
                if (!pExt->extract(pos++, obj.timestamp_))
                    obj.timestamp_ = defVal.timestamp_;
                if (!pExt->extract(pos++, obj.level_))
                    obj.level_ = defVal.level_;
                if (!pExt->extract(pos++, obj.source_))
                    obj.source_ = defVal.source_;
                if (!pExt->extract(pos++, obj.transactionId_))
                    obj.transactionId_ = defVal.transactionId_;
                if (!pExt->extract(pos++, obj.message_))
                    obj.message_ = defVal.message_;
            }

        private:
            TypeHandler();
            ~TypeHandler();
            TypeHandler(const TypeHandler&);
            TypeHandler& operator=(const TypeHandler&);
        };
    }
} // namespace Poco::Data


namespace Common::Logging {
    // static data member
    std::unique_ptr<Poco::Data::Session>  DatabaseLogger::pSession_;

    using namespace Poco::Data::Keywords;
    using Poco::Data::PostgreSQL::PostgreSQLException;
    using Poco::Data::PostgreSQL::ConnectionException;
    using Poco::Data::PostgreSQL::StatementException;

    DatabaseLogger::DatabaseLogger(std::string_view configFile) :
        pConfig_{ std::make_unique<Configuration>(configFile) },
        user_{ pConfig_->getDbUser() },
        password_{ pConfig_->getDbPassword() },
        hostaddr_{ pConfig_->getDbHostAddr() },
        port_{ pConfig_->getDbPort() },
        level_{ pConfig_->getDbLoggingLevel() }
    {
        // Register connector
        Poco::Data::PostgreSQL::Connector::registerConnector();

        // Connection strings
        std::string dbConnString;
        dbConnString += "host=" + hostaddr_ + " ";
        dbConnString += "user=" + user_ + " ";
        dbConnString += "password=" + password_ + " ";
        dbConnString += "port=" + port_;

        try { pSession_ = std::make_unique<Poco::Data::Session>(Poco::Data::PostgreSQL::Connector::KEY, dbConnString); }
        catch (const Poco::Data::ConnectionFailedException& ex)
        { std::cout << "Postgres database connection exception: " << ex.displayText() << std::endl; }

        executeQuery(sql_create_);
    }

    DatabaseLogger::~DatabaseLogger()
    {
        // Unregister connector
        Poco::Data::PostgreSQL::Connector::unregisterConnector();
    }

    std::vector<std::string> DatabaseLogger::search(const std::string& pattern)
    {
        std::vector<std::string> timestamps;
        std::vector<std::string> levels;
        std::vector<std::string> sources;
        std::vector<int>         transactionIds;
        std::vector<std::string> messages;

        std::stringstream ss;
        ss << "SELECT * FROM logs WHERE to_tsvector(logs::text) @@ to_tsquery('" << pattern << "')";
        try { *pSession_ << ss.str(), into(timestamps), into(levels), into(sources), into(transactionIds), into(messages), now; }
        catch (ConnectionException& ce) { std::cout << ce.displayText() << std::endl; }

        std::vector<std::string> result;
        if (!timestamps.empty() && !levels.empty() && !sources.empty() && !transactionIds.empty() && !messages.empty())
        {
            for (size_t ii{ 0 }; ii < timestamps.size(); ++ii)
            {
                result.push_back(timestamps[ii] + " " + levels[ii] + " " + sources[ii] + " " + std::to_string(transactionIds[ii]) + " " + messages[ii]);
            }
        }
        return result;
    }

    void DatabaseLogger::logFatal(std::string_view source, const int transaction_id, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::fatal)
        {
            log("FATAL", source, transaction_id, msg);
        }
    }

    void DatabaseLogger::logError(std::string_view source, const int transaction_id, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::error)
        {
            log("ERROR", source, transaction_id, msg);
        }
    }

    void DatabaseLogger::logWarning(std::string_view source, const int transaction_id, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::warning)
        {
            log("WARNING", source, transaction_id, msg);
        }
    }

    void DatabaseLogger::logInfo(std::string_view source, const int transaction_id, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::information)
        {
            log("INFO", source, transaction_id, msg);
        }
    }

    void DatabaseLogger::logDebug(std::string_view source, const int transaction_id, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::debug)
        {
            log("DEBUG", source, transaction_id, msg);
        }
    }

    void DatabaseLogger::logTrace(std::string_view source, const int transaction_id, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::trace)
        {
            log("TRACE", source, transaction_id, msg);
        }
    }

    void DatabaseLogger::log(std::string_view level, std::string_view source, const int transaction_id, std::string_view msg)
    {
        // Get current time
        Poco::Timestamp now;
        const std::string timestamp = Poco::DateTimeFormatter::format(now, Poco::DateTimeFormat::SORTABLE_FORMAT);
        Row row(timestamp, level.data(), source.data(), transaction_id, checkJsonFromat(msg));
        insertSingleRow(row);
    }

    void DatabaseLogger::executeQuery(std::string_view query)
    {
        try { *pSession_ << query, now; }
        catch (const ConnectionException& ce) { std::cout << ce.displayText() << std::endl; }
    }

    void DatabaseLogger::insertSingleRow(Row& row)
    {
        Poco::Data::Statement stmt(*pSession_);
        stmt << "INSERT INTO logs VALUES ($1,$2,$3,$4,$5)", use(row.timestamp_), use(row.level_),
            use(row.source_), use(row.transactionId_), use(row.message_);
        try { stmt.execute(); }
        catch (const StatementException& se) { std::cout << se.displayText() << std::endl; }
    }

    Row  DatabaseLogger::selectSingleRow(const std::string& token)
    {
        std::vector<std::string> timestamps;
        std::vector<std::string> levels;
        std::vector<std::string> sources;
        std::vector<int>         transactionIds;
        std::vector<std::string> messages;

        std::stringstream ss;
        ss << "SELECT * FROM logs WHERE to_tsvector(logs::text) @@ to_tsquery('" << token << "')";
        try { *pSession_ << ss.str(), into(timestamps), into(levels), into(sources), into(transactionIds), into(messages), now; }
        catch (ConnectionException& ce) { std::cout << ce.displayText() << std::endl; }

        Row row;
        if (!timestamps.empty() && !levels.empty() && !sources.empty() && !transactionIds.empty() && !messages.empty())
        {
            row.timestamp_ = timestamps[0];
            row.level_ = levels[0];
            row.source_ = sources[0];
            row.transactionId_ = transactionIds[0];
            row.message_ = messages[0];
        }

        return row;
    }

    void DatabaseLogger::insertMultipleRows(multipleRows_t& rows)
    {
        const auto SIZE = rows.size();
        std::vector<std::string> timestamps;
        timestamps.reserve(SIZE);
        std::vector<std::string> levels;
        levels.reserve(SIZE);
        std::vector<std::string> sources;
        sources.reserve(SIZE);
        std::vector<int>         transactionIds;
        transactionIds.reserve(SIZE);
        std::vector<std::string> messages;
        messages.reserve(SIZE);

        std::for_each(rows.begin(), rows.end(), [&](tableData_t& e) {
            timestamps.push_back(e[0].data());
            levels.push_back(e[1].data());
            sources.push_back(e[2].data());
            transactionIds.push_back(std::stoi(e[3].data()));
            messages.push_back(e[4].data());
            });

        Poco::Data::Statement stmt(*pSession_);
        stmt << "INSERT INTO logs VALUES ($1,$2,$3,$4,$5)", use(timestamps), use(levels),
            use(sources), use(transactionIds), use(messages);
        try { stmt.execute(); }
        catch (const StatementException& se) { std::cout << se.displayText() << std::endl; }
    }

    std::string DatabaseLogger::checkJsonFromat(std::string_view msg) const
    {
        try {
            Poco::JSON::Parser parser;
            const auto result = parser.parse(msg.data());
            (void)result;
            return msg.data();
        }
        catch (const Poco::Exception&) {
            std::stringstream ss;
            ss << "{\"message\" : \"" << msg.data() << "\"}";
            return ss.str();
        }
    }
}

