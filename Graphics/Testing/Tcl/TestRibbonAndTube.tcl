package require vtk
package require vtkinteraction

# create pipeline
#
vtkPolyDataReader reader
    reader SetFileName "$VTK_DATA_ROOT/Data/vtk.vtk"

# Read a ruler texture
vtkPNGReader r
  r SetFileName "$VTK_DATA_ROOT/Data/ruler.png"
vtkTexture atext
  atext SetInput [r GetOutput]
  atext InterpolateOn

# produce some ribbons
vtkRibbonFilter ribbon
    ribbon SetInput [reader GetOutput]
    ribbon SetWidth 0.1
    ribbon SetGenerateTCoordsToUseLength
    ribbon SetTextureLength 1.0
    ribbon UseDefaultNormalOn
    ribbon SetDefaultNormal 0 0 1
vtkPolyDataMapper ribbonMapper
    ribbonMapper SetInput [ribbon GetOutput]
vtkActor ribbonActor
    ribbonActor SetMapper ribbonMapper
    eval [ribbonActor GetProperty] SetColor 1 1 0
    ribbonActor SetTexture atext

# produce some tubes
vtkTubeFilter tuber
    tuber SetInput [reader GetOutput]
    tuber SetRadius 0.1
    tuber SetNumberOfSides 12
    tuber SetGenerateTCoordsToUseLength
    tuber SetTextureLength 0.5
    tuber CappingOn
vtkPolyDataMapper tubeMapper
    tubeMapper SetInput [tuber GetOutput]
vtkActor tubeActor
    tubeActor SetMapper tubeMapper
    eval [tubeActor GetProperty] SetColor 1 1 0
    tubeActor SetTexture atext
    tubeActor AddPosition 5 0 0

# Create the RenderWindow, Renderer and both Actors
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# Add the actors to the renderer, set the background and size
#
ren1 AddActor ribbonActor
ren1 AddActor tubeActor

ren1 SetBackground 1 1 1
renWin SetSize 900 350
eval ren1 SetBackground 1 1 1
[ren1 GetActiveCamera] Zoom 4

# render the image
#
renWin Render
iren AddObserver UserEvent {wm deiconify .vtkInteract}

# prevent the tk window from showing up then start the event loop
wm withdraw .

