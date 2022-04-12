include_directories(../src/logger)

find_package(GTest REQUIRED)

add_executable(TestDatabaseLogger databaseLoggerTest.cpp)
target_link_libraries(TestDatabaseLogger PRIVATE GTest::GTest logger)
set_target_properties(TestDatabaseLogger PROPERTIES FOLDER "tests")
add_test(NAME DatabaseLoggerTest COMMAND COMMAND $<TARGET_FILE:TestDatabaseLogger> --success)
