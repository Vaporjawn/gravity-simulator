cmake_minimum_required(VERSION 3.21)
project(GravitySimulator LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

add_executable(GravitySimulator
    src/main.cpp
    src/CelestialBody.cpp
    src/SolarSystem.cpp
    src/Physics.cpp
    src/Renderer.cpp
    src/InputHandler.cpp
)

target_link_libraries(GravitySimulator PRIVATE sfml-graphics sfml-window sfml-system)
target_compile_features(GravitySimulator PRIVATE cxx_std_17)

if(WIN32)
    add_custom_command(
        TARGET GravitySimulator
        COMMENT "Deploy Qt libraries"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<TARGET_FILE_DIR:sfml-graphics>/../bin
            $<TARGET_FILE_DIR:GravitySimulator>
        VERBATIM)
endif()

# Install rules
install(TARGETS GravitySimulator)
