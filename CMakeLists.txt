cmake_minimum_required(VERSION 3.10)
project(Hyperion-Core)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add Source Files
set(SOURCES 
    src/main.cpp
    src/physics.cpp
    src/network.cpp
)

# Create Executable
add_executable(hyperion ${SOURCES})

# Link Windows Socket Library
if(WIN32)
    target_link_libraries(hyperion ws2_32)
endif()

# --- OPTIMIZATION FIX ---
if(MSVC)
    # Apply AVX2 to all builds
    target_compile_options(hyperion PRIVATE /arch:AVX2)
    
    # ONLY apply /O2 (Speed) if we are in Release mode
    target_compile_options(hyperion PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(hyperion PRIVATE -O3 -march=native -pthread)
endif()