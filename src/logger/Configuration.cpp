#include "Configuration.h"
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

namespace Common::Logging {

    Configuration::Configuration(std::string_view configFile)
    {
        // Load the configuration file once 
        // This Lambda trick is equivalent to the call_once functionality but nicer.
        static auto tmp = [this, &configFile]() { loadConfigFile(configFile); return 0; }();
        (void)tmp;
    }

    void Configuration::loadConfigFile(std::string_view path)
    {
        // Load configuration file
        pConfJson->load(path.data());

        // Parse inputs
        getConfigParams<std::string>(redis_.host,  "redisHost", [this](const std::string& key) { return pConfJson->getString(key); });
        getConfigParams<unsigned int>(redis_.port, "redisPort", [this](const std::string& key) { return pConfJson->getUInt(key); });

        const auto json = pConfJson->getRawString("channelType");
        Poco::JSON::Parser parser;
        const auto result = parser.parse(json);
        const auto& arr   = result.extract<Poco::JSON::Array::Ptr>();

        auto SIZE = static_cast<int>(arr->size());
        while (SIZE > 0)
        {
            if (const auto object1 = arr->getObject(SIZE - 1); Poco::icompare(object1->getValue<std::string>("type"), "console") == 0)
            {
                getConfigParams<std::string>(console_.loggingLevel, "loggingLevel", [&object1](const std::string& key) { return object1->getValue<std::string>(key); });
                getConfigParams<std::string>(console_.times_,       "times",        [&object1](const std::string& key) { return object1->getValue<std::string>(key); });
            }
            else if (const auto object2 = arr->getObject(SIZE - 1); Poco::icompare(object2->getValue<std::string>("type"), "file") == 0)
            {
                getConfigParams<std::string>(file_.loggingLevel, "loggingLevel", [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
                getConfigParams<std::string>(file_.path,         "path",         [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
                getConfigParams<std::string>(file_.rotation,     "rotation",     [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
                getConfigParams<std::string>(file_.archive,      "archive",      [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
                getConfigParams<std::string>(file_.pattern,      "pattern",      [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
                getConfigParams<std::string>(file_.times,        "times",        [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
                getConfigParams<std::string>(file_.compress,     "compress",     [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
                getConfigParams<std::string>(file_.purgeAge,     "purgeAge",     [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
                getConfigParams<std::string>(file_.purgeCount,   "purgeCount",   [&object2](const std::string& key) { return object2->getValue<std::string>(key); });
            }
            else if(const auto object3 = arr->getObject(SIZE - 1); Poco::icompare(object3->getValue<std::string>("type"), "database") == 0)
            {
                database_.loggingLevel = object3->getValue<std::string>("loggingLevel");
                database_.loggingLevel = object3->getValue<std::string>("times");
                database_.name         = object3->getValue<std::string>("name");
                database_.user         = object3->getValue<std::string>("user");
                database_.password     = object3->getValue<std::string>("password");
                database_.host         = object3->getValue<std::string>("host");
                database_.port         = object3->getValue<std::string>("port");
                database_.tableName    = object3->getValue<std::string>("tableName");

                getConfigParams<std::string>(database_.loggingLevel, "loggingLevel", [&object3](const std::string& key) { return object3->getValue<std::string>(key); });
                getConfigParams<std::string>(database_.times_,       "times",        [&object3](const std::string& key) { return object3->getValue<std::string>(key); });
                getConfigParams<std::string>(database_.name,         "name",         [&object3](const std::string& key) { return object3->getValue<std::string>(key); });
                getConfigParams<std::string>(database_.user,         "user",         [&object3](const std::string& key) { return object3->getValue<std::string>(key); });
                getConfigParams<std::string>(database_.password,     "password",     [&object3](const std::string& key) { return object3->getValue<std::string>(key); });
                getConfigParams<std::string>(database_.host,         "host",         [&object3](const std::string& key) { return object3->getValue<std::string>(key); });
                getConfigParams<std::string>(database_.port,         "port",         [&object3](const std::string& key) { return object3->getValue<std::string>(key); });
                getConfigParams<std::string>(database_.tableName,    "tableName",    [&object3](const std::string& key) { return object3->getValue<std::string>(key); });
            }
            else
            {
                throw Poco::InvalidArgumentException("Invalid channel type");
            }
            --SIZE;
        }
    }

    std::string Configuration::getRedisHost() const
    {
        return redis_.host;
    }

    unsigned int Configuration::getRedisPort() const
    {
        return redis_.port;
    }

    std::string Configuration::getConsoleLoggingLevel() const
    {
        return console_.loggingLevel;
    }

    std::string Configuration::getConsoleTimeZone() const
    {
        return console_.times_;
    }

    std::string Configuration::getFilePath() const
    {
        return file_.path;
    }

    std::string Configuration::getFileRotation() const
    {
        return file_.rotation;
    }

    std::string Configuration::getFileArchive() const
    {
        return file_.archive;
    }

    std::string Configuration::getFilePattern() const
    {
        return file_.pattern;
    }

    std::string Configuration::getFileTimeZone() const
    {
        return file_.times;
    }

    std::string Configuration::getFileCompress() const
    {
        return file_.compress;
    }

    std::string Configuration::getFilePurgeAge() const
    {
        return file_.purgeAge;
    }

    std::string Configuration::getFilePurgeCount() const
    {
        return file_.purgeCount;
    }

    std::string Configuration::getFileLoggingLevel() const
    {
        return file_.loggingLevel;
    }

    std::string Configuration::getDbTimeZone() const
    {
        return database_.times_;
    }

    std::string Configuration::getDbName() const
    {
        return database_.name;
    }

    std::string Configuration::getDbUser() const
    {
        return database_.user;
    }

    std::string Configuration::getDbPassword() const
    {
        return database_.password;
    }

    std::string Configuration::getDbHostAddr() const
    {
        return database_.host;
    }

    std::string Configuration::getDbPort() const
    {
        return database_.port;
    }

    std::string Configuration::getDbLoggingLevel() const
    {
        return database_.loggingLevel;
    }

    std::string Configuration::getDbTableName() const
    {
        return database_.tableName;
    }

    void Configuration::setRedisHost(std::string_view host)
    {
        redis_.host = host;
    }

    void Configuration::setRedisPort(unsigned int port)
    {
        redis_.port = port;
    }

    void Configuration::setConsoleLoggingLevel(std::string_view consoleLoggingLevel)
    {
        console_.loggingLevel = consoleLoggingLevel;
    }

    void Configuration::setConsoleTimeZone(std::string_view consoleTimesZone)
    {
        console_.times_ = consoleTimesZone;
    }

    void Configuration::setFilePath(std::string_view filePath)
    {
        file_.path = filePath;
    }

    void Configuration::setFileRotation(std::string_view fileRotation)
    {
        file_.rotation = fileRotation;
    }

    void Configuration::setFileArchive(std::string_view fileArchive)
    {
        file_.archive = fileArchive;
    }

    void Configuration::setFilePattern(std::string_view filePattern)
    {
        file_.pattern = filePattern;
    }

    void Configuration::setFileTimeZone(std::string_view fileTimes)
    {
        file_.times = fileTimes;
    }

    void Configuration::setFileCompress(std::string_view fileCompress)
    {
        file_.compress = fileCompress;
    }

    void Configuration::setFilePurgeAge(std::string_view filePurgeAge)
    {
        file_.purgeAge = filePurgeAge;
    }

    void Configuration::setFilePurgeCount(std::string_view filePurgeCount)
    {
        file_.purgeCount = filePurgeCount;
    }

    void Configuration::setFileLoggingLevel(std::string_view fileLoggingLevel)
    {
        file_.loggingLevel = fileLoggingLevel;
    }

    void Configuration::setDbTimeZone(std::string_view dbTimeZone)
    {
        database_.times_ = dbTimeZone;
    }

    void Configuration::setDbName(std::string_view dbName)
    {
        database_.name = dbName;
    }

    void Configuration::setDbUser(std::string_view dbUser)
    {
        database_.user = dbUser;
    }

    void Configuration::setDbPassword(std::string_view dbPassword)
    {
        database_.password = dbPassword;
    }

    void Configuration::setDbHostAddr(std::string_view dbHostAddr)
    {
        database_.host = dbHostAddr;
    }

    void Configuration::setDbLoggingLevel(std::string_view dbLoggingLevel)
    {
        database_.loggingLevel = dbLoggingLevel;
    }

    void Configuration::setDbPort(std::string_view dbPort)
    {
        database_.port = dbPort;
    }

    void Configuration::setDbTableName(std::string_view dbTableName)
    {
        database_.tableName = dbTableName;
    }
}
