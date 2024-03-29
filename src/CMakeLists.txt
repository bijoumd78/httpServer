add_subdirectory(logger)
add_subdirectory(RedisCache)
add_subdirectory(http_server)
add_subdirectory(AI_Image_Processing_Unit)
add_subdirectory(AI_Image_Processing_Unit2)
add_subdirectory(HandleUserLogin)
add_subdirectory(HandleImageProcessing)

add_executable(AIRESTAPI main.cpp)
if(WIN32)
    add_dependencies(AIRESTAPI logger http_server AI_Image_Processing_Unit AI_Image_Processing_Unit2)
endif()
target_link_libraries(AIRESTAPI PUBLIC AI_Image_Processing_Unit AI_Image_Processing_Unit2 http_server logger)

#install
install (TARGETS
    AIRESTAPI
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

message (STATUS "Copying configuration files Config.json and AIRESTAPI.json to the binary directory")
add_custom_command(TARGET AIRESTAPI 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                    ${PROJECT_SOURCE_DIR}/config/Config.JSON 
                    ${PROJECT_SOURCE_DIR}/config/AIRESTAPI.json
                    $<TARGET_FILE_DIR:AIRESTAPI>)

if(ENABLE_UNIT_TESTS)
    message (STATUS "Copying configuration file Config_Tests.json to the binary directory")
    add_custom_command(TARGET AIRESTAPI 
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                        ${PROJECT_SOURCE_DIR}/config/Config_Tests.JSON 
                        $<TARGET_FILE_DIR:AIRESTAPI>)
endif()

message (STATUS "Copying wwwroot directory to the binary directory")
add_custom_command(TARGET AIRESTAPI 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory 
                    ${PROJECT_SOURCE_DIR}/src/wwwroot/
                    $<TARGET_FILE_DIR:AIRESTAPI>/wwwroot)
					
message (STATUS "Copying high resolution and horse to zebra AI models")
add_custom_command(TARGET AIRESTAPI 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory 
                    ${PROJECT_SOURCE_DIR}/src/AI_Image_Processing_Unit/ai_models
                    ${PROJECT_SOURCE_DIR}/src/AI_Image_Processing_Unit2/ai_models
                    $<TARGET_FILE_DIR:AIRESTAPI>/ai_models)

if (WIN32)
	message (STATUS "Copying pytorch DLLs to the binary directory")
	file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
	add_custom_command(TARGET AIRESTAPI
						POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy_if_different
						${TORCH_DLLS}
						$<TARGET_FILE_DIR:AIRESTAPI>)

	message (STATUS "Creating download directory")
	add_custom_command(TARGET AIRESTAPI 
                       POST_BUILD 
				       COMMAND ${CMAKE_COMMAND} -E make_directory 
				       ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${DOWNLOAD_DIR})
elseif()
	message (STATUS "Creating download directory")
	add_custom_command(TARGET AIRESTAPI 
                       POST_BUILD 
				       COMMAND ${CMAKE_COMMAND} -E make_directory 
				       ${CMAKE_BINARY_DIR}/bin/${DOWNLOAD_DIR})
endif()

