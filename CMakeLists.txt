cmake_minimum_required(VERSION 3.5)
project(VRiscV VERSION 0.1.0 LANGUAGES C)

# include(CTest)
# enable_testing()

add_executable(VRiscV)
file(GLOB_RECURSE sources CONFIGURE_DEPENDS ./src/*.c ./src/*.h)
target_sources(VRiscV PUBLIC ${sources})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
