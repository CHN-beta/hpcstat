cmake_minimum_required(VERSION 3.14)
project(hpcstat VERSION 0.0.0 LANGUAGES CXX)
enable_testing()
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY CXX_STANDARD 23)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS headers filesystem)
find_package(zxorm REQUIRED)

add_executable(hpcstat src/main.cpp)
target_link_libraries(hpcstat PRIVATE fmt::fmt Boost::headers Boost::filesystem zxorm::zxorm)

install(TARGETS hpcstat RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

get_property(ImportedTargets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message("Imported targets: ${ImportedTargets}")
message("List of compile features: ${CMAKE_CXX_COMPILE_FEATURES}")