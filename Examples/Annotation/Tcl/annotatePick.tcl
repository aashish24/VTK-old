# This example demonstrates cell picking using vtkCellPicker.  It displays
# the results of picking using a vtkTextMapper.

#
# First we include the VTK Tcl packages which will make available
# all of the vtk commands to Tcl
#
package require vtk
package require vtkinteraction

# create a sphere source, mapper, and actor
#
vtkSphereSource sphere

vtkPolyDataMapper sphereMapper
    sphereMapper SetInput [sphere GetOutput]
    sphereMapper GlobalImmediateModeRenderingOn
vtkLODActor sphereActor
    sphereActor SetMapper sphereMapper

# create the spikes by glyphing the sphere with a cone.  Create the mapper
# and actor for the glyphs.
vtkConeSource cone
vtkGlyph3D glyph
    glyph SetInput [sphere GetOutput]
    glyph SetSource [cone GetOutput]
    glyph SetVectorModeToUseNormal
    glyph SetScaleModeToScaleByVector
    glyph SetScaleFactor 0.25
vtkPolyDataMapper spikeMapper
    spikeMapper SetInput [glyph GetOutput]
vtkLODActor spikeActor
    spikeActor SetMapper spikeMapper

# Create a cell picker.
vtkCellPicker picker
    picker SetEndPickMethod annotatePick

# Create a text mapper and actor to display the results of picking.
vtkTextMapper textMapper
    textMapper SetFontFamilyToArial
    textMapper SetFontSize 10
    textMapper BoldOn
    textMapper ShadowOn
vtkActor2D textActor
    textActor VisibilityOff
    textActor SetMapper textMapper
    [textActor GetProperty] SetColor 1 0 0

# Create the Renderer, RenderWindow, and RenderWindowInteractor
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin
    iren SetPicker picker

# Add the actors to the renderer, set the background and size
#
ren1 AddActor2D textActor
ren1 AddActor sphereActor
ren1 AddActor spikeActor
ren1 SetBackground 0.1 0.2 0.4
renWin SetSize 300 300

# Get the camera and zoom in closer to the image.
set cam1 [ren1 GetActiveCamera]
$cam1 Zoom 1.4

# Set the user method (bound to key 'u')
iren SetUserMethod {wm deiconify .vtkInteract}
iren Initialize

# Withdraw the default tk window
wm withdraw .

# Create a Tcl procedure to create the text for the text mapper used to
# display the results of picking.
proc annotatePick {} {
    if { [picker GetCellId] < 0 } {
	textActor VisibilityOff

    } else {
	set selPt [picker GetSelectionPoint]
	set x [lindex $selPt 0] 
	set y [lindex $selPt 1]
	set pickPos [picker GetPickPosition]
	set xp [lindex $pickPos 0] 
	set yp [lindex $pickPos 1]
	set zp [lindex $pickPos 2]

	textMapper SetInput "($xp, $yp, $zp)"
	textActor SetPosition $x $y
	textActor VisibilityOn
    }

    renWin Render
}

# Pick the cell at this location.
picker Pick 85 126 0 ren1
