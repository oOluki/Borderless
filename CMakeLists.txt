cmake_minimum_required(VERSION 3.10.0)
project(Borderless VERSION 0.0.11 LANGUAGES C)


# Set C/C++ standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 99)


# Add executable
file(GLOB SRC_FILES "src/*.c")
add_executable(${PROJECT_NAME} ${SRC_FILES})
add_executable(map_designer map_designer.c)

if(NOT WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

if(NOT DONT_SUPPORT_SDL)
  find_package(SDL2 QUIET)
  if(NOT SDL2_FOUND)
    message("SDL2 not found, graphics support won't be included")
  endif()
endif()

if(SDL2_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DSUPPORT_SDL=1)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DDONT_SUPPORT_SDL=1)
endif()

include(CTest)

add_test(general_test ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} -nf --input ${CMAKE_CURRENT_SOURCE_DIR}/input.txt)

enable_testing()

