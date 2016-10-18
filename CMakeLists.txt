# @0140929 - GSHHG Project - http://www.soest.hawaii.edu/pwessel/gshhg/
# CMakeLists.txt, generated gencmake.pl, on 2014/09/28 18:01:17
cmake_minimum_required( VERSION 2.8.8 )

include (TestBigEndian)

# We have some custom .cmake scripts not in the official distribution.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project( gshhg )

# The version number.
set( gshhg_MAJOR 3 )
set( gshhg_MINOR 0 )
set( gshhg_POINT 0 )

# Allow developer to select is Dynamic or static library built
set( LIB_TYPE STATIC )  # set default static
option( BUILD_SHARED_LIB "Build Shared Library" OFF )
# EXPERIMENTAL - Turn ON to combine library into EXE - above shared library MUST be OFF
option( BUILD_AS_ONE "Build as one. Combine library into EXE" OFF )
option( BUILD_GMT2IMG "Set ON to build gmt2img example."      OFF )
option( BUILD_HDFDATA "Set ON to build hdfdata example."      OFF )

if(CMAKE_COMPILER_IS_GNUCXX)
    set( WARNING_FLAGS -Wall )
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
endif() 

if(WIN32 AND MSVC)
    # turn off various warnings - none needed in this compile
    set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
    # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
    #     set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
    # endforeach(warning)
    set( MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS" )
    # if (${MSVC_VERSION} EQUAL 1600)
    #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
    # endif (${MSVC_VERSION} EQUAL 1600)
    #set( NOMINMAX 1 )
    #if(MINGW)
        # need to specifically handle rc files, like
        # resource compilation for mingw
        #ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o
        #                   COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
        #                                       -i${CMAKE_CURRENT_SOURCE_DIR}/test.rc
        #                                       -o ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o)
        #SET(test_EXE_SRCS ${test_EXE_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o)
    #else(MINGW)
        #SET(test_EXE_SRCS ${test_EXE_SRCS} test.rc)
    #endif(MINGW)
    # to distinguish between debug and release lib
    set( CMAKE_DEBUG_POSTFIX "d" )
else()
#   unix things
    set( WARNING_FLAGS "${WARNING_FLAGS} -Wno-write-strings" )
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )

test_big_endian (WORDS_BIGENDIAN)
if (WORDS_BIGENDIAN)
    add_definitions( -DWORDS_BIGENDIAN )
    message(STATUS "*** Running on a BIGENDIAN CPU")
else ()
    message(STATUS "*** Running on a LITTLEENDIAN CPU")
endif ()    

# PNG_FOUND If false, do not try to use PNG.
# PNG_INCLUDE_DIRS  where to find png.h, etc.
# PNG_LIBRARIES the libraries to link against to use PNG.
# PNG_DEFINITIONS You should add_definitons(${PNG_DEFINITIONS}) before compiling code
#   that includes png library files.
# PNG_VERSION_STRING the version of the PNG library found (since CMake 2.8.8)
# PNG_LIBRARY where to find the PNG library.
#find_package(PNG)
find_package(PNG2)
if (PNG_FOUND)
    message(STATUS "*** Found PNG (and ZLIB) inc ${PNG_INCLUDE_DIRS} lib ${PNG_LIBRARIES}")
    include_directories(${PNG_INCLUDE_DIRS})
    list(APPEND add_LIBS ${PNG_LIBRARIES})
    message(STATUS "*** Version ${PNG_VERSION_STRING} defs ${PNG_DEFINITIONS}")
    add_definitions( -DUSE_PNG_LIB )
else ()
    message(STATUS "*** PNG (or ZLIB) NOT found!")
endif ()

# add_definitions( -DHAVE_CONFIG_H )

if(BUILD_SHARED_LIB)
   set(LIB_TYPE SHARED)
   message(STATUS "*** Building DLL library ${LIB_TYPE}")
else(BUILD_SHARED_LIB)
   message(STATUS "*** Option BUILD_SHARED_LIB is OFF ${LIB_TYPE}")
endif(BUILD_SHARED_LIB)

set(name utils)
set(dir src/utils)
include_directories( ${dir} )
set(${name}_SRCS
    ${dir}/sprtf.cxx
    ${dir}/utils.cxx
    )
set(${name}_HDRS
    ${dir}/sprtf.hxx
    ${dir}/utils.hxx
    )
add_library( ${name} ${${name}_SRCS} ${${name}_HDRS} )
list(APPEND add_LIBS ${name})    
# no install of this static utility library

################################################################
# BMP utility library
set(name bmp_utils)
set(dir src/bmp)
include_directories( ${dir} )
set(${name}_SRCS
    ${dir}/readbmp.c
    ${dir}/endianness.c
    )
set(${name}_HDRS
    ${dir}/bmp_utils.hxx
    ${dir}/bmp.h
    ${dir}/bmptypes.h
    ${dir}/endianness.h
    ${dir}/readbmp.h
    )
if (WIN32)
    list(APPEND ${name}_SRCS ${dir}/winBMP.cxx)
endif ()
add_library( ${name} ${${name}_SRCS} ${${name}_HDRS} )
list(APPEND add_LIBS ${name} )
# no install of this library

################################################################
if (PNG_FOUND)
    set(name png_utils)
    set(dir src/png)
    include_directories( ${dir} )
    set(${name}_SRCS
        ${dir}/${name}.cxx
        )
    set(${name}_HDRS
        ${dir}/${name}.hxx
        )
    add_library( ${name} ${${name}_SRCS} ${${name}_HDRS} )
    list(APPEND add_LIBS ${name} )
    # no install of this library
endif ()

################################################################
# BMP test - read a BMP file, and output ASCII information
set(name bmp-test)
set(dir src/bmp)
set(${name}_SRCS
    ${dir}/bmp-test.c
    )
set(${name}_HDRS
    ${dir}/bmp-config.h
    )
add_executable(${name} ${${name}_SRCS} ${${name}_HDRS})
if(add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if(WIN32)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif()
# deal with install 
# install(TARGETS ${name} DESTINATION bin )

set(name Int2BMP)
set(dir src/bmp)
set(${name}_SRCS
    ${dir}/Int2BMP.cpp
    )
add_executable(${name} ${${name}_SRCS})
if(add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if(WIN32)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
else()
    set_target_properties( ${name} PROPERTIES COMPILE_FLAGS "-std=c++11")
endif()
# deal with install 
# install(TARGETS ${name} DESTINATION bin )

# =========================================================
# from : http://paulbourke.net/dataformats/bmp/
# Contribution by Adam Majewski that writes a one bit per pixel BMP file: pf1bit_bmp.c.
set(name bmp-1bit)
set(dir src/bmp)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable(${name} ${${name}_SRCS} )
if(add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if(WIN32)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif()
# deal with install 
# install(TARGETS ${name} DESTINATION bin )
# =========================================================

set(name bmp_io)
set(dir src/bmp)
set(${name}_SRCS
    ${dir}/${name}.cpp
    )
set(${name}_HDRS
    ${dir}/${name}.hpp
    )
add_executable(${name} ${${name}_SRCS} ${${name}_HDRS})
if(add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if(WIN32)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif()
# deal with install 
# install(TARGETS ${name} DESTINATION bin )

set(name write-bmp1)
set(dir src/bmp)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable(${name} ${${name}_SRCS})
if(add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if(WIN32)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif()
# deal with install 
# install(TARGETS ${name} DESTINATION bin )

set(name write-bmp2)
set(dir src/bmp)
set(${name}_SRCS
    ${dir}/${name}.cxx
    )
add_executable(${name} ${${name}_SRCS})
if(add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if(WIN32)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif()
# deal with install 
# install(TARGETS ${name} DESTINATION bin )

###################################################################
### TGA
set(name tga-test)
set(dir src/tga)
set(${name}_SRCS
    ${dir}/tga-test.cpp
    ${dir}/tgaImage.cpp
    )
set(${name}_HDRS
    ${dir}/tgaImage.h
    )
add_executable(${name} ${${name}_SRCS} ${${name}_HDRS})
if(add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if(WIN32)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif()
# deal with install 
# install(TARGETS ${name} DESTINATION bin )

##################################################################
### PPM Potable PixMap - Portable GrayMap (PGM) - Portable BitMap (PBM)
set(name pgm-test)
set(dir src/pgm)
set(${name}_SRCS
    ${dir}/pgm-test.cpp
    ${dir}/pgmimage.cpp
    )
set(${name}_HDRS
    ${dir}/pgmimage.h
    )
add_executable(${name} ${${name}_SRCS} ${${name}_HDRS})
if(add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if(WIN32)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif()
# deal with install 
# install(TARGETS ${name} DESTINATION bin )

if (BUILD_GMT2IMG OR BUILD_HDFDATA)
    #   HDF5_FOUND - true if HDF5 was found on the system
    #   HDF5_INCLUDE_DIRS - Location of the hdf5 includes
    #   HDF5_DEFINITIONS - Required compiler definitions for HDF5
    #   HDF5_LIBRARIES - Required libraries for all requested bindings
    #   HDF5_LIBRARY_DIRS - the full set of library directories
    #   HDF5_C_LIBRARIES - Required libraries for the HDF5 C bindings.
    #   HDF5_CXX_LIBRARIES - Required libraries for the HDF5 C++ bindings
    #   HDF5_HL_LIBRARIES - Required libraries for the HDF5 high level API
    find_package(HDF5 COMPONENTS C CXX HL TOOLS)
    #find_package(HDF5)
endif ()

if (HDF5_FOUND)
    message(STATUS "*** Found HDF5 libs ${HDF5_LIBRARIES} inc ${HDF5_INCLUDE_DIR}")
    include_directories( ${HDF5_INCLUDE_DIR} )
    list(APPEND add_LIBS ${HDF5_LIBRARIES})
    if (HDF5_DEFINITIONS)
        message(STATUS "*** Check HDF5 definitiona ${HDF5_DEFINITIONS}")
    endif ()
    if (BUILD_HDFDATA)
        message(STATUS "*** Build HDFDATA is ON")
        set(name hdfData)
        set(dir src/gmt)
        set(${name}_SRCS
            ${dir}/${name}.cxx
            )
        set(${name}_HDRS
            ${dir}/${name}.hxx
            )
        add_executable(${name} ${${name}_SRCS} ${${name}_HDRS})
        if(add_LIBS)
            target_link_libraries( ${name} ${add_LIBS} )
        endif ()
        if(MSVC)
            set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
        endif()
        # deal with install 
        # install(TARGETS ${name} DESTINATION bin )
    else ()
        message(STATUS "*** Build HDFDATA is OFF")
    endif ()
else ()
    message(STATUS "*** HDF5 NOT found")
endif ()

#################################################
if (PNG_FOUND AND HDF5_FOUND AND BUILD_GMT2IMG)
    ### GMT ASCII to image
    set(name gmt2img)
    set(dir src/gmt)
    set(${name}_SRCS
        ${dir}/${name}.cxx
        )
    add_executable(${name} ${${name}_SRCS})
    if(add_LIBS)
        target_link_libraries( ${name} ${add_LIBS} )
    endif ()
    if(WIN32)
        set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
    endif()
    # deal with install 
    # install(TARGETS ${name} DESTINATION bin )
    #################################################
    
endif ()

###################################################################
# src EXECUTABLE from [F:\Projects\gshhg\src],
# have 1 C/C++ sources, 1 headers
set(name gshhg)
set(dir src)
set(${name}_SRCS
    ${dir}/gshhg.cxx
    )
set(${name}_HDRS
    ${dir}/gshhg.hxx
    )
add_executable( ${name} ${${name}_SRCS} ${${name}_HDRS} )
if (add_LIBS)
    target_link_libraries( ${name} ${add_LIBS} )
endif ()
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
endif ()
# deal with install, if any...
#install( TARGETS ${name} DESTINATION bin )

# eof
