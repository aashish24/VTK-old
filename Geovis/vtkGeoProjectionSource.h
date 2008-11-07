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
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
// .NAME vtkGeoProjectionSource - A 2D geographic geometry source
//
// .SECTION Description
// vtkGeoProjectionSource is a vtkGeoSource suitable for use in vtkTerrain2D.
// This source uses the libproj4 library to produce geometry patches at
// multiple resolutions. Each patch covers a specific region in projected
// space.

#include "vtkGeoSource.h"

class vtkGeoTerrainNode;
class vtkTransformFilter;
class vtkMutexLock;

class VTK_GEOVIS_EXPORT vtkGeoProjectionSource : public vtkGeoSource
{
public:
  static vtkGeoProjectionSource *New();
  vtkTypeRevisionMacro(vtkGeoProjectionSource,vtkGeoSource);

  vtkGeoProjectionSource();
  ~vtkGeoProjectionSource();

  // Description:
  // Blocking methods for sources with low latency.
  virtual bool FetchRoot(vtkGeoTreeNode* root);
  virtual bool FetchChild(vtkGeoTreeNode* node, int index, vtkGeoTreeNode* child);

  // Description:
  // The projection ID defining the projection.
  vtkGetMacro(Projection, int);
  virtual void SetProjection(int projection);

  // Description:
  // The minimum number of cells per node.
  vtkGetMacro(MinCellsPerNode, int);
  vtkSetMacro(MinCellsPerNode, int);

protected:
  void RefineAndComputeError(vtkGeoTerrainNode* node);

  int Projection;
  int MinCellsPerNode;
  virtual void SetTransform(vtkTransformFilter* transform);
  vtkTransformFilter* Transform;

  vtkMutexLock* TransformLock;

private:
  vtkGeoProjectionSource(const vtkGeoProjectionSource&); // Not implemented
  void operator=(const vtkGeoProjectionSource&); // Not implemented
};

