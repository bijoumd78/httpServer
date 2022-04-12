#include "Logger.h"
#include <gtest/gtest.h>
#include <Poco/Data/PostgreSQL/PostgreSQLException.h>
#include <Poco/StringTokenizer.h>
#include <sstream>

namespace test::AIRESTAPI{
    using namespace Common::Logging;
    using namespace Poco::Data::Keywords;
    using Poco::Data::PostgreSQL::ConnectionException;

// The fixture for testing DatabaseLogger class
class DatabaseLoggerTest : public ::testing::Test
{
public:
    DatabaseLogger* pDb_;

protected:
    ~DatabaseLoggerTest() override {}

   void SetUp() override{
       pDb_ = new  DatabaseLogger(configFile_);
   }

   void TearDown() override{
       delete pDb_;
   }

private:
    std::string configFile_{ "Config.json" };
};

TEST_F(DatabaseLoggerTest, insertSingleRow)
{
    tableData_t data{ "2022-04-10 21:13:37-04", "INFO", "TEST_SINGLE_ROW", "1", "{\"message\": \"This is a test\"}" };
    Row row(data[0].data(), data[1].data(), data[2].data(), std::stoi(data[3].data()), data[4].data());
    pDb_->insertSingleRow(row);

    // Retrieve the data from the database
    const auto results = pDb_->search(data[2].data());

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
    pDb_->insertMultipleRows(rows);

    // Retrieve the data from the database
    const auto results = pDb_->search("TEST_MULTIPLE_ROWS");
    const auto SIZE = results.size();

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
    pDb_->insertMultipleRows(rows);

    // Retrieve the data from the database
    const auto results = pDb_->search("TEST_SEARCH_MULTIPLE_ROWS");
    const auto SIZE = results.size();

    ASSERT_EQ(results.size(), rows.size());
}

}// namespace test::AIRESTAPI

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}