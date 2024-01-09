cmake_minimum_required(VERSION 3.5)

project(VRiscV VERSION 0.1.0 LANGUAGES C)

add_executable(VRiscV)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

target_sources(VRiscV PUBLIC ${sources})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})

set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

configure_file(
  config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/src/include/config.h
  @ONLY
  )


include(CPack)
