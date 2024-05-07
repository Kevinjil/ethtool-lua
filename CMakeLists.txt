cmake_minimum_required(VERSION 3.26)
project(
  ethtool-lua
  VERSION 0.1.0
  LANGUAGES C
)

set(Lua_FIND_VERSION_EXACT true)
set(Lua_FIND_VERSION_COUNT 2) # Needs to be >1 for Lua_FIND_VERSION_EXACT
set(Lua_FIND_VERSION_MAJOR 5)
set(Lua_FIND_VERSION_MINOR 1)
include(FindLua)

if(LUA_FOUND)
  message(STATUS "Found Lua ${LUA_VERSION_STRING} at ${LUA_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "Lua 5.1 not found.")
endif()

set(ethtool_INCLUDE_DIRS ${ethtool_SOURCE_DIR} ${LUA_INCLUDE_DIR})
include_directories(${ethtool_INCLUDE_DIRS})

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(ethtool SHARED ethtool.c)
install(TARGETS ethtool DESTINATION /usr/lib/lua)
