package require vtk
package require vtkinteraction
#
# create a triangular texture
#

# Create the RenderWindow, Renderer and both Actors
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin


vtkTriangularTexture aTriangularTexture
    aTriangularTexture SetTexturePattern 1
    aTriangularTexture SetXSize 32
    aTriangularTexture SetYSize 32

vtkPoints points
    points InsertPoint 0 0.0 0.0 0.0
    points InsertPoint 1 1.0 0.0 0.0
    points InsertPoint 2 .5 1.0 0.0
    points InsertPoint 3 1.0 0.0 0.0
    points InsertPoint 4 0.0 0.0 0.0
    points InsertPoint 5 .5 -1.0 .5

vtkTCoords tCoords
    tCoords InsertTCoord 0 0.0 0.0 0.0
    tCoords InsertTCoord 1 1.0 0.0 0.0
    tCoords InsertTCoord 2 .5 .86602540378443864676 0.0
    tCoords InsertTCoord 3 0.0 0.0 0.0
    tCoords InsertTCoord 4 1.0 0.0 0.0
    tCoords InsertTCoord 5 .5 .86602540378443864676 0.0

vtkPointData pointData
    pointData SetTCoords tCoords

vtkCellArray triangles
    triangles InsertNextCell 3
    triangles InsertCellPoint 0
    triangles InsertCellPoint 1
    triangles InsertCellPoint 2
    triangles InsertNextCell 3
    triangles InsertCellPoint 3
    triangles InsertCellPoint 4
    triangles InsertCellPoint 5

vtkPolyData triangle
    triangle SetPolys triangles
    triangle SetPoints points
    [triangle GetPointData]  SetTCoords tCoords

vtkPolyDataMapper triangleMapper
    triangleMapper SetInput triangle

vtkTexture aTexture
    aTexture SetInput [aTriangularTexture GetOutput]

vtkActor triangleActor
    triangleActor SetMapper triangleMapper
    triangleActor SetTexture aTexture

ren1 SetBackground .3 .7 .2
ren1 AddActor triangleActor
[ren1 GetActiveCamera] Zoom 1.5

# render the image
#
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize

# prevent the tk window from showing up then start the event loop
wm withdraw .
