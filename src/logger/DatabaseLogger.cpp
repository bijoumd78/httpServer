#include <pqxx/pqxx> 
#include "DatabaseLogger.h"
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/JSON/Parser.h>
#include <Poco/String.h>
#include <sstream>


namespace Common::Logging {
    DatabaseLogger::DatabaseLogger(std::string_view configFile):
        pConfig_{ std::make_unique<Configuration>(configFile) },
        databaseName_{ pConfig_->getDbName()},
        user_{ pConfig_->getDbUser()},
        password_{ pConfig_->getDbPassword()},
        hostaddr_{ pConfig_->getDbHostAddr()},
        port_{ pConfig_->getDbPort()},
        level_{ pConfig_->getDbLoggingLevel()}
    {
        executeSQLQuery(sql_create_);
    }

	std::vector<std::string> DatabaseLogger::search(const std::string & pattern)
	{
		return std::vector<std::string>();
	}

	void DatabaseLogger::logFatal(std::string_view source, const int transaction_id, std::string_view msg)
    {
        if (getLoggingLevel(level_) >= Level::fatal )
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

    void DatabaseLogger::BulkInsert(const multipleRows_t& rows)
    {
        //TODO: Need to finish this!
        // 
        // 
        // Build query value
        std::stringstream ss;
        ss << "INSERT INTO LOGS VALUES ";

        for (size_t ii = 0, SIZE = rows.size(); ii < SIZE; ++ii)
        {
            if (ii == SIZE - 1)
            {
                ss << "( \'" << rows[ii][0] << "\', \'" << rows[ii][1] << "\', \'" << rows[ii][2] << "\', \'" << rows[ii][3] << "\', \'" << rows[ii][4] << "\' )";
            }
            ss << "( \'" << rows[ii][0] << "\', \'" << rows[ii][1] << "\', \'" << rows[ii][2] << "\', \'" << rows[ii][3] << "\', \'" << rows[ii][4] << "\' ),";
        }

        //std::cout << ss.str() << std::endl;

        executeSQLQuery(ss.str());
    }

    void DatabaseLogger::log(std::string_view level, std::string_view source, const int transaction_id, std::string_view msg)
    {
        // Get current time
        Poco::Timestamp now;
        const auto timestamp = Poco::DateTimeFormatter::format(now, Poco::DateTimeFormat::SORTABLE_FORMAT);

        // Build query value
        std::stringstream ss;
        ss << "INSERT INTO LOGS VALUES ( \'" << timestamp << "\', \'" << level << "\', \'" << source 
           << "\', \'" << transaction_id << "\', \'" << checkJsonFromat(msg) << "\' )";

        executeSQLQuery(ss.str());
    }

    void DatabaseLogger::executeSQLQuery(std::string_view query)
    {
        try {
            std::stringstream ss;
            ss << "dbname = " << databaseName_ << " "
                << "user = " << user_ << " "
                << "password = " << password_ << " "
                << "hostaddr = " << hostaddr_ << " "
                << "port = " << port_;

            pqxx::connection C(ss.str());

            if (!C.is_open())
            {
                throw std::runtime_error("Can't open database");
            }

            // Create a transactional object.
            pqxx::work W(C);

            // Execute SQL query
            // First check if table already exist
            std::string_view s{ "SELECT EXISTS("                         \
                                "SELECT FROM information_schema.tables " \
                                "WHERE  table_schema = 'public' "        \
                                "AND    table_name = 'logs');"
            };

            auto result = W.exec(s);

            // If the table does not exist, create it
            if (query == sql_create_ && std::string{ result[0][0].c_str() } == "f")
            {
                W.exec(query);
                W.commit();
            }
            else if (query != sql_create_)
            {
                W.exec(query);
                W.commit();
            }
        }
        catch (const Poco::Exception&) {
            throw Poco::InvalidArgumentException("Execution of SQL query failed");
        }
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
            ss << "{\"message\" : \""<< msg.data() << "\"}";
            return ss.str();
        }
    }
}

