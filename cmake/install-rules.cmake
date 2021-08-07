if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_INSTALL_INCLUDEDIR include/gc-c-jlox CACHE PATH "")
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package gc-c-jlox)

install(
    TARGETS gc-c-jlox_gc-c-jlox
    EXPORT gc-c-jloxTargets
    RUNTIME COMPONENT gc-c-jlox_Runtime
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    gc-c-jlox_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(gc-c-jlox_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${gc-c-jlox_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT gc-c-jlox_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${gc-c-jlox_INSTALL_CMAKEDIR}"
    COMPONENT gc-c-jlox_Development
)

install(
    EXPORT gc-c-jloxTargets
    NAMESPACE gc-c-jlox::
    DESTINATION "${gc-c-jlox_INSTALL_CMAKEDIR}"
    COMPONENT gc-c-jlox_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
