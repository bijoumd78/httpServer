include_directories(../logger)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/gflags")
    function(add_gflags)
        if(NOT WIN32)
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-all")
            set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-all")
        endif()
        set(BUILD_SHARED_LIBS OFF)
        add_subdirectory(thirdparty/gflags EXCLUDE_FROM_ALL)
        set_target_properties(gflags_nothreads_static PROPERTIES FOLDER thirdparty)
    endfunction()
    add_gflags()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/common/utils")
    add_subdirectory(common/utils)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/common/opencv_c_wrapper")
    add_subdirectory(common/opencv_c_wrapper)
endif()

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/HighResolution.cpp) 
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/HighResolution.h)

add_library(AI_Image_Processing_Unit2 ${SOURCES} ${HEADERS})
target_compile_definitions(AI_Image_Processing_Unit2 PUBLIC USE_OPENCV)
target_include_directories(AI_Image_Processing_Unit2 PUBLIC ${InferenceEngine_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/common/utils/include"  )

if(WIN32)
    add_dependencies(AI_Image_Processing_Unit2 logger)
endif()

target_link_libraries(AI_Image_Processing_Unit2 PUBLIC ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES} ie_samples_utils gflags)
target_link_libraries(AI_Image_Processing_Unit2 PRIVATE logger)

set_target_properties(AI_Image_Processing_Unit2 PROPERTIES FOLDER "AI_Image_Processing_Unit2")

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