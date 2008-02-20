from PyQt4 import QtCore
from PyQt4 import QtGui
from PyQt4 import uic
from vtk import *
import sys

application = QtGui.QApplication(sys.argv)

window = uic.loadUi("gui.ui")
vertex_slider = window.findChild(QtGui.QSlider, "vertexCount")
edge_slider = window.findChild(QtGui.QSlider, "edgeCount")

source = vtkRandomGraphSource()
source.SetNumberOfVertices(vertex_slider.value())
source.SetNumberOfEdges(edge_slider.value())

def change_vertex_count(count):
  source.SetNumberOfVertices(count)
  edge_slider.setValue(source.GetNumberOfEdges())
  view.GetRenderer().ResetCamera()
  render_window.Render()

def change_edge_count(count):
  source.SetNumberOfEdges(count)
  edge_slider.setValue(source.GetNumberOfEdges())
  view.GetRenderer().ResetCamera()
  render_window.Render()

QtCore.QObject.connect(vertex_slider, QtCore.SIGNAL("valueChanged(int)"), change_vertex_count)
QtCore.QObject.connect(edge_slider, QtCore.SIGNAL("valueChanged(int)"), change_edge_count)
window.show()

# Setup a VTK window, but don't start an event-loop ...
view = vtkGraphLayoutView()
view.AddRepresentationFromInputConnection(source.GetOutputPort())

render_window = vtkRenderWindow()
render_window.SetSize(600, 600)
view.SetupRenderWindow(render_window)
render_window.Start()

# Start the Qt event-loop ...
application.exec_()

