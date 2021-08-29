cmake_minimum_required(VERSION 3.14)

include(cmake/in-source-guard.cmake)

project(
    gc-c-jlox
    VERSION 0.1.0
    DESCRIPTION "JLox in C"
    HOMEPAGE_URL "https://github.com/Phytolizer/gc-c-jlox"
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

set(FOLDER_gc "gc-8.0.4")
add_subdirectory("${FOLDER_gc}")

# ---- Declare library ----

add_library(
    gc-c-jlox_lib OBJECT
    source/lib.c
    source/private/interpreter.c
    source/private/environment.c
    source/private/list.c
    source/private/object.c
    source/private/parser.c
    source/private/runtime_error.c
    source/private/scanner.c
    source/private/strutils.c
    source/private/token.c
    source/private/token_type.c
    source/private/ast/debug.c
    source/private/ast/expr.c
    source/private/ast/printer.c
    source/private/ast/stmt.c
    source/private/hash/fnv.c
    source/private/hash/table.c
)

target_include_directories(
    gc-c-jlox_lib
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_link_libraries(gc-c-jlox_lib PUBLIC gc-lib)
target_include_directories(gc-c-jlox_lib PUBLIC "${FOLDER_gc}/include")
target_compile_features(gc-c-jlox_lib PUBLIC c_std_11)

# ---- Declare executable ----

add_executable(gc-c-jlox_gc-c-jlox source/main.c)
add_executable(gc-c-jlox::gc-c-jlox ALIAS gc-c-jlox_gc-c-jlox)

set_target_properties(
    gc-c-jlox_gc-c-jlox PROPERTIES
    OUTPUT_NAME gc-c-jlox
    EXPORT_NAME gc-c-jlox
)

target_compile_features(gc-c-jlox_gc-c-jlox PRIVATE c_std_11)

target_link_libraries(gc-c-jlox_gc-c-jlox PRIVATE gc-c-jlox_lib)

# ---- Install rules ----

include(cmake/install-rules.cmake)

# ---- Developer mode ----

if(NOT gc-c-jlox_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of gc-c-jlox"
  )
endif()

include(cmake/dev-mode.cmake)
