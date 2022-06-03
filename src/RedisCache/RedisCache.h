#pragma once
#include "Poco/Redis/Redis.h"
#include "Poco/Redis/Client.h"
#include <Logger.h>
#include <optional>
#include <initializer_list>
#include <vector>

namespace rediscache {
    
    class RedisCache
    {
    public:
        explicit RedisCache(std::string_view configFile);
        ~RedisCache();
        
        // Set a key to a value
        //SET key value
        static bool set(std::string_view key, std::string_view value);

        // Set a key to a value with an expiration time in second
        // SETEX key value EX time(seconds)
        static bool setex(std::string_view key, std::string_view value, unsigned sec);
        
        // Set multiple [key - value] in one command
        // MSET key1 "value1" key2 "value2" key3 "value3"
        static bool mset(const std::initializer_list<std::string>& keysValues);

        // Get value correponding to a key
        // Get key
        static std::optional<std::string> get(std::string_view key);

        // Get multiple values corresponding to multiple keys
        // MGET key1 key2 key3 key4
        static std::optional<std::vector<std::string>> mget(const std::initializer_list<std::string>& keys);

        // Set field in the hash stored at key to value
        // HSET myhash key value
        static bool hset(std::string_view key, std::string_view field, std::string_view value);

        // Get value for field in the hash stored at key to value
        // HGET myhash key
        static std::optional<std::string> hget(std::string_view key, std::string_view field);

        // Delete key for field in the hash stored at key to value
        // DEL key
        static bool hdel(std::string_view key, std::string_view field);

        // Delete key
        // DEL key
        static bool delKey(std::string_view key);

        // Delete all caches
        static bool flushall();

    private:
        std::unique_ptr<Common::Logging::Configuration> pConfig_;
        std::string                                     host_{"localhost"};
        unsigned                                        port_{ 6379 };
        static bool                                     connected_;
        static Poco::Redis::Client                      redis_;
    };
}
