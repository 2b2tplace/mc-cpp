cmake_minimum_required(VERSION 4.0)
project(mc_cpp)

set(CMAKE_CXX_STANDARD 23)
set(BOTCRAFT_BUILD_DOC OFF)

file(GLOB_RECURSE LIBS "include/*.hpp" "include/*.h" "include/*.cpp" "include/*.c")
include_directories(mc_cpp ${CMAKE_CURRENT_SOURCE_DIR}/include)
find_package(fmt REQUIRED)
link_libraries(z sqlite3 crypto)
if(WIN32)
    link_libraries(ws2_32)
endif()

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

FetchContent_Declare(Botcraft
        GIT_REPOSITORY https://github.com/adepierre/Botcraft
        GIT_TAG master
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Botcraft)

FetchContent_Declare(abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG        master
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(abseil)

set(DEPLOY_KEY_PATH "~/.ssh/result_repo")
set(ENV{GIT_SSH_COMMAND} "ssh -i ${DEPLOY_KEY_PATH}")
FetchContent_Declare(result
        GIT_REPOSITORY git@github.com:2b2tplace/result-cpp.git
        GIT_TAG        main
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(result)

add_library(mc_cpp ${LIBS})
target_compile_definitions(mc_cpp PUBLIC $<$<CONFIG:Debug>:DEBUG_MODE> BOTCRAFT_GAME_VERSION="${BOTCRAFT_GAME_VERSION}")
target_include_directories(mc_cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mc_cpp
        PUBLIC absl::flat_hash_map
        PUBLIC protocolCraft
        PUBLIC fmt
)

add_executable(mc_cpp_run main.cpp)
target_link_libraries(mc_cpp_run
        PUBLIC mc_cpp
)