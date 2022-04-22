#include "RedisCache.h"
#include "Poco/Redis/Command.h"

namespace rediscache
{
    using namespace Poco::Redis;

    bool RedisCache::connected_{ false };
    Poco::Redis::Client RedisCache::redis_;

    RedisCache::RedisCache(std::string_view configFile):
        pConfig_{ std::make_unique<Common::Logging::Configuration>(configFile) }
    {
        host_ = pConfig_->getRedisHost();
        port_ = pConfig_->getRedisPort();

        if (!connected_)
        {
            std::stringstream ss;
            try
            {
                // Connect within 10 seconds
                Poco::Timespan t(10, 0); 
                redis_.connect(host_, port_, t);
                connected_ = true;
                ss << "Connected to [" << host_ << ':' << port_ << ']';
                Common::Logging::Logger::log("information", "RedisCache", -1, ss.str());
            }
            catch (const Poco::Exception& e)
            {
                ss.str(std::string{});
                ss << "Couldn't connect to [" << host_ << ':' << port_ << ']' << e.message() << ".";
                Common::Logging::Logger::log("error", "RedisCache", -1, ss.str());
            }
        }
    }

    RedisCache::~RedisCache()
    {
        if (connected_)
        {
            redis_.disconnect();
            connected_ = false;
            std::stringstream ss;
            ss << "Disconnected from [" << host_ << ':' << port_ << ']';
            Common::Logging::Logger::log("information", "RedisCache", -1, ss.str());
        }
    }

    bool RedisCache::set(std::string_view key, std::string_view value)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return false;
        }

        auto set = Command::set(key.data(), value.data());
        // A set responds with a simple OK string
        try
        {
            std::string result = redis_.execute<std::string>(set);
            return result.compare("OK") == 0;
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        return false;
    }

    bool RedisCache::mset(const std::initializer_list<std::string>& keysValues)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return false;
        }

        Command command("MSET");
        for (const auto& e : keysValues)
        {
            command << e;
        }

        // A MSET responds with a simple OK string
        try
        {
            std::string result = redis_.execute<std::string>(command);
            return result.compare("OK") == 0;
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        return false;
    }

    std::optional<std::string> RedisCache::get(std::string_view key)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return{};
        }

        Array command;
        command.add("GET").add(key.data());
        try
        {
            BulkString s = redis_.execute<BulkString>(command);
            if (!s.isNull()) { return s.value(); }
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        return{};
    }

    std::optional<std::vector<std::string>> RedisCache::mget(const std::initializer_list<std::string>& keys)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return{};
        }

        Command command("MGET");
        for (const auto& e : keys)
        {
            command.add(e);
        }

        try
        {
            Array result = redis_.execute<Array>(command);

            std::vector<std::string> res;

            for (size_t ii{ 0 }; ii < result.size(); ++ii)
            {
                BulkString value = result.get<BulkString>(ii);

                if (value.isNull()) { res.emplace_back(""); }
                res.push_back(value.value());
            }
            return res;
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        catch (const Poco::BadCastException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }

        return {};
    }

    bool RedisCache::hset(std::string_view key, std::string_view field, std::string_view value)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return false;
        }

        Command hset = Command::hset(key.data(), field.data(), value.data());
        try
        {
            Poco::Int64 result = redis_.execute<Poco::Int64>(hset);
            return result == 1;
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        return false;
    }

    std::optional<std::string> RedisCache::hget(std::string_view key, std::string_view field)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return{};
        }

        Command hget = Command::hget(key.data(), field.data());
        try
        {
            BulkString s = redis_.execute<BulkString>(hget);
            if (!s.isNull()) { return s.value(); }
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        return{};
    }

    bool RedisCache::hdel(std::string_view key, std::string_view field)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return false;
        }

        Command hdel = Command::hdel(key.data(), field.data());
        try
        {
            Poco::Int64 result = redis_.execute<Poco::Int64>(hdel);
            return result == 1;
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        return false;
    }

    bool RedisCache::delKey(std::string_view key)
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return false;
        }

        Command delCommand = Command::del(key.data());
        try
        {
            Poco::Int64 result = redis_.execute<Poco::Int64>(delCommand);
            return result == 1;
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        catch (const Poco::BadCastException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        return false;
    }

    bool RedisCache::flushall()
    {
        if (!connected_)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, "Not connected to Redis server");
            return false;
        }

        Command command("FLUSHALL");

        // A flushall responds with a simple OK string
        try
        {
            std::string result = redis_.execute<std::string>(command);
            return result.compare("OK") == 0;
        }
        catch (const RedisException& e)
        {
            Common::Logging::Logger::log("error", "RedisCache", -1, e.message());
        }
        return false;
    }
}