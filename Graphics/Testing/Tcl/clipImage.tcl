package require vtk
package require vtkinteraction
package require vtktesting

# create pipeline
#
vtkVolume16Reader v16
  v16 SetDataDimensions 64 64
  [v16 GetOutput] SetOrigin 0.0 0.0 0.0
  v16 SetDataByteOrderToLittleEndian
  v16 SetFilePrefix "$VTK_DATA_ROOT/Data/headsq/quarter"
  v16 SetImageRange 45 45
  v16 SetDataSpacing 3.2 3.2 1.5
  v16 Update

# do the pixel clipping
vtkClipDataSet clip
  clip SetInput [v16 GetOutput]
  clip SetValue 1000
vtkDataSetMapper clipMapper
  clipMapper SetInput [clip GetOutput]
  clipMapper SetScalarRange 600 1200
vtkActor clipActor
  clipActor SetMapper clipMapper

# put an outline around the data
vtkOutlineFilter outline
  outline SetInput [v16 GetOutput]
vtkPolyDataMapper outlineMapper
  outlineMapper SetInput [outline GetOutput]
vtkActor outlineActor
  outlineActor SetMapper outlineMapper
  outlineActor VisibilityOff

# Create the RenderWindow, Renderer and both Actors
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# Add the actors to the renderer, set the background and size
#
ren1 AddActor outlineActor
ren1 AddActor clipActor

ren1 SetBackground 0.9 .9 .9
renWin SetSize 200 200
iren Initialize

# render the image
#
iren AddObserver UserEvent {wm deiconify .vtkInteract}

# prevent the tk window from showing up then start the event loop
wm withdraw .


