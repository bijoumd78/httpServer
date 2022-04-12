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
        explicit Configuration(std::string_view configFile);
        ~Configuration() override = default;

        //Load config file
        void load(std::string_view path);

        unsigned int             getPort()const;
        bool                     useSsl()const;
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

        void setPort(unsigned int port);
        void setUseSsl(bool useSsl);
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

        unsigned int               port_{8100};
        bool                       useSsl_{false};

        struct {
            std::string loggingLevel{"information"};
        }console_;

        struct {
            std::string path{"logs/my_logFile"};
            std::string rotation{ "2 K" };
            std::string archive{ "timestamp" };
            std::string pattern{ "%Y-%m-%d %H:%M:%S %s %t" };
            std::string times{ "local" };
            std::string compress{ "false" };
            std::string purgeAge{ "12 months" };
            std::string purgeCount{ "100" };
            std::string loggingLevel{"information"};
        }file_;

        struct {
            std::string loggingLevel{"information"};
            std::string name{"Dev"};
            std::string user{"postgres"};
            std::string password{"password"};
            std::string host{"127.0.0.1"};
            std::string port{"5432"};
            std::string tableName{ "logs" };
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

