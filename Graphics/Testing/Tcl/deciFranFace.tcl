package require vtktcl

# Create the RenderWindow, Renderer and both Actors
#
vtkRenderer ren1
vtkRenderer ren2
vtkRenderer ren3
vtkRenderer ren4
vtkRenderWindow renWin
    renWin AddRenderer ren1
    renWin AddRenderer ren2
    renWin AddRenderer ren3
    renWin AddRenderer ren4

vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

vtkPNGReader pnm1
    pnm1 SetFileName "$VTK_DATA_ROOT/Data/fran_cut.png"

vtkTexture atext
  atext SetInput [pnm1 GetOutput]
  atext InterpolateOn

# create a cyberware source
#
vtkPolyDataReader fran
    fran SetFileName "$VTK_DATA_ROOT/Data/fran_cut.vtk"

vtkPolyDataMapper mp1
mp1 SetInput [fran GetOutput]

set topologies "On Off"
set accumulates "On Off"
foreach topology $topologies {
  foreach accumulate $accumulates {
    vtkDecimatePro deci$topology$accumulate
      deci$topology$accumulate SetInput [fran GetOutput]
      deci$topology$accumulate SetTargetReduction .99
      deci$topology$accumulate PreserveTopology$topology
      deci$topology$accumulate AccumulateError$accumulate
    vtkPolyDataMapper mapper$topology$accumulate
      mapper$topology$accumulate SetInput [deci$topology$accumulate GetOutput]
    vtkActor fran$topology$accumulate
      fran$topology$accumulate SetMapper mapper$topology$accumulate
      fran$topology$accumulate SetTexture atext
  }
}

# Add the actors to the renderer, set the background and size
#
ren1 SetViewport 0 .5 .5 1
ren2 SetViewport .5 .5 1 1
ren3 SetViewport 0 0 .5 .5
ren4 SetViewport .5 0 1 .5

franOnOn SetMapper mp1

ren1 AddActor franOnOn
ren2 AddActor franOnOff
ren3 AddActor franOffOn
ren4 AddActor franOffOff

vtkCamera camera
ren1 SetActiveCamera camera
ren2 SetActiveCamera camera
ren3 SetActiveCamera camera
ren4 SetActiveCamera camera

[ren1 GetActiveCamera] SetPosition 0.314753 -0.0699988 -0.264225 
[ren1 GetActiveCamera] SetFocalPoint 0.00188636 -0.136847 -5.84226e-09 
[ren1 GetActiveCamera] SetViewAngle 30
[ren1 GetActiveCamera] SetViewUp  0 1 0

ren1 ResetCameraClippingRange
ren2 ResetCameraClippingRange
ren3 ResetCameraClippingRange
ren4 ResetCameraClippingRange

ren1 SetBackground 1 1 1
ren2 SetBackground 1 1 1
ren3 SetBackground 1 1 1
ren4 SetBackground 1 1 1

renWin SetSize 500 500

# render the image
#
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize

# prevent the tk window from showing up then start the event loop
wm withdraw .


