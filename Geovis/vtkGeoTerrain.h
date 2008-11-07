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
// .NAME vtkGeoTerrain - A 3D terrain model for the globe.
//
// .SECTION Description
// vtkGeoTerrain contains a multi-resolution tree of geometry representing
// the globe. It uses a vtkGeoSource subclass to generate the terrain, such
// as vtkGeoGlobeSource. This source must be set before using the terrain in
// a vtkGeoView. The terrain also contains an AddActors() method which
// will update the set of actors representing the globe given the current
// camera position.

#ifndef __vtkGeoTerrain_h
#define __vtkGeoTerrain_h

#include "vtkObject.h"

class vtkAssembly;
class vtkCollection;
class vtkGeoCamera;
class vtkGeoSource;
class vtkGeoTerrainNode;
class vtkRenderer;

class VTK_GEOVIS_EXPORT vtkGeoTerrain : public vtkObject
{
public:
  static vtkGeoTerrain *New();
  vtkTypeRevisionMacro(vtkGeoTerrain,vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The source used to obtain geometry patches.
  virtual vtkGeoSource* GetSource()
    { return this->GeoSource; }
  virtual void SetSource(vtkGeoSource* source);

  // Description:
  // Save the set of patches up to a given maximum depth.
  void SaveDatabase(const char* path, int depth);

  // Description:
  // Update the actors in an assembly used to render the globe.
  // ren is the current renderer, and imageReps holds the collection of
  // vtkGeoAlignedImageRepresentations that will be blended together to
  // form the image on the globe. camera is the vtkGeoCamera from the
  // vtkGeoView that contains the camera parameters in spherical coordinates.
  void AddActors(
    vtkRenderer* ren,
    vtkAssembly* assembly,
    vtkCollection* imageReps,
    vtkGeoCamera* camera);

protected:
  vtkGeoTerrain();
  ~vtkGeoTerrain();

  virtual void SetGeoSource(vtkGeoSource* source);
  vtkGeoSource* GeoSource;
  vtkGeoTerrainNode* Root;

  // Description:
  // Initialize the terrain with a new source.
  void Initialize();

  // Description:
  // Evaluate whether a node should be refined, coarsened, or remain
  // at the same level for a particular camera location.
  int EvaluateNode(vtkGeoTerrainNode* node, vtkGeoCamera* camera);

  // Description:
  // Print the tree of terrain nodes.
  void PrintTree(ostream & os, vtkIndent indent, vtkGeoTerrainNode* node);

private:
  vtkGeoTerrain(const vtkGeoTerrain&); // Not implemented
  void operator=(const vtkGeoTerrain&); // Not implemented
};

#endif
