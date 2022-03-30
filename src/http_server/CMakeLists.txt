include_directories(../logger)
include_directories(../AI_Image_Processing_Unit)
include_directories(../AI_Image_Processing_Unit2)
include_directories(../HandleUserLogin)
include_directories(../HandleImageProcessing)


file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(http_server ${SOURCES} ${HEADERS})
target_include_directories(http_server PRIVATE ${Poco_INCLUDE_DIRS})

if(WIN32)
    add_dependencies(http_server logger HandleUserLogin HandleImageProcessing AI_Image_Processing_Unit AI_Image_Processing_Unit2)
endif()

target_link_libraries(http_server PRIVATE logger AI_Image_Processing_Unit AI_Image_Processing_Unit2 HandleUserLogin HandleImageProcessing )

set_target_properties(http_server PROPERTIES FOLDER "http_server")
