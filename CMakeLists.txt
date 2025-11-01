cmake_minimum_required(VERSION 3.23)

project(SocketSphere)

set(BOOST_ENABLE_CMAKE ON)
set(CXX_)

include(FetchContent)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_PROGRESS TRUE
  GIT_TAG boost-1.89.0
)
FetchContent_MakeAvailable(Boost)

add_executable(socket_sphere_server
    src/server.cpp
)
target_include_directories(socket_sphere_server
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include/server
    ${PROJECT_SOURCE_DIR}/include/common
)
target_link_libraries(socket_sphere_server
    PRIVATE 
    Boost::asio
)
target_compile_features(socket_sphere_server PRIVATE cxx_std_20)

add_executable(socket_sphere_client
    src/client.cpp
)
target_include_directories(socket_sphere_client PRIVATE
    ${PROJECT_SOURCE_DIR}/include/client
    ${PROJECT_SOURCE_DIR}/include/common
)
target_link_libraries(socket_sphere_client
    PRIVATE
    Boost::asio
)
target_compile_features(socket_sphere_client PRIVATE cxx_std_20)
