IF (WIN32)
  IF (NOT BORLAND)
   IF(NOT CYGWIN)
    VTK_GET_TCL_TK_VERSION ("TCL_TK_MAJOR_VERSION" "TCL_TK_MINOR_VERSION")
    IF (TCL_TK_MAJOR_VERSION AND TCL_TK_MINOR_VERSION)
      SET (TK_RESOURCE_FILE_TRY 
           "${VTK_SOURCE_DIR}/Utilities/TclTk/resources/tk${TCL_TK_MAJOR_VERSION}.${TCL_TK_MINOR_VERSION}/win/rc/tk.rc")
      IF (EXISTS ${TK_RESOURCE_FILE_TRY})
        SET(TK_RESOURCE_FILE ${TK_RESOURCE_FILE_TRY} CACHE INTERNAL 
              "The tk.rc resource file required to add the proper resources to a Tk command-line interpreter (i.e. vtk.exe)")
      ENDIF (EXISTS ${TK_RESOURCE_FILE_TRY})
    ENDIF (TCL_TK_MAJOR_VERSION AND TCL_TK_MINOR_VERSION)
    IF (TK_RESOURCE_FILE)
      GET_FILENAME_COMPONENT(TK_RESOURCE_PATH ${TK_RESOURCE_FILE} PATH)
      INCLUDE_DIRECTORIES(${TK_RESOURCE_PATH})
      CONFIGURE_FILE(
        ${VTK_SOURCE_DIR}/Wrapping/Tcl/resources/vtk.rc.in
        ${VTK_BINARY_DIR}/Wrapping/Tcl/resources/vtk.rc)
      INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/resources")
      SET(VTK_EXE_RESOURCE_FILES 
          "${VTK_BINARY_DIR}/Wrapping/Tcl/resources/vtk.rc")
    ENDIF (TK_RESOURCE_FILE)
   ENDIF(NOT CYGWIN)
  ENDIF (NOT BORLAND)
ENDIF (WIN32)

