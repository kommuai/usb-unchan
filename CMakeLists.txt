cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

project("unchan")
include(FindPkgConfig)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

find_package(FLTK REQUIRED)
pkg_check_modules(USB REQUIRED libusb-1.0)

include_directories(${PROJECT_SOURCE_DIR}
                    ${USB_INCLUDE_DIRS}
                    )

add_executable(unchan unchan.cpp)

target_link_libraries(unchan fltk
                             ${USB_LIBRARIES}
                             )

