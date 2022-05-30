#include "DatabaseLoggerTest.h"
#include "FileLoggerTest.h"
#include "RedisCacheTest.h"
#include "RedisPubSubTest.h"
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>
#include <Poco/StringTokenizer.h>
#include <sstream>
#include <thread>

namespace test::AIRESTAPI{
    using namespace  Common::Logging;
    using namespace rediscache;
    using namespace Poco::Data::Keywords;
    using Poco::Data::PostgreSQL::ConnectionException;

TEST_F(DatabaseLoggerTest, insertSingleRow)
{
    tableData_t data{ "2022-04-10 21:13:37-04", "INFO", "TEST_SINGLE_ROW", "1", "{\"message\": \"This is a test\"}" };
    Row row(data[0].data(), data[1].data(), data[2].data(), std::stoi(data[3].data()), data[4].data());
    pDb_->insertSingleRow(row);

    // Retrieve the data from the database
    const auto results = DatabaseLogger::search(data[2].data());

    std::vector<std::string> vec;
    if (!results.empty())
    {
        // If there is more than one result, choose the first row
        // Tokenize the row
        Poco::StringTokenizer tok(results[0], " ", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
        vec.push_back(tok[0] + " " + tok[1]);
        vec.push_back(tok[2]);
        vec.push_back(tok[3]);
        vec.push_back(tok[4]);
        vec.push_back(Poco::cat(std::string(" "), tok.begin() + 5, tok.end()));
    }

    EXPECT_TRUE(data[0].data() == vec[0]);
    EXPECT_TRUE(data[1].data() == vec[1]);
    EXPECT_TRUE(data[2].data() == vec[2]);
    EXPECT_TRUE  (data[3].data() == vec[3]);
    EXPECT_TRUE(data[4].data() == vec[4]);
}

TEST_F(DatabaseLoggerTest, insertMultipleRows)
{
    tableData_t data{ "2022-04-10 21:13:37-04", "INFO", "TEST_MULTIPLE_ROWS", "10", "{\"message\": \"This is a test\"}" };
    multipleRows_t rows(10, data);
    DatabaseLogger::insertMultipleRows(rows);

    // Retrieve the data from the database
    const auto results = DatabaseLogger::search("TEST_MULTIPLE_ROWS");

    ASSERT_EQ(results.size(), rows.size());

    int count{};

    for (const auto& e : results)
    {
        std::vector<std::string> vec;
        Poco::StringTokenizer tok(e, " ", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
        vec.push_back(tok[0] + " " + tok[1]);
        vec.push_back(tok[2]);
        vec.push_back(tok[3]);
        vec.push_back(tok[4]);
        vec.push_back(Poco::cat(std::string(" "), tok.begin() + 5, tok.end()));

        EXPECT_TRUE(rows[count][0] == vec[0]);
        EXPECT_TRUE(rows[count][1] == vec[1]);
        EXPECT_TRUE(rows[count][2] == vec[2]);
        EXPECT_TRUE(rows[count][3] == vec[3]);
        EXPECT_TRUE(rows[count][4] == vec[4]);
        ++count;
    }
}

TEST_F(DatabaseLoggerTest, searchDB)
{
    tableData_t data{ "2022-04-10 21:13:37-04", "INFO", "TEST_SEARCH_MULTIPLE_ROWS", "100", "{\"message\": \"This is a test\"}" };
    multipleRows_t rows(10, data);
    DatabaseLogger::insertMultipleRows(rows);

    // Retrieve the data from the database
    const auto results = DatabaseLogger::search("TEST_SEARCH_MULTIPLE_ROWS");

    ASSERT_EQ(results.size(), rows.size());
}

TEST_F(FileLoggerTest, searchLOG)
{
    // Log some values
    pFile_->logInfo("TEST_SEARCH_LOG", 1, "This is a test for the search function");
    pFile_->logDebug("TEST_SEARCH_LOG", 1, "This is a test for the search function");
    pFile_->logWarning("TEST_SEARCH_LOG", 1, "This is a test for the search function");
    pFile_->logFatal("TEST_SEARCH_LOG", 1, "This is a test for the search function");
    pFile_->logError("TEST_SEARCH_LOG", 1, "This is a test for the search function");
    pFile_->logTrace("TEST_SEARCH_LOG", 1, "This is a test for the search function");

    const auto result1 = FileLogger::search("Info");
    const auto result2 = FileLogger::search("Debug");
    const auto result3 = FileLogger::search("Warning");
    const auto result4 = FileLogger::search("Fatal");
    const auto result5 = FileLogger::search("Error");
    const auto result6 = FileLogger::search("Trace");
    
    EXPECT_EQ(result1.size(), 1);
    EXPECT_EQ(result2.size(), 1);
    EXPECT_EQ(result3.size(), 1);
    EXPECT_EQ(result4.size(), 1);
    EXPECT_EQ(result5.size(), 1);
    EXPECT_EQ(result6.size(), 1);

    const auto results = FileLogger::search("TEST_SEARCH_LOG");

    EXPECT_EQ(results.size(), 6);
}

TEST_F(RedisCacheTest, SET_GET)
{
    RedisCache::set("key1", "value1");
    RedisCache::set("key2", "value2");
    const auto result1 = RedisCache::get("key1");
    const auto result2 = RedisCache::get("key2");
    const auto result3 = RedisCache::get("key3");

    EXPECT_EQ(result1.value(), "value1" );
    EXPECT_EQ(result2.value(), "value2" );
    EXPECT_EQ(result3, std::nullopt );
}

TEST_F(RedisCacheTest, SETEX_GET)
{
    RedisCache::setex("key3", "value3", 10);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    const auto result1 = RedisCache::get("key3");
    EXPECT_EQ(result1, std::nullopt);

    RedisCache::setex("key4", "value4", 60);
    const auto result2 = RedisCache::get("key4");
    EXPECT_EQ(result2.value(), "value4");
}

TEST_F(RedisCacheTest, MSET_MGET)
{
    std::initializer_list<std::string> keys_values{ "key11", "value11", "key22", "value22", "key33", "value33" };
    RedisCache::mset(keys_values);

    std::initializer_list<std::string> keys{ "key11", "key22", "key33", "key44" };
    const auto results = RedisCache::mget(keys);

    EXPECT_EQ(results.value().size(), 4);
    EXPECT_EQ(results.value().at(0), "value11");
    EXPECT_EQ(results.value().at(1), "value22");
    EXPECT_EQ(results.value().at(2), "value33");
    EXPECT_EQ(results.value().at(3), "");
}

TEST_F(RedisCacheTest, DEL)
{
    RedisCache::set("key55", "value55");
    RedisCache::delKey("key55");
    const auto result = RedisCache::get("key55");

    EXPECT_EQ(result, std::nullopt);
}

TEST_F(RedisCacheTest, HSET_HGET)
{
    RedisCache::hset("Person", "first_name", "Jack");
    RedisCache::hset("Person", "last_name", "Olseen");
    RedisCache::hset("Person", "age", "25");

    const auto value1 = RedisCache::hget("Person", "first_name");
    const auto value2 = RedisCache::hget("Person", "last_name");
    const auto value3 = RedisCache::hget("Person", "age");

    EXPECT_EQ(value1, "Jack");
    EXPECT_EQ(value2, "Olseen");
    EXPECT_EQ(value3, "25");
}

TEST_F(RedisCacheTest, HDEL)
{
    RedisCache::hset("Employee", "first_name", "Alice");
    RedisCache::hset("Employee", "last_name", "Smith");
    RedisCache::hset("Employee", "age", "30");

    RedisCache::hdel("Employee", "last_name");
    RedisCache::hdel("Employee", "age");

    const auto value1 = RedisCache::hget("Employee", "first_name");
    const auto value2 = RedisCache::hget("Employee", "last_name");
    const auto value3 = RedisCache::hget("Employee", "age");

    EXPECT_EQ(value1, "Alice");
    EXPECT_EQ(value2, std::nullopt);
    EXPECT_EQ(value3, std::nullopt);
}

TEST_F(RedisPubSubTest, PUBSUB)
{
    std::thread Tpub(&redispublish::RedisPublish::publish, std::string_view{ "test" }, std::string_view{ "hello world" });
    redissubscribe::RedisSubscribe::subscribe("test");
    Tpub.join();

    const auto tmp = redissubscribe::RedisSubscribe::getMessages();

    EXPECT_EQ(tmp.size(), 2);
    EXPECT_EQ(tmp[0], "hello world");
    EXPECT_EQ(tmp[1], "break");
}

}// namespace test::AIRESTAPI

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}