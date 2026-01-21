cmake_minimum_required(VERSION 3.25)
project(mc_cpp)
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 23)
set(BOTCRAFT_BUILD_DOC OFF)
set(BOTCRAFT_GAME_VERSION "1.21.4" CACHE STRING "Minecraft game version")

file(GLOB_RECURSE LIBS "include/*.hpp" "include/*.h" "include/*.cpp" "include/*.c")
include_directories(mc_cpp ${CMAKE_CURRENT_SOURCE_DIR}/include)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS iostreams)
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
        PUBLIC result_cpp
        PUBLIC "${Boost_IOSTREAMS_LIBRARY}"
)

add_executable(mc_cpp_run main.cpp)
target_link_libraries(mc_cpp_run
        PUBLIC mc_cpp
)