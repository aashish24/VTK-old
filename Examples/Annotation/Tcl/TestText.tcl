# This example demonstrates the use of 2D text.

#
# First we include the VTK Tcl packages which will make available
# all of the vtk commands to Tcl
#
package require vtk
package require vtkinteraction

# Create a sphere source, mapper, and actor
vtkSphereSource sphere

vtkPolyDataMapper   sphereMapper
    sphereMapper SetInput [sphere GetOutput]
    sphereMapper GlobalImmediateModeRenderingOn
vtkLODActor sphereActor
    sphereActor SetMapper sphereMapper

# Create a scaled text actor. 
# Set the text, font, justification, and properties (bold, italics, etc.).
vtkTextActor textActor
    textActor SetInput "This is a sphere"
    textActor SetFontSize 18
    textActor SetFontFamilyToArial
    textActor SetJustificationToCentered
    textActor BoldOn
    textActor ItalicOn
    textActor ShadowOn
    textActor ScaledTextOn
    textActor SetDisplayPosition 90 50 
    [textActor GetProperty] SetColor 0 0 1

# Create the Renderer, RenderWindow, RenderWindowInteractor
#
vtkRenderer ren1
vtkRenderWindow renWin
    renWin AddRenderer ren1
vtkRenderWindowInteractor iren
    iren SetRenderWindow renWin

# Add the actors to the renderer; set the background and size; zoom in;
# and render.
#
ren1 AddActor2D textActor
ren1 AddActor sphereActor

ren1 SetBackground 1 1 1
renWin SetSize 250 125
[ren1 GetActiveCamera] Zoom 1.5
renWin Render

# Set the user method (bound to key 'u')
iren AddObserver UserEvent {wm deiconify .vtkInteract}

# Withdraw the tk window
wm withdraw .
