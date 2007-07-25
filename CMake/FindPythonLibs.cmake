# - Find python libraries
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  PYTHONLIBS_FOUND     = have the Python libs been found
#  PYTHON_LIBRARIES     = path to the python library
#  PYTHON_INCLUDE_PATH  = path to where Python.h is found
#  PYTHON_DEBUG_LIBRARIES = path to the debug library
#

INCLUDE(CMakeFindFrameworks)
# Search for the python framework on Apple.
CMAKE_FIND_FRAMEWORKS(Python)

FOREACH(_CURRENT_VERSION 2.5 2.4 2.3 2.2 2.1 2.0 1.6 1.5)
  STRING(REPLACE "." "" _CURRENT_VERSION_NO_DOTS ${_CURRENT_VERSION})
  IF(WIN32)
    FIND_LIBRARY(PYTHON_DEBUG_LIBRARY
      NAMES python${_CURRENT_VERSION_NO_DOTS}_d python
      PATHS
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs/Debug
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs )
  ENDIF(WIN32)

  FIND_LIBRARY(PYTHON_LIBRARY
    NAMES python${_CURRENT_VERSION_NO_DOTS} python${_CURRENT_VERSION}
    PATHS
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
    PATH_SUFFIXES
      python${_CURRENT_VERSION}/config
    # Avoid finding the .dll in the PATH.  We want the .lib.
    NO_SYSTEM_ENVIRONMENT_PATH
  )

  SET(PYTHON_FRAMEWORK_INCLUDES)
  IF(Python_FRAMEWORKS AND NOT PYTHON_INCLUDE_PATH)
    FOREACH(dir ${Python_FRAMEWORKS})
      SET(PYTHON_FRAMEWORK_INCLUDES ${PYTHON_FRAMEWORK_INCLUDES}
        ${dir}/Versions/${_CURRENT_VERSION}/include/python${_CURRENT_VERSION})
    ENDFOREACH(dir)
  ENDIF(Python_FRAMEWORKS AND NOT PYTHON_INCLUDE_PATH)

  FIND_PATH(PYTHON_INCLUDE_PATH
    NAMES Python.h
    PATHS
      ${PYTHON_FRAMEWORK_INCLUDES}
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/include
    PATH_SUFFIXES
      python${_CURRENT_VERSION}
  )
  
ENDFOREACH(_CURRENT_VERSION)

MARK_AS_ADVANCED(
  PYTHON_DEBUG_LIBRARY
  PYTHON_LIBRARY
  PYTHON_INCLUDE_PATH
)

# Python Should be built and installed as a Framework on OSX
IF(Python_FRAMEWORKS)
  # If a framework has been selected for the include path,
  # make sure "-framework" is used to link it.
  IF("${PYTHON_INCLUDE_PATH}" MATCHES "Python\\.framework")
    SET(PYTHON_LIBRARY "")
    SET(PYTHON_DEBUG_LIBRARY "")
  ENDIF("${PYTHON_INCLUDE_PATH}" MATCHES "Python\\.framework")
  IF(NOT PYTHON_LIBRARY)
    SET (PYTHON_LIBRARY "-framework Python" CACHE FILEPATH "Python Framework" FORCE)
  ENDIF(NOT PYTHON_LIBRARY)
  IF(NOT PYTHON_DEBUG_LIBRARY)
    SET (PYTHON_DEBUG_LIBRARY "-framework Python" CACHE FILEPATH "Python Framework" FORCE)
  ENDIF(NOT PYTHON_DEBUG_LIBRARY)
ENDIF(Python_FRAMEWORKS)

# We use PYTHON_LIBRARY and PYTHON_DEBUG_LIBRARY for the cache entries
# because they are meant to specify the location of a single library.
# We now set the variables listed by the documentation for this
# module.
SET(PYTHON_LIBRARIES "${PYTHON_LIBRARY}")
SET(PYTHON_DEBUG_LIBRARIES "${PYTHON_DEBUG_LIBRARY}")


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonLibs DEFAULT_MSG PYTHON_LIBRARIES PYTHON_INCLUDE_PATH)

# make sure that the list of python modules is generated new every time cmake runs
# and not appended to the list of the previous cmake run
GET_TARGET_PROPERTY(_FIND_PYTHONLIBS_ALREADY_INCLUDED __FindPythonLibsHelper TYPE)
IF(NOT _FIND_PYTHONLIBS_ALREADY_INCLUDED)
  ADD_CUSTOM_TARGET(__FindPythonLibsHelper)  # so if this file is included several times the next time GET_TARGET_PROPERTIES() will work
  FILE(REMOVE ${CMAKE_BINARY_DIR}/CMakeFiles/PyModulesList.txt)
  FILE(REMOVE ${CMAKE_BINARY_DIR}/CMakeFiles/PyStaticModulesList.txt)
ENDIF(NOT _FIND_PYTHONLIBS_ALREADY_INCLUDED)


MACRO(PYTHON_ADD_MODULE _NAME )
  OPTION(PYTHON_ENABLE_MODULE_${_NAME} "Add module ${_NAME}" TRUE)
  OPTION(PYTHON_MODULE_${_NAME}_SHARED "Add module ${_NAME} shared" FALSE)
  MARK_AS_ADVANCED(PYTHON_ENABLE_MODULE_${_NAME} PYTHON_MODULE_${_NAME}_SHARED)

  IF(PYTHON_ENABLE_MODULE_${_NAME})
    IF(PYTHON_MODULE_${_NAME}_SHARED)
      SET(PY_MODULE_TYPE MODULE)
    ELSE(PYTHON_MODULE_${_NAME}_SHARED)
      SET(PY_MODULE_TYPE STATIC)
      IF(EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/PyStaticModulesList.txt")
        FILE(APPEND "${CMAKE_BINARY_DIR}/CMakeFiles/PyStaticModulesList.txt" ";${_NAME}")
      ELSE(EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/PyStaticModulesList.txt")
        FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/PyStaticModulesList.txt" "${_NAME}")
      ENDIF(EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/PyStaticModulesList.txt")
    ENDIF(PYTHON_MODULE_${_NAME}_SHARED)

    IF(EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/PyModulesList.txt")
      FILE(APPEND "${CMAKE_BINARY_DIR}/CMakeFiles/PyModulesList.txt" ";${_NAME}")
    ELSE(EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/PyModulesList.txt")
      FILE(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/PyModulesList.txt" "${_NAME}")
    ENDIF(EXISTS "${CMAKE_BINARY_DIR}/CMakeFiles/PyModulesList.txt")
    ADD_LIBRARY(${_NAME} ${PY_MODULE_TYPE} ${ARGN})
#    TARGET_LINK_LIBRARIES(${_NAME} ${PYTHON_LIBRARIES})
  ENDIF(PYTHON_ENABLE_MODULE_${_NAME})
ENDMACRO(PYTHON_ADD_MODULE)


MACRO(PYTHON_WRITE_MODULES_HEADER _filename)
  FILE(READ "${CMAKE_BINARY_DIR}/CMakeFiles/PyStaticModulesList.txt" PY_STATIC_MODULES_LIST)
  FILE(READ "${CMAKE_BINARY_DIR}/CMakeFiles/PyModulesList.txt"       PY_MODULES_LIST)
  message(STATUS "modules: ${PY_MODULES_LIST}")
  message(STATUS "static modules: ${PY_STATIC_MODULES_LIST}")
  GET_FILENAME_COMPONENT(_name "${_filename}" NAME)
  STRING(REPLACE "." "_" _name "${_name}")
  STRING(TOUPPER ${_name} _name)
  FILE(WRITE ${_filename} "/*Created by cmake, do not edit, changes will be lost*/\n")
  FILE(APPEND ${_filename} 
"#ifndef ${_name}
#define ${_name}

#include <Python.h>

#ifdef __cplusplus
extern \"C\" {
#endif /* __cplusplus */

")

  FOREACH(_currentModule ${PY_MODULES_LIST})
    FILE(APPEND ${_filename} "extern void init${PYTHON_MODULE_PREFIX}${_currentModule}(void);\n\n")
  ENDFOREACH(_currentModule ${PY_MODULES_LIST})

  FILE(APPEND ${_filename} 
"#ifdef __cplusplus
}
#endif /* __cplusplus */

")


  FOREACH(_currentModule ${PY_MODULES_LIST})
    FILE(APPEND ${_filename} "int CMakeLoadPythonModule_${_currentModule}(void) \n{\n  return PyImport_AppendInittab(\"${PYTHON_MODULE_PREFIX}${_currentModule}\", init${PYTHON_MODULE_PREFIX}${_currentModule});\n}\n\n")
  ENDFOREACH(_currentModule ${PY_MODULES_LIST})

  FILE(APPEND ${_filename} "#ifndef EXCLUDE_LOAD_ALL_FUNCTION\nvoid CMakeLoadAllPythonModules(void)\n{\n")
  FOREACH(_currentModule ${PY_STATIC_MODULES_LIST})
    FILE(APPEND ${_filename} "  CMakeLoadPythonModule_${_currentModule}();\n")
  ENDFOREACH(_currentModule ${PY_STATIC_MODULES_LIST})
  FILE(APPEND ${_filename} "}\n#endif\n\n#endif\n")
ENDMACRO(PYTHON_WRITE_MODULES_HEADER)
