# CMakeList.txt : CMake project for Renderer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Renderer")

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

add_compile_definitions(PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}/")

# Add source to this project's executable.
file(GLOB_RECURSE SRC_FILES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/lib/*.cpp"
    "${PROJECT_SOURCE_DIR}/lib/*.h"
    "${PROJECT_SOURCE_DIR}/shaders/*.glsl"
)

add_executable(Renderer ${SRC_FILES})

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(SDL3_ttf REQUIRED)

#find_package(flecs CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(freetype REQUIRED)
find_package(yaml-cpp REQUIRED)

find_package(nlohmann_json REQUIRED)
find_package(magic_enum REQUIRED)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Renderer PROPERTY CXX_STANDARD 20)
endif()

add_custom_command(TARGET Renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:Renderer>/shaders

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Renderer>/assets

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/logs
    $<TARGET_FILE_DIR:Renderer>/logs
)

target_include_directories(Renderer PRIVATE
    ${PROJECT_SOURCE_DIR}   
    ${PROJECT_SOURCE_DIR}/src     
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/core/fsm
    ${PROJECT_SOURCE_DIR}/src/core/style
    ${PROJECT_SOURCE_DIR}/src/generation
    ${PROJECT_SOURCE_DIR}/src/gameplay
    ${PROJECT_SOURCE_DIR}/src/graphics/rendering
    ${PROJECT_SOURCE_DIR}/src/graphics/opengl
    
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/lib/glm
    ${PROJECT_SOURCE_DIR}/lib/imgui
    
    ${PROJECT_SOURCE_DIR}/includes
    ${PROJECT_SOURCE_DIR}/includes/core
    ${PROJECT_SOURCE_DIR}/includes/gameplay
    ${PROJECT_SOURCE_DIR}/includes/graphics/rendering
    ${PROJECT_SOURCE_DIR}/includes/graphics/opengl
)

target_link_libraries(Renderer PRIVATE 
	SDL3::SDL3
	SDL3_image::SDL3_image
	SDL3_mixer::SDL3_mixer
	SDL3_ttf::SDL3_ttf

	#flecs::flecs
	imgui::imgui
	glad::glad
	Freetype::Freetype
    yaml-cpp::yaml-cpp

    nlohmann_json::nlohmann_json
    magic_enum::magic_enum
)