package require vtktcl

# we have to make sure it works with multiple scalar components 

# Image pipeline

vtkBMPReader reader
  reader SetFileName "$VTK_DATA_ROOT/Data/masonry.bmp"
  reader SetDataExtent 0 255 0 255 0 0
  reader SetDataSpacing 1 1 1
  reader SetDataOrigin 0 0 0
  reader UpdateWholeExtent

vtkTransform transform
  transform RotateZ 45
  transform Scale 1.414 1.414 1.414

vtkImageReslice reslice
  reslice SetInput [reader GetOutput]
  reslice SetResliceTransform transform
  reslice InterpolateOn
  reslice SetInterpolationModeToCubic
  reslice WrapOn
  reslice AutoCropOutputOn

vtkImageViewer viewer
  viewer SetInput [reslice GetOutput]
  viewer SetZSlice 0
  viewer SetColorWindow 256.0
  viewer SetColorLevel 127.5
  viewer Render



