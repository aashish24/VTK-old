package require vtk
package require vtkinteraction
package require vtktesting

# This example demonstrates how to use the vtkImagePlaneWidget to probe
# a 3D image dataset with three orthogonal planes.

# Start by loading some data.
#
vtkVolume16Reader v16
  v16 SetDataDimensions 64 64  
  v16 SetDataByteOrderToLittleEndian  
  v16 SetFilePrefix "$VTK_DATA_ROOT/Data/headsq/quarter"   
  v16 SetImageRange 1 93  
  v16 SetDataSpacing 3.2 3.2 1.5  
  v16 Update  

# An outline is shown for context.
#
vtkOutlineFilter outline
  outline SetInput [v16 GetOutput]  

vtkPolyDataMapper outlineMapper
  outlineMapper SetInput [outline GetOutput]  

vtkActor outlineActor
  outlineActor SetMapper outlineMapper  

# Create the RenderWindow, Renderer and Interactor
# the Interactor is needed for the widgets
#

vtkTkRenderWidget .renWin
set renWin [.renWin GetRenderWindow]
vtkRenderer ren1
# vtkRenderWindow renWin
$renWin AddRenderer ren1


vtkRenderWindowInteractor iren
  iren SetRenderWindow $renWin  

# The shared picker enables us to use 3 planes at one time
# and gets the picking order right
#
vtkCellPicker picker 
  picker SetTolerance 0.005   

# The 3 image plane widgets are used to probe the dataset.
#
vtkImagePlaneWidget planeWidgetX
  planeWidgetX DisplayTextOn
  planeWidgetX SetInput [v16 GetOutput]
  planeWidgetX SetInteractor iren
  planeWidgetX SetPlaneOrientationToXAxes
  planeWidgetX SetSliceIndex 32
  planeWidgetX SetPicker picker
  planeWidgetX SetKeyPressActivationValue "x"
  set prop1 [planeWidgetX GetPlaneProperty]
  $prop1 SetColor 1 0 0
  planeWidgetX On

vtkImagePlaneWidget planeWidgetY
  planeWidgetY DisplayTextOn
  planeWidgetY SetInput [v16 GetOutput]
  planeWidgetY SetInteractor iren
  planeWidgetY SetPlaneOrientationToYAxes
  planeWidgetY SetSliceIndex 32
  planeWidgetY SetPicker picker
  planeWidgetY SetKeyPressActivationValue "y"
  set prop2 [planeWidgetY GetPlaneProperty]
  $prop2 SetColor 1 1 0
  planeWidgetY SetLookupTable [planeWidgetX GetLookupTable]
  planeWidgetY On

# for the z-slice, turn off texture interpolation:
# interpolation is now nearest neighbour, to demonstrate
# cross-hair cursor snapping to pixel centers
#
vtkImagePlaneWidget planeWidgetZ
  planeWidgetZ DisplayTextOn
  planeWidgetZ TextureInterpolateOff
  planeWidgetZ SetInput [v16 GetOutput]
  planeWidgetZ SetInteractor iren
  planeWidgetZ SetPlaneOrientationToZAxes
  planeWidgetZ SetSliceIndex 46
  planeWidgetZ SetPicker picker
  planeWidgetZ SetKeyPressActivationValue "z"
  set prop3 [planeWidgetZ GetPlaneProperty]
  $prop3 SetColor 0 0 1
  planeWidgetZ SetLookupTable [planeWidgetX GetLookupTable]
  planeWidgetZ On

# Add the outline actor to the renderer, set the background and size
#
ren1 AddActor outlineActor

[ren1 GetCullers] RemoveAllItems

ren1 SetBackground  0.1 0.1 0.2
$renWin SetSize 400 400

set cam1 [ren1 GetActiveCamera]
$cam1 Elevation 110
$cam1 SetViewUp 0 0 -1
$cam1 Azimuth 45
ren1 ResetCameraClippingRange

# render the image
#
iren AddObserver UserEvent {wm deiconify .vtkInteract}
$renWin Render

# Prevent the tk window from showing up then start the event loop.
# wm withdraw .
