include_directories(../logger)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(AI_Image_Processing_Unit ${SOURCES} ${HEADERS})
target_include_directories(AI_Image_Processing_Unit PUBLIC ${Torch_INCLUDE_DIRS})
if(WIN32)
    add_dependencies(AI_Image_Processing_Unit logger)
endif()
target_link_libraries(AI_Image_Processing_Unit PUBLIC ${TORCH_LIBRARIES})
target_link_libraries(AI_Image_Processing_Unit PRIVATE logger)

set_target_properties(AI_Image_Processing_Unit PROPERTIES FOLDER "AI_Image_Processing_Unit")

install(TARGETS
    AI_Image_Processing_Unit
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