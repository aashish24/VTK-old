package require vtktcl

# This script is for testing the 3D Sobel filter.
# Displays the 3 components using color.


# Image pipeline

vtkImageReader reader
reader ReleaseDataFlagOff
reader SetDataByteOrderToLittleEndian
reader SetDataExtent 0 63 0 63 1 93
reader SetDataSpacing 3.2 3.2 1.5
reader SetFilePrefix "$VTK_DATA_ROOT/Data/headsq/quarter"
reader SetDataMask 0x7fff

vtkImageSobel3D sobel
sobel SetInput [reader GetOutput]
sobel ReleaseDataFlagOff

vtkImageViewer viewer
#viewer DebugOn
viewer SetInput [sobel GetOutput]
viewer SetZSlice 22
viewer SetColorWindow 400
viewer SetColorLevel 0


viewer Render








