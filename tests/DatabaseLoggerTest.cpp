#include <gtest/gtest.h>

namespace test::AIRESTAPI{
// The fixture for testing DatabaseLogger class
class DatabaseLoggerTest : public ::testing::Test
{
protected:
   void SetUp() override{

   }

   void TearDown() override{

   }
    
};

TEST_F(DatabaseLoggerTest, insertSingleRow)
{
    EXPECT_EQ(0, 0);
}

TEST_F(DatabaseLoggerTest, insertMultipleRows)
{
    EXPECT_EQ(0, 0);
}

TEST_F(DatabaseLoggerTest, searchDB)
{
    EXPECT_EQ(0, 0);
}

}// namespace test::AIRESTAPI

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}