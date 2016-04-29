externalproject_add(JVAR
    GIT_TAG a41b76b339ab4b925b3a3d28aa0e91e310bc3453
    GIT_REPOSITORY https://github.com/YasserAsmi/jvar.git
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/jvar
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cmake ${CMAKE_SOURCE_DIR}/third_party/jvar
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
)
set(JVAR_INCLUDES ${CMAKE_SOURCE_DIR}/third_party/jvar/include)

mark_as_advanced(
    JVAR
    JVAR_INCLUDES
)