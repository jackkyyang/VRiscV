cmake_minimum_required(VERSION 3.5)

project(VRiscV VERSION 0.1.0 LANGUAGES C)

# 使用pkg-config
find_package(PkgConfig REQUIRED)
# 指定多线程库
find_package(Threads)

MESSAGE(STATUS "Project Name: " ${PROJECT_NAME})
add_executable(${PROJECT_NAME})

file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

#通过执行pkg-config程序，并指定需要的模块是gtk+-3.0
pkg_check_modules(GTK REQUIRED gtk+-3.0)

# # 打印GTK3的路径
# MESSAGE(STATUS "------ Found GTK3 INCLUDE_DIRS ------:\n" ${GTK_INCLUDE_DIRS})
# MESSAGE(STATUS "------ Found GTK3 Lib Dirs ------:\n" ${GTK_LIBRARIES})
# MESSAGE(STATUS "------ Found GTK3 CFLAGS ------:\n" ${GTK_CFLAGS})
# MESSAGE(STATUS "------ Found GTK3 Other CFLAGS ------:\n" ${GTK_CFLAGS_OTHER})
# MESSAGE(STATUS "------ Found GTK3 linker flags ------:\n" ${GTK_LDFLAGS})
# MESSAGE(STATUS "------ Found GTK3 all other linker flags ------:\n" ${GTK_LDFLAGS_OTHER})
# MESSAGE(STATUS "------ Found GTK3 LIBRARY_DIRS ------:\n" ${GTK_LIBRARY_DIRS})

if(GTK_FOUND)
  MESSAGE(STATUS "Found GTK")
  add_definitions(${GTK_CFLAGS_OTHER})
else()
  MESSAGE(FATAL_ERROR "CMake Cannot find the GTK, stop and exit!\n")
endif()

link_directories(${GTK_LIBRARY_DIRS})

# 指定头文件路径
target_include_directories(${PROJECT_NAME} PUBLIC ${GTK_INCLUDE_DIRS})

#指定连接库名称
target_link_libraries(${PROJECT_NAME} ${GTK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# 指定源文件
target_sources(${PROJECT_NAME} PUBLIC ${sources})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})

set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

configure_file(
  config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/src/include/config.h
  @ONLY
  )


include(CPack)
