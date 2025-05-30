cmake_minimum_required(VERSION 3.15)
project(Terrain VERSION 1.0.0)

# Set C++ standard
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add DREngine dependency
add_subdirectory(dependencies/DREngine)

# Source files
file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp", "src/*.h")
file(GLOB GPU CONFIGURE_DEPENDS "src/GPU/*.cpp", "src/GPU/*.h")
file(GLOB TERRAIN CONFIGURE_DEPENDS "src/Terrain/*.cpp", "src/Terrain/*.h")

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd")
  source_group("Source Files" FILES ${SOURCES})
  source_group("Source Files/GPU" FILES ${GPU})
  source_group("Source Files/Terrain" FILES ${TERRAIN})
endif()

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${GPU} ${TERRAIN})


# Link with DREngine
target_link_libraries(${PROJECT_NAME} PRIVATE DREngine glu32.lib)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/DREngine/include
)

# Copy DLL files to output directory
file(GLOB DLL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.dll")
foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL_FILE}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
