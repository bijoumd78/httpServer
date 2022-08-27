#pragma once
#include <Poco/Util/JSONConfiguration.h>
#include <Poco/AutoPtr.h>
#include <string_view>
#include <functional>

namespace Common::Logging {

    using Poco::Util::JSONConfiguration;
    using Poco::AutoPtr;

    class Configuration : public Poco::Util::JSONConfiguration
    {
    public:
        Configuration() = delete;
        explicit Configuration(std::string_view configFile);
        ~Configuration() override = default;

        // Prevent copy construction and assignment operation.
        Configuration(const Configuration&) = delete;
        Configuration& operator=(const Configuration&) = delete;

        // Allow move construction and assignment operation.
        Configuration(Configuration&&) = default;
        Configuration& operator=(Configuration&&) = default;

        //Load config file
        void loadConfigFile(std::string_view path);

        std::string              getRedisHost()const;
        unsigned int             getRedisPort()const;
        std::string              getConsoleLoggingLevel()const;
        std::string              getFilePath()const;
        std::string              getFileRotation()const;
        std::string              getFileArchive()const;
        std::string              getFilePattern()const;
        std::string              getFileTimes()const;
        std::string              getFileCompress()const;
        std::string              getFilePurgeAge()const;
        std::string              getFilePurgeCount()const;
        std::string              getFileLoggingLevel()const;
        std::string              getDbName()const;
        std::string              getDbUser()const;
        std::string              getDbPassword()const;
        std::string              getDbHostAddr()const;
        std::string              getDbPort()const;
        std::string              getDbLoggingLevel()const;
        std::string              getDbTableName()const;

        void setRedisHost(std::string_view host);
        void setRedisPort(unsigned int port);
        void setConsoleLoggingLevel(std::string_view consoleLoggingLevel);
        void setFilePath(std::string_view filePath);
        void setFileRotation(std::string_view fileRotation);
        void setFileArchive(std::string_view fileArchive);
        void setFilePattern(std::string_view filePattern);
        void setFileTimes(std::string_view fileTimes);
        void setFileCompress(std::string_view fileCompress);
        void setFilePurgeAge(std::string_view filePurgeAge);
        void setFilePurgeCount(std::string_view filePurgeCount);
        void setFileLoggingLevel(std::string_view fileLoggingLevel);
        void setDbName(std::string_view dbName);
        void setDbUser(std::string_view dbUser);
        void setDbPassword(std::string_view dbPassword);
        void setDbHostAddr(std::string_view dbHostAddr);
        void setDbLoggingLevel(std::string_view dbLoggingLevel);
        void setDbPort(std::string_view dbPort);
        void setDbTableName(std::string_view dbTableName);

    private:
        template<typename T>
        void getConfigParams(T& value, const std::string& key, std::function<T(const std::string&)> Fun);

        struct{
            inline static std::string  host{"localhost"};
            inline static unsigned int port{ 8100 };
        }redis_;

        struct {
            inline static std::string loggingLevel{"information"};
        }console_;

        struct {
            inline static std::string path{"logs/my_logFile"};
            inline static std::string rotation{ "2 K" };
            inline static std::string archive{ "timestamp" };
            inline static std::string pattern{ "%Y-%m-%d %H:%M:%S %s %t" };
            inline static std::string times{ "local" };
            inline static std::string compress{ "false" };
            inline static std::string purgeAge{ "12 months" };
            inline static std::string purgeCount{ "100" };
            inline static std::string loggingLevel{"information"};
        }file_;

        struct {
            inline static std::string loggingLevel{"information"};
            inline static std::string name{"Dev"};
            inline static std::string user{"postgres"};
            inline static std::string password{"password"};
            inline static std::string host{"localhost"};
            inline static std::string port{"5432"};
            inline static std::string tableName{ "logs" };
        }database_;

        AutoPtr<JSONConfiguration> pConfJson{ new JSONConfiguration };
    };

    template<typename T>
    inline void Configuration::getConfigParams(T& value, const std::string& key, std::function<T(const std::string&)> Fun)
    {
        try {
            value = Fun(key);
        }
        catch (const Poco::Exception& e)
        {
            if (Poco::icompare(std::string{ e.what() }, "Not found") == 0 ||
                Poco::icompare(std::string{ e.what() }, "Invalid access") == 0)
            {
                //no-op, use default value
            }
            else
            {
                throw Poco::InvalidArgumentException("SyntaxException, the property can not be converted to type that it is assigned to");
            }
        }
    }
}

