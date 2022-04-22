include_directories(../src/logger)
include_directories(../src/RedisCache)

find_package(GTest REQUIRED)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(TestSuite ${SOURCES} ${HEADERS})
target_link_libraries(TestSuite PRIVATE GTest::GTest logger RedisCache)
set_target_properties(TestSuite PROPERTIES FOLDER "tests")
add_test(NAME TestSuite COMMAND $<TARGET_FILE:TestSuite> --success)
