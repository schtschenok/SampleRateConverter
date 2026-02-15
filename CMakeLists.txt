# LLM-generated slop

cmake_minimum_required(VERSION 3.14)
project(wav_resampler C)

set(CMAKE_C_STANDARD 99)

# --- Fetch libsndfile from Git ---
include(FetchContent)

# Disable external dependencies to speed up build
set(ENABLE_MPEG OFF CACHE BOOL "Disable MPEG" FORCE)
set(ENABLE_BOW_DOCS OFF CACHE BOOL "Disable docs" FORCE)
set(BUILD_PROGRAMS OFF CACHE BOOL "Disable tools" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Disable examples" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable tests" FORCE)
set(ENABLE_EXTERNAL_LIBS OFF CACHE BOOL "Disable external codec libs" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)

FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(libsndfile)

# --- Build the Resampler ---

add_executable(wav_resampler 
    main.c 
    src.c 
    src.h
)

# FIX: Link ONLY against sndfile. Removed 'm'.
target_link_libraries(wav_resampler 
    PRIVATE 
    sndfile
)

# If you ever move this code to Linux/Mac, you can uncomment this:
# if(UNIX)
#     target_link_libraries(wav_resampler PRIVATE m)
# endif()

target_include_directories(wav_resampler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})