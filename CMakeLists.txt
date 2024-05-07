cmake_minimum_required(VERSION 3.26)
project(
  ethtool-lua
  VERSION 0.1.0
  LANGUAGES C
)

find_library(LIB_LUA lua /usr/lib REQUIRED)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(ethtool SHARED ethtool.c)
install(TARGETS ethtool DESTINATION /usr/lib/lua)
