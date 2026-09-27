cmake_minimum_required(VERSION 3.22.1)
project("aegis")

# Load Android's native app infrastructure
find_package(android.app.NativeActivity REQUIRED)

# Link your existing AEGIS Source directories
include_directories(
    ../../../../Source/Combat
    ../../../../Source/VR
    ../../../../Source/Monetization
    ../../../../Source/AI
)

# Compile into a shared Android library (.so)
add_library(aegis SHARED
    main.cpp
    # Note: As you expand, list your other .cpp files from the Source directory here
)

# Connect Android system libraries for rendering and logging
target_link_libraries(aegis
    android
    native_app_glue
    EGL
    GLESv3
    log
)
