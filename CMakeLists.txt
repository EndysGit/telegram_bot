cmake_minimum_required(VERSION 3.16)

project(telegram_bot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SOURCE_DIR /media/endyxx/data_hub/Development/cpp_projects/telegram_bot_repo)

if (MSVC)
    add_compile_options(\W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.71.0 REQUIRED COMPONENTS system regex thread date_time)

find_package(RapidJSON)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
