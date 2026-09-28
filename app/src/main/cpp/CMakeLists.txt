cmake_minimum_required(VERSION 3.22.1)
project("aegis")

# 1. Compile native_app_glue directly from the Android NDK
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

# 2. Include project C++ source directories
include_directories(
    ../../../../AEGIS_Whole_Game_Master/Source/Combat
    ../../../../AEGIS_Whole_Game_Master/Source/VR
    ../../../../AEGIS_Whole_Game_Master/Source/Monetization
    ../../../../AEGIS_Whole_Game_Master/Source/AI
)

# 3. Create your game shared library
add_library(aegis SHARED
    main.cpp
)

# 4. Link everything together
target_link_libraries(aegis
    native_app_glue
    android
    EGL
    GLESv3
    log
)
