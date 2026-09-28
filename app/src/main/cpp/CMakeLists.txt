cmake_minimum_required(VERSION 3.22.1)
project("aegis")

find_package(android.app.NativeActivity REQUIRED)

# Point CMake directly inside your uploaded folder structure
include_directories(
    ../../../../AEGIS_Whole_Game_Master/AEGIS_Whole_Game_Master/Source/Combat
    ../../../../AEGIS_Whole_Game_Master/AEGIS_Whole_Game_Master/Source/VR
    ../../../../AEGIS_Whole_Game_Master/AEGIS_Whole_Game_Master/Source/Monetization
    ../../../../AEGIS_Whole_Game_Master/AEGIS_Whole_Game_Master/Source/AI
)

add_library(aegis SHARED
    main.cpp
)

target_link_libraries(aegis
    android
    native_app_glue
    EGL
    GLESv3
    log
)
