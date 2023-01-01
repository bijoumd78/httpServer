include_directories(../logger)
include_directories(../AI_Image_Processing_Unit)
include_directories(../AI_Image_Processing_Unit2)


file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(HandleImageProcessing ${SOURCES} ${HEADERS})

if(WIN32)
    add_dependencies(HandleImageProcessing logger AI_Image_Processing_Unit AI_Image_Processing_Unit2)
endif()

target_link_libraries(HandleImageProcessing PRIVATE logger AI_Image_Processing_Unit AI_Image_Processing_Unit2)

set_target_properties(HandleImageProcessing PROPERTIES FOLDER "AI_Image_Processing")

install(TARGETS
    HandleImageProcessing
    ARCHIVE
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
    RUNTIME
    DESTINATION ${INSTALL_BINDIR}
    COMPONENT bin
    LIBRARY
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
    PUBLIC_HEADER
    DESTINATION ${INSTALL_INCLUDEDIR}
    COMPONENT dev)