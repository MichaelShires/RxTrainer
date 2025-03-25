cmake_minimum_required(VERSION 3.25)
project(FinalProjectLab)

set(CMAKE_CXX_STANDARD 20)

# Add executable and source files
add_executable(FinalProjectLab
        src/Main.cpp
        src/Core.cpp
        src/Core.h
        src/CoreElement.cpp
        src/CoreElement.h
        src/CoolantChunk.cpp
        src/CoolantChunk.h
        src/CoolantLoop.cpp
        src/CoolantLoop.h
        src/ProtectiveActionLogic.cpp
        src/ProtectiveActionLogic.h
        src/MainSimulation.cpp
        src/MainSimulation.h
        src/Visualization.cpp
        src/Visualization.h
        src/glad.c
        src/Constants.h
        src/main.cpp
)

# Include directories
target_include_directories(FinalProjectLab PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
)

# Set GLFW directory
if(APPLE)
    # For macOS
    set(glfw3_DIR "/opt/homebrew/lib/cmake/glfw3")
elseif(UNIX)
    # For Linux
    set(glfw3_DIR "/usr/lib/cmake/glfw3")
endif()

# Find OpenGL and GLFW
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED CONFIG)

# If you're using pthreads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Include GLM using FetchContent
include(FetchContent)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Include GLM headers
target_include_directories(FinalProjectLab PRIVATE ${glm_SOURCE_DIR})

# Collect libraries to link
set(LIBS
        OpenGL::GL
        glfw
        ${CMAKE_DL_LIBS}
        Threads::Threads
        glm::glm
)

# For macOS, link against the necessary frameworks
if(APPLE)
    list(APPEND LIBS
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework IOKit"
            "-framework CoreVideo"
    )
endif()

# Link libraries using the keyword signature
target_link_libraries(FinalProjectLab PUBLIC ${LIBS})