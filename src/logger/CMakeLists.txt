file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(logger ${SOURCES} ${HEADERS})
target_include_directories(logger PRIVATE ${Poco_INCLUDE_DIRS})
target_link_libraries(logger PRIVATE Poco::Foundation Poco::JSON Poco::Util Poco::Net Poco::Data)

set_target_properties(logger PROPERTIES FOLDER "logger")
