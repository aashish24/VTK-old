vtk_module(vtkRenderingTk
  GROUPS
    Tk
  DEPENDS
    vtkRenderingOpenGL
    vtkInteractionStyle
    vtkInteractionImage
  COMPILE_DEPENDS
    vtkTclTk
  EXCLUDE_FROM_WRAPPING
  )
