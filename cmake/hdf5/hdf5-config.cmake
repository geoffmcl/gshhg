#-----------------------------------------------------------------------------
# HDF5 Config file for compiling against hdf5 install directory
#-----------------------------------------------------------------------------
GET_FILENAME_COMPONENT (SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${SELF_DIR}" PATH)
GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if (NOT WIN32)
  GET_FILENAME_COMPONENT(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
endif (NOT WIN32)

#-----------------------------------------------------------------------------
# User Options
#-----------------------------------------------------------------------------
set (HDF5_ENABLE_PARALLEL OFF)
set (HDF5_BUILD_FORTRAN   OFF)
set (HDF5_ENABLE_F2003    )
set (HDF5_BUILD_CPP_LIB   OFF)
set (HDF5_BUILD_TOOLS     OFF)
set (HDF5_BUILD_HL_LIB    ON)
set (HDF5_ENABLE_Z_LIB_SUPPORT OFF)
set (HDF5_ENABLE_SZIP_SUPPORT  OFF)
set (HDF5_ENABLE_SZIP_ENCODING )
set (HDF5_BUILD_SHARED_LIBS    OFF)
set (HDF5_PACKAGE_EXTLIBS      OFF)

#-----------------------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------------------
IF(HDF5_ENABLE_PARALLEL)
  SET(HDF5_MPI_C_INCLUDE_PATH "")
  SET(HDF5_MPI_C_LIBRARIES    "")
ENDIF(HDF5_ENABLE_PARALLEL)

#-----------------------------------------------------------------------------
# Directories
#-----------------------------------------------------------------------------
set (HDF5_INCLUDE_DIR "${_IMPORT_PREFIX}/include" "${HDF5_MPI_C_INCLUDE_PATH}" )

if (HDF5_BUILD_FORTRAN)
  set (HDF5_INCLUDE_DIR_FORTRAN "${_IMPORT_PREFIX}/include" )
endif (HDF5_BUILD_FORTRAN)
  
if (HDF5_BUILD_CPP_LIB)
  set (HDF5_INCLUDE_DIR_CPP "${_IMPORT_PREFIX}/include" )
endif (HDF5_BUILD_CPP_LIB)

if (HDF5_BUILD_HL_LIB)
  set (HDF5_INCLUDE_DIR_HL "${_IMPORT_PREFIX}/include" )
endif (HDF5_BUILD_HL_LIB)

if (HDF5_BUILD_HL_LIB AND HDF5_BUILD_CPP_LIB)
  set (HDF5_INCLUDE_DIR_HL_CPP "${_IMPORT_PREFIX}/include" )
endif (HDF5_BUILD_HL_LIB AND HDF5_BUILD_CPP_LIB)

if (HDF5_BUILD_TOOLS)
  set (HDF5_INCLUDE_DIR_TOOLS "${_IMPORT_PREFIX}/include" )
  set (HDF5_TOOLS_DIR "${_IMPORT_PREFIX}/bin" )
endif (HDF5_BUILD_TOOLS)

#-----------------------------------------------------------------------------
# Version Strings
#-----------------------------------------------------------------------------
set (HDF5_VERSION_STRING 1.8.14)
set (HDF5_VERSION_MAJOR  1.8)
set (HDF5_VERSION_MINOR  14)

#-----------------------------------------------------------------------------
# Don't include targets if this file is being picked up by another
# project which has already built hdf5 as a subproject
#-----------------------------------------------------------------------------
if (NOT TARGET "hdf5")
  if (HDF5_ENABLE_Z_LIB_SUPPORT AND HDF5_PACKAGE_EXTLIBS AND NOT TARGET "zlib")
    include (${SELF_DIR}/../ZLIB/-targets.cmake)
  endif (HDF5_ENABLE_Z_LIB_SUPPORT AND HDF5_PACKAGE_EXTLIBS AND NOT TARGET "zlib")
  if (HDF5_ENABLE_SZIP_SUPPORT AND HDF5_PACKAGE_EXTLIBS AND NOT TARGET "szip")
    include (${SELF_DIR}/../SZIP/-targets.cmake)
  endif (HDF5_ENABLE_SZIP_SUPPORT AND HDF5_PACKAGE_EXTLIBS AND NOT TARGET "szip")
  include (${SELF_DIR}/hdf5-targets.cmake)
  set (HDF5_LIBRARIES "hdf5;hdf5_hl")
endif (NOT TARGET "hdf5")

