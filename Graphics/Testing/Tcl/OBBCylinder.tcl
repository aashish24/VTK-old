package require vtktcl
package require vtktcl_interact

vtkCylinderSource cylinder
cylinder SetHeight 1
cylinder SetRadius 4
cylinder SetResolution 100
cylinder CappingOff

vtkTransform foo
foo RotateX 20
foo RotateY 10
foo RotateZ 27
foo Scale 1 .7 .3

vtkTransformPolyDataFilter transPD
transPD SetInput [cylinder GetOutput]
transPD SetTransform foo

vtkPolyDataMapper dataMapper
  dataMapper SetInput [transPD GetOutput]
vtkActor model
  model SetMapper dataMapper
  [model GetProperty] SetColor 1 0 0


vtkOBBTree obb
  obb SetMaxLevel 10
  obb SetNumberOfCellsPerBucket 5
  obb AutomaticOff

vtkSpatialRepresentationFilter boxes
  boxes SetInput [transPD GetOutput]
  boxes SetSpatialRepresentation obb
vtkPolyDataMapper boxMapper
  boxMapper SetInput [boxes GetOutput]
vtkActor boxActor
  boxActor SetMapper boxMapper
  [boxActor GetProperty] SetAmbient 1
  [boxActor GetProperty] SetDiffuse 0
  [boxActor GetProperty] SetRepresentationToWireframe


vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# Add the actors to the renderer, set the background and size
#
ren1 AddActor model
ren1 AddActor boxActor
ren1 SetBackground 0.1 0.2 0.4
renWin SetSize 300 300
[ren1 GetActiveCamera] Zoom 1.5

# render the image
#
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize

# prevent the tk window from showing up then start the event loop
wm withdraw .

