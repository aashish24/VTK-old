set(Module_SRCS
  vtkMathTextUtilities.cxx
  vtkMathTextActor.cxx
  vtkMathTextActor3D.cxx
  )

set_source_files_properties(
  vtkMathTextUtilities
  ABSTRACT
  )

vtk_module_library(${vtk-module} ${Module_SRCS})
