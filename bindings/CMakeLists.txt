# Copyright: (C) 2009 RobotCub Consortium
# Authors: Paul Fitzpatrick, Arjan Gijsberts, Lorenzo Natale, Fabien Benureau, Stephane Lallee, Ali Paikan
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

cmake_minimum_required(VERSION 2.8.9)

include(CMakeDependentOption)
option(YARP_COMPILE_BINDINGS "Compile optional language bindings" FALSE)


#############################################################################
## Options for compiling supported languages.  There's nothing magical
## about this list, any SWIG-supported language should work - take a
## look at e.g. ruby code below for how to do it.

foreach(lang "Java"
             "Python"
             "Perl"
             "Chicken Scheme"
             "CSharp"
             "Allegro Common Lisp"
             "TCL"
             "Ruby"
             "Lua"
             "Octave")
  string(TOUPPER "${lang}" LANG)
  string(REGEX REPLACE " .+" "" LANG "${LANG}")
  cmake_dependent_option(CREATE_${LANG} "Do you want to create the ${lang} interface" OFF
                         YARP_COMPILE_BINDINGS OFF)
  if(YARP_COMPILE_BINDINGS)
    if(CREATE_${LANG})
      message(STATUS " +++ bindings ${lang} is enabled")
    else()
      message(STATUS " +++ bindings ${lang} is not enabled")
    endif()
  endif()
endforeach()


if(YARP_COMPILE_BINDINGS)


#############################################################################
## Find YARP and SWIG

get_property(YARP_INCLUDE_DIRS GLOBAL PROPERTY YARP_TREE_INCLUDE_DIRS)
get_property(YARP_LIBRARIES GLOBAL PROPERTY YARP_LIBS)
include_directories(${YARP_INCLUDE_DIRS})

find_package(SWIG "1.3.29" REQUIRED)
include(${SWIG_USE_FILE})

set_source_files_properties(yarp.i PROPERTIES CPLUSPLUS ON)


#############################################################################
## Disable deprecated warnings, since we are building bindings also for
## deprecated methods

if(NOT YARP_NO_DEPRECATED AND NOT MSVC)
  check_cxx_compiler_flag("-Wno-deprecated-declarations" CXX_HAS_WNO_DEPRECATED_DECLARATIONS)
  if(CXX_HAS_WNO_DEPRECATED_DECLARATIONS)
    add_definitions(-Wno-deprecated-declarations)
  endif()
endif()


#############################################################################
## Create Java bindings

if(CREATE_JAVA)
  ### Options
  option(PREPARE_CLASS_FILES "Prepare class files" OFF)
  set(JAVA_TARGET_VERSION 1.6 CACHE STRING "Java target version")

  ### Find required packages and include required directories
  find_package(JNI REQUIRED)
  find_program(JAVA_JAVAC_EXECUTABLE NAMES javac)
  if(NOT JAVA_JAVAC_EXECUTABLE)
    message(FATAL_ERROR "Could not find javac compiler")
  endif()
  include_directories(SYSTEM ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})

  ### Set SWIG options
  set(CMAKE_SWIG_OUTDIR "${CMAKE_BINARY_DIR}/generated_src/java")
  set(CMAKE_SWIG_FLAGS "-package;yarp;-Wall;-module;yarp")

  ### Add SWIG target
  set(target_name yarp_bindings_java)
  swig_add_module(${target_name} java yarp.i)
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "jyarp")

  ### Prepare class files
  if(PREPARE_CLASS_FILES)
    set(TARGET_DIR generated_src/java)
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${TARGET_DIR}")
    set(BINDINGS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(JAVA_SRCS LoadYarp.java
                  YarpImageHelper.java
                  Example1.java)
    set(CLASS_FILES)
    foreach(java_file ${JAVA_SRCS})
      string(REPLACE ".java" ".class" class_file ${java_file})
      list(APPEND JAVA_FILES "${BINDINGS_SRC_DIR}/${java_file}")
      list(APPEND CLASS_FILES "${CMAKE_BINARY_DIR}/${class_file}")
    endforeach()
    add_custom_command(OUTPUT ${CLASS_FILES}
                       COMMAND ${JAVA_JAVAC_EXECUTABLE} -source ${JAVA_TARGET_VERSION}
                                                        -target ${JAVA_TARGET_VERSION}
                                                        -d "${CMAKE_BINARY_DIR}"
                                                        "${CMAKE_BINARY_DIR}/${TARGET_DIR}/*.java"
                       COMMAND ${JAVA_JAVAC_EXECUTABLE} -source ${JAVA_TARGET_VERSION}
                                                        -target ${JAVA_TARGET_VERSION}
                                                        -d "${CMAKE_BINARY_DIR}/"
                                                        "${BINDINGS_SRC_DIR}/*.java"
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                       DEPENDS ${JAVA_FILES} ${target_name})
    add_custom_target(${target_name}_classes ALL DEPENDS ${CLASS_FILES})
  endif()

  ### Issues
  # issue on MINGW where C++ name-mangling prevents java finding methods
  if(MINGW)
    message(STATUS "untested MINGW patch - see CMakeLists.txt")
    set_target_properties(${target_name} PROPERTIES LINK_FLAGS "--add-stdcall-alias")
  endif(MINGW)
  ## the following is based on:
  ## http://www.cmake.org/pipermail/cmake/2007-September/016343.html
  if(APPLE)
    set_target_properties(${target_name} PROPERTIES SUFFIX ".jnilib")
  endif(APPLE)

  # FIXME Add installation
endif()


#############################################################################
## Create Python bindings

if(CREATE_PYTHON)
  ### Options
  if(CREATE_PYTHON_VERSION)
    message(WARNING "CREATE_PYTHON_VERSION was renamed YARP_USE_PYTHON_VERSION")
    set(YARP_USE_PYTHON_VERSION "${CREATE_PYTHON_VERSION}" CACHE STRING "Specify python version to use" )
    unset(CREATE_PYTHON_VERSION CACHE)
  else()
    set(YARP_USE_PYTHON_VERSION "" CACHE STRING "Specify python version to use" )
  endif()

  ### Find required packages and include required directories
  find_package(PythonInterp ${YARP_USE_PYTHON_VERSION} REQUIRED)
  if(NOT YARP_USE_PYTHON_VERSION)
    set (YARP_USE_PYTHON_VERSION ${PYTHON_VERSION_STRING})
  endif()
  find_package(PythonLibs ${YARP_USE_PYTHON_VERSION} EXACT REQUIRED)
  include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})

  ### Set SWIG options
  set(CMAKE_SWIG_OUTDIR "${CMAKE_BINARY_DIR}/lib/python")
  set(CMAKE_SWIG_FLAGS "-Wall;-module;yarp;-threads")

  ### Add SWIG target
  set(target_name yarp_bindings_python)
  swig_add_module(${target_name} python yarp.i)
  swig_link_libraries(${target_name} ${PYTHON_LIBRARY})
  set(target_name _${target_name}) # For some reason, swig target for python bindings starts with an "_"
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "_yarp")
  set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/python")
  if(WIN32)
    set_target_properties(${target_name} PROPERTIES SUFFIX ".pyd")
  endif(WIN32)

  # uncomment if we want a debug version of the library
  #add_definitions(-DPy_DEBUG)

  ### Installation
  # installation path is determined reliably on most platforms using distutils
  execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_python_lib(1,0,prefix=''))"
                  OUTPUT_VARIABLE PYTHON_INSTDIR
                  OUTPUT_STRIP_TRAILING_WHITESPACE )
  install(FILES ${CMAKE_BINARY_DIR}/lib/python/yarp.py
          DESTINATION ${PYTHON_INSTDIR})
  install(TARGETS ${target_name}
          DESTINATION ${PYTHON_INSTDIR})

endif()


#############################################################################
## Create Perl bindings

if(CREATE_PERL)
  ### Find required packages and include required directories
  find_package(PerlLibs)
  include_directories(SYSTEM ${PERL_INCLUDE_PATH})

  ### Find Perl dir
  get_filename_component(_CMAKE_INSTALL_PERLDIR ${PERL_LIBRARY} PATH)
  file(RELATIVE_PATH _CMAKE_INSTALL_PERLDIR "${_CMAKE_INSTALL_PERLDIR}" "${PERL_VENDORARCH}")
  set(_CMAKE_INSTALL_PERLDIR "${CMAKE_INSTALL_LIBDIR}/${_CMAKE_INSTALL_PERLDIR}")

  ### Options
  set(CMAKE_INSTALL_PERLDIR "${_CMAKE_INSTALL_PERLDIR}" CACHE PATH "perl bindings (${_CMAKE_INSTALL_PERLDIR})")
  mark_as_advanced(CMAKE_INSTALL_PERLDIR)
  if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_PERLDIR})
    set(CMAKE_INSTALL_FULL_PERLDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_PERLDIR}")
  else()
    set(CMAKE_INSTALL_FULL_PERLDIR "${CMAKE_INSTALL_PERLDIR}")
  endif()

  ### Set SWIG options
  set(CMAKE_SWIG_OUTDIR "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PERLDIR}")
  set(CMAKE_SWIG_FLAGS "-Wall;-module;yarp")

  ### Add SWIG target
  set(target_name yarp_bindings_perl)
  swig_add_module(${target_name} perl5 yarp.i)
  set_target_properties(${target_name} PROPERTIES PREFIX "")
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "yarp")
  set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PERLDIR}/auto/yarp")

  ### Update RPATH
  if(NOT CMAKE_SKIP_RPATH AND NOT CMAKE_SKIP_INSTALL_RPATH)
    file(RELATIVE_PATH _rel_path "${CMAKE_INSTALL_FULL_PERLDIR}" "${CMAKE_INSTALL_FULL_LIBDIR}")
    get_target_property(_current_rpath ${target_name} INSTALL_RPATH)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      list(APPEND _current_rpath "@loader_path/${_rel_path}")
    else()
      list(APPEND _current_rpath "\$ORIGIN/${_rel_path}")
    endif()
    set_target_properties(${target_name} PROPERTIES INSTALL_RPATH "${_current_rpath}")
  endif()

  ### Installation
  install(TARGETS ${target_name}
          DESTINATION ${CMAKE_INSTALL_PERLDIR}/auto/yarp)
  install(FILES ${CMAKE_SWIG_OUTDIR}/yarp.pm
          DESTINATION ${CMAKE_INSTALL_PERLDIR})

endif()


#############################################################################
## Create Chicken Scheme bindings

if(CREATE_CHICKEN)
  # FIXME Find chicken

  ### Set SWIG options
  set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/chicken")
  set(CMAKE_SWIG_FLAGS "-Wall;-module;cyarp")

  ### Add SWIG target
  set(target_name yarp_bindings_chicken)
  swig_add_module(${target_name} chicken yarp.i)
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "libcyarp")

  # FIXME Add installation
endif()


#############################################################################
## Create Allegro Common Lisp bindings

if(CREATE_ALLEGRO)
  ### Options
  set(ALLEGRO_INSTALL_DIR "lib/allegro/site" CACHE PATH "Install path of allegro bindings.")
  set(ALLEGRO_ASDF_SYSTEMS_DIR "lib/allegro/asdf-systems" CACHE PATH "Path to register asdf file.")

  ### Set SWIG options
  set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/allegro")
  set(CMAKE_SWIG_FLAGS "-Wall;-module;yarp;-identifier-converter;lispify")

  ### Add SWIG target
  set(target_name yarp_bindings_allegro)
  swig_add_module(${target_name} allegrocl yarp.i)
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "yarp-allegro")

  ### Installation
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/allegro/yarp.asd
          DESTINATION ${ALLEGRO_INSTALL_DIR}/yarp)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/yarp.cl
          DESTINATION ${ALLEGRO_INSTALL_DIR}/yarp/src
          RENAME yarp.lisp)
  install(TARGETS ${target_name}
          DESTINATION ${ALLEGRO_INSTALL_DIR}/yarp/lib)
  if(UNIX)
    install(CODE "execute_process(COMMAND \"mkdir\" \"-p\" \"${ALLEGRO_ASDF_SYSTEMS}\"
                                  COMMAND \"ln\" \"-s\" \"${CMAKE_INSTALL_PREFIX}/${ALLEGRO_INSTALL_DIR}/yarp/yarp.asd\" \"${CMAKE_INSTALL_PREFIX}/${ALLEGRO_ASDF_SYSTEMS}\")")
  endif()
endif()


#############################################################################
## Create TCL bindings


if(CREATE_TCL)
  ### Find required packages and include required directories
  find_package(TCL REQUIRED)
  include_directories(SYSTEM ${TCL_INCLUDE_PATH})

  ### Set SWIG options
  set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/tcl")
  set(CMAKE_SWIG_FLAGS "-Wall;-module;yarp")

  ### Add SWIG target
  set(target_name yarp_bindings_tcl)
  swig_add_module(${target_name} tcl yarp.i)
  swig_link_libraries(${target_name} ${TCL_LIBRARY})
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "libtclyarp")

  # FIXME Add installation
endif()


#############################################################################
## Create C# bindings

if(CREATE_CSHARP)
  ### Options
  if(WIN32 AND NOT CYGWIN)
    option(PREPARE_CLASS_FILES "Prepare NET library" ON)
  endif()

  ### Set SWIG options
  set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/csharp")
  if(NOT ${CMAKE_MINIMUM_REQUIRED_VERSION} VERSION_LESS 3.0)
    message(AUTHOR_WARNING "CMAKE_MINIMUM_REQUIRED_VERSION is now ${CMAKE_MINIMUM_REQUIRED_VERSION}. This check can be removed.")
  endif()
  # -dllimport is actually needed only for CMake versions prior to 3.0 ( http://public.kitware.com/Bug/view.php?id=13814 )
  if(${CMAKE_VERSION} VERSION_LESS 3.0)
    set(CMAKE_SWIG_FLAGS "-Wall;-dllimport;${target_name}")
  else()
    set(CMAKE_SWIG_FLAGS "-Wall")
  endif()

  ### Add SWIG target
  set(target_name yarp_bindings_csharp)
  swig_add_module(${target_name} csharp yarp.i)
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "yarp_cs")

  ### Prepare class files
  if(WIN32 AND PREPARE_CLASS_FILES)
    add_custom_command(TARGET ${target_name}
                       POST_BUILD
                       COMMAND IF NOT EXIST .\\generated_src mkdir generated_src
                       WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    add_custom_command(TARGET ${target_name}
                       POST_BUILD
                       COMMAND move *.cs ${CMAKE_BINARY_DIR}/generated_src
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  # FIXME Add installation
endif()


#############################################################################
## Create Ruby bindings

if(CREATE_RUBY)
  ### Find required packages and include required directories
  find_package(Ruby)
  include_directories(SYSTEM ${RUBY_INCLUDE_PATH})

  ### Set SWIG options
  set(CMAKE_SWIG_FLAGS "-Wall;-module;yarp")
  set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/ruby")

  ### Add SWIG target
  set(target_name yarp_bindings_ruby)
  swig_add_module(${target_name} ruby yarp.i)
  swig_link_libraries(${target_name} ${RUBY_LIBRARY})
  set_target_properties(${target_name} PROPERTIES PREFIX "")
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "yarp")

  # FIXME Add installation
endif()


#############################################################################
## Create Lua bindings

if(CREATE_LUA)
  ### Find required packages and include required directories
  find_package(Lua REQUIRED)
  include_directories(SYSTEM ${LUA_INCLUDE_DIR})

  ### Set SWIG options
  set(CMAKE_SWIG_FLAGS "-Wall;-module;yarp")
  set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/lua")

  ### Add SWIG target
  set(target_name yarp_bindings_lua)
  swig_add_module(${target_name} lua yarp.i)
  swig_link_libraries(${target_name} ${LUA_LIBRARY})
  set_target_properties(${target_name} PROPERTIES PREFIX "")
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "yarp")
  set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/lua)

  ### Installation
  set(LUA_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/lua/${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR})
  install(TARGETS ${target_name}
          DESTINATION ${LUA_INSTALL_DIR})

endif()


#############################################################################
## Create Octave bindings

if(CREATE_OCTAVE)
  ### Find required packages and include required directories
  find_package(Octave REQUIRED)
  get_filename_component(OCTAVE_INCLUDE_TOP ${OCTAVE_INCLUDE_DIR} DIRECTORY)
  include_directories(SYSTEM ${OCTAVE_INCLUDE_DIRS} ${OCTAVE_INCLUDE_TOP})

  ### Set SWIG options
  set(CMAKE_SWIG_FLAGS "-Wall;-module;yarp")
  set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/octave")

  ### Add SWIG target
  set(target_name yarp_bindings_octave)
  swig_add_module(${target_name} octave yarp.i)
  swig_link_libraries(${target_name} ${OCTAVE_LIBRARIES})
  set_target_properties(${target_name} PROPERTIES PREFIX "")
  set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "yarp")
  set_target_properties(${target_name} PROPERTIES SUFFIX ".oct")
  set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/octave)

  # FIXME Add installation
endif()

endif()
