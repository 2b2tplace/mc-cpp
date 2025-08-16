cmake_minimum_required(VERSION 4.0)
project(mc_cpp)

set(CMAKE_CXX_STANDARD 23)

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

add_library(mc_cpp ${LIBS})
target_compile_definitions(mc_cpp PUBLIC $<$<CONFIG:Debug>:DEBUG_MODE> BOTCRAFT_GAME_VERSION="${BOTCRAFT_GAME_VERSION}")
target_include_directories(mc_cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mc_cpp
        PUBLIC protocolCraft
        PUBLIC fmt
)