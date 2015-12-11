include(FindPkgConfig)
pkg_check_modules(NANOMSG REQUIRED "libnanomsg")
set(NANOMSG_INCLUDE_DIRS "/usr/include") # temp

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NANOMSG DEFAULT_MSG NANOMSG_LIBRARIES NANOMSG_INCLUDE_DIRS)
mark_as_advanced(NANOMSG_LIBRARIES NANOMSG_INCLUDE_DIRS)

