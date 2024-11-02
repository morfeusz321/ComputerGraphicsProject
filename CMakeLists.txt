cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(ComputerGraphics C CXX)

# Set this before including framework such that it knows to use the OpenGL4.5 version of GLAD
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
    add_subdirectory("framework")
elseif(PROJECT_IS_TOP_LEVEL)
    add_subdirectory("../../../framework/" "${CMAKE_CURRENT_BINARY_DIR}/framework/")
endif()

# Define source files
set(SOURCES
    src/application.cpp
    src/cubemap/cubemap.cpp
    src/skybox/skybox.cpp
    src/texture.cpp
    src/mesh.cpp
    src/camera.cpp
)

# Define header files
set(HEADERS
    src/cubemap/cubemap.h
    src/skybox/skybox.h
    src/texture.h
    src/mesh.h
    src/camera.h
)

add_executable(Master_TechDemo ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(Master_TechDemo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cubemap
    ${CMAKE_CURRENT_SOURCE_DIR}/src/skybox
)

# Set compile definitions and features
target_compile_definitions(Master_TechDemo PRIVATE 
    RESOURCE_ROOT="${CMAKE_CURRENT_LIST_DIR}/"
)
target_compile_features(Master_TechDemo PRIVATE cxx_std_20)

# Link libraries
target_link_libraries(Master_TechDemo PRIVATE CGFramework)

# Enable sanitizers and warnings
enable_sanitizers(Master_TechDemo)
set_project_warnings(Master_TechDemo)

# Resource copying
add_custom_command(TARGET Master_TechDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_LIST_DIR}/resources/" "$<TARGET_FILE_DIR:Master_TechDemo>/resources/"
)

# Shader handling
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders/")
set(Master_TechDemo_shader_copies "")

# List all *.glsl files in the shaders folder
file(GLOB Master_TechDemo_shader_sources "${CMAKE_CURRENT_LIST_DIR}/shaders/*.glsl")
foreach(shader_file IN LISTS Master_TechDemo_shader_sources)
    get_filename_component(file_name ${shader_file} NAME)
    message("shader_file: ${file_name}")
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/${file_name}"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_LIST_DIR}/shaders/${file_name}"
        "${CMAKE_CURRENT_BINARY_DIR}/shaders/${file_name}"
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/shaders/${file_name}"
    )
    list(APPEND Master_TechDemo_shader_copies "${CMAKE_CURRENT_BINARY_DIR}/shaders/${file_name}")
endforeach()

add_custom_target(Master_TechDemo_copy_shaders DEPENDS ${Master_TechDemo_shader_copies})
add_dependencies(Master_TechDemo Master_TechDemo_copy_shaders)