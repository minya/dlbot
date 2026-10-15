cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(dlbot)

include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra -Werror")
set(BUILD_SHARED_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED program_options json)
find_package(CURL)
include_directories(${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()


ExternalProject_Add(tgbot-cpp
    GIT_REPOSITORY https://github.com/reo7sp/tgbot-cpp
    GIT_TAG bff3045898dfb4a8af624e58552dab9cd3087c1f # v1.13+1
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} -DOPENSSL_LIBRARIES=${OPENSSL_LIBRARIES}
)
ExternalProject_Add(sentry-native
    GIT_REPOSITORY https://github.com/getsentry/sentry-native.git
    GIT_TAG d04453d6d3c4fbde9015b591e64df474e2164781 # 0.17.3+9
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DSENTRY_BACKEND=crashpad
)

include_directories(SYSTEM ${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_executable(dlbot
    main.cpp
    path_util.cpp
    dlbot.cpp
    transmission_rpc.cpp
    sentry_logger.cpp
    http/http_client.cpp 
    http/http_request.cpp
    http/http_response.cpp
    http/uri.cpp)
add_dependencies(dlbot tgbot-cpp sentry-native)
target_link_libraries(dlbot
    libTgBot.a
    Boost::program_options
    Boost::json
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    sentry
)
