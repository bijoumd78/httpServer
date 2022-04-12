include_directories(../src/logger)

find_package(GTest REQUIRED)

add_executable(TestSuite TestSuite.cpp)
target_link_libraries(TestSuite PRIVATE GTest::GTest logger)
set_target_properties(TestSuite PROPERTIES FOLDER "tests")
add_test(NAME TestSuite COMMAND $<TARGET_FILE:TestSuite> --success)
