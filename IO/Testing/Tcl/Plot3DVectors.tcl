package require vtktcl_interactor

#
# All Plot3D vector functions
#

# Create the RenderWindow, Renderer and both Actors
#
vtkRenderWindow renWin
vtkRenderer ren1
  ren1 SetBackground .8 .8 .2
renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

set vectorLabels  "Velocity Vorticity Momentum Pressure_Gradient"
set vectorFunctions  "200 201 202 210"
vtkCamera camera
vtkLight light

set i 0
foreach vectorFunction $vectorFunctions {
  vtkPLOT3DReader pl3d$vectorFunction
    pl3d$vectorFunction SetXYZFileName "$VTK_DATA_ROOT/Data/bluntfinxyz.bin"
    pl3d$vectorFunction SetQFileName "$VTK_DATA_ROOT/Data/bluntfinq.bin"
    pl3d$vectorFunction SetVectorFunctionNumber $vectorFunction
    pl3d$vectorFunction Update
vtkStructuredGridGeometryFilter plane$vectorFunction
    plane$vectorFunction SetInput [pl3d$vectorFunction GetOutput]
    plane$vectorFunction SetExtent 25 25 0 100 0 100
vtkHedgeHog hog$vectorFunction
  hog$vectorFunction SetInput [plane$vectorFunction GetOutput]
  hog$vectorFunction SetScaleFactor [expr 1.0 / [[[[pl3d$vectorFunction GetOutput] GetPointData] GetVectors] GetMaxNorm]]
vtkPolyDataMapper mapper$vectorFunction
    mapper$vectorFunction SetInput [hog$vectorFunction GetOutput]
vtkActor actor$vectorFunction
    actor$vectorFunction SetMapper mapper$vectorFunction

vtkRenderer ren$vectorFunction
  ren$vectorFunction SetBackground 0.5 .5 .5
  ren$vectorFunction SetActiveCamera camera
  ren$vectorFunction AddLight light
  renWin AddRenderer ren$vectorFunction

ren$vectorFunction AddActor actor$vectorFunction

vtkTextMapper textMapper$vectorFunction
  textMapper$vectorFunction SetInput [lindex $vectorLabels $i]
  textMapper$vectorFunction SetFontSize 10
  textMapper$vectorFunction SetFontFamilyToArial
vtkActor2D text$vectorFunction
  text$vectorFunction SetMapper textMapper$vectorFunction
  text$vectorFunction SetPosition 2 5
  [text$vectorFunction GetProperty] SetColor .3 1 1
  if { [info command rtExMath] == ""} {
    ren$vectorFunction AddActor2D text$vectorFunction
  }
incr i
}
#
# now layout renderers
set column 1
set row 1
set deltaX [expr 1.0/2.0]
set deltaY [expr 1.0/2.0]

foreach vectorFunction $vectorFunctions {
    ren${vectorFunction} SetViewport [expr ($column - 1) * $deltaX + ($deltaX * .05)] [expr ($row - 1) * $deltaY + ($deltaY*.05)] [expr $column * $deltaX - ($deltaX * .05)] [expr $row * $deltaY - ($deltaY * .05)]
    incr column
    if { $column > 2 } {set column 1; incr row}
}


camera SetViewUp 1 0 0
camera SetFocalPoint 0 0 0
camera SetPosition .4 -.5 -.75
ren200 ResetCamera
camera Dolly 1.25
ren200 ResetCameraClippingRange
ren201 ResetCameraClippingRange
ren202 ResetCameraClippingRange
ren210 ResetCameraClippingRange

eval light SetPosition [camera GetPosition]
eval light SetFocalPoint [camera GetFocalPoint]

renWin SetSize 350 350
renWin Render

iren Initialize

# render the image
#
iren SetUserMethod {wm deiconify .vtkInteract}

# prevent the tk window from showing up then start the event loop
wm withdraw .
