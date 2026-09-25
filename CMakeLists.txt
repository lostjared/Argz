cmake_minimum_required(VERSION 3.16)
project(Argz VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(argz INTERFACE)
add_library(Argz::argz ALIAS argz)
target_compile_features(argz INTERFACE cxx_std_20)
target_include_directories(argz INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(FILES include/argz.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Argz)
install(TARGETS argz EXPORT ArgzTargets)
set(ARGZ_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Argz)
install(EXPORT ArgzTargets NAMESPACE Argz:: DESTINATION ${ARGZ_CMAKE_DIR})
configure_package_config_file(cmake/ArgzConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ArgzConfig.cmake
    INSTALL_DESTINATION ${ARGZ_CMAKE_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ArgzConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ArgzConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ArgzConfigVersion.cmake DESTINATION ${ARGZ_CMAKE_DIR})

include(CTest)
if(BUILD_TESTING)
    add_executable(argz_tests test.cpp)
    target_link_libraries(argz_tests PRIVATE Argz::argz)
    add_test(NAME argz_tests COMMAND argz_tests)
endif()
