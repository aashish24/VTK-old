/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkQtLabelRenderStrategy - Renders labels with Qt
//
// .SECTION Description
// This class uses Qt to render labels and compute sizes. The labels are
// rendered to a QImage, then EndFrame() converts that image to a vtkImageData
// and textures the image onto a quad spanning the render area.

#ifndef __vtkQtLabelRenderStrategy_h
#define __vtkQtLabelRenderStrategy_h

#include "vtkLabelRenderStrategy.h"

class vtkLabelSizeCalculator;
class vtkLabeledDataMapper;
class vtkPlaneSource;
class vtkPolyDataMapper2D;
class vtkQImageToImageSource;
class vtkTexture;
class vtkTexturedActor2D;
class vtkTextureMapToPlane;

class VTK_RENDERING_EXPORT vtkQtLabelRenderStrategy : public vtkLabelRenderStrategy
{
 public:
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkQtLabelRenderStrategy, vtkLabelRenderStrategy);
  static vtkQtLabelRenderStrategy* New();

  //BTX
  // Description:
  // Compute the bounds of a label. Must be performed after the renderer is set.
  virtual void ComputeLabelBounds(vtkTextProperty* tprop, vtkUnicodeString label, double bds[4]);

  // Description:
  // Render a label at a location in world coordinates.
  // Must be performed between StartFrame() and EndFrame() calls.
  virtual void RenderLabel(double x[3], vtkTextProperty* tprop, vtkUnicodeString label);
  //ETX

  // Description:
  // Start a rendering frame. Renderer must be set.
  virtual void StartFrame();

  // Description:
  // End a rendering frame.
  virtual void EndFrame();

protected:
  vtkQtLabelRenderStrategy();
  ~vtkQtLabelRenderStrategy();

  //BTX
  class Internals;
  Internals* Implementation;
  //ETX

  vtkQImageToImageSource* QImageToImage;
  vtkPlaneSource* PlaneSource;
  vtkTextureMapToPlane* TextureMapToPlane;
  vtkTexture* Texture;
  vtkPolyDataMapper2D* Mapper;
  vtkTexturedActor2D* Actor;

private:
  vtkQtLabelRenderStrategy(const vtkQtLabelRenderStrategy&);  // Not implemented.
  void operator=(const vtkQtLabelRenderStrategy&);  // Not implemented.
};

#endif

