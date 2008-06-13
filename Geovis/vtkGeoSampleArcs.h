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
// .NAME vtkGeoArcs - layout graph edges on a globe as arcs.
//
// .SECTION Description

// .SECTION Thanks

#ifndef __vtkGeoSampleArcs_h
#define __vtkGeoSampleArcs_h

#include "vtkPolyDataAlgorithm.h"

class VTK_GEOVIS_EXPORT vtkGeoSampleArcs : public vtkPolyDataAlgorithm 
{
public:
  static vtkGeoSampleArcs *New();

  vtkTypeRevisionMacro(vtkGeoSampleArcs,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The base radius used to determine the earth's surface.
  // Default is the earth's radius in meters.
  // TODO: Change this to take in a vtkGeoTerrain to get altitude.
  vtkSetMacro(GlobeRadius, double);
  vtkGetMacro(GlobeRadius, double);

  // Description:
  // The maximum distance, in meters, between adjacent points.
  vtkSetMacro(MaximumDistanceMeters, double);
  vtkGetMacro(MaximumDistanceMeters, double);
  
protected:
  vtkGeoSampleArcs();
  ~vtkGeoSampleArcs();

  // Description:
  // Convert the vtkGraph into vtkPolyData.
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
  double GlobeRadius;
  double MaximumDistanceMeters;
  
private:
  vtkGeoSampleArcs(const vtkGeoSampleArcs&);  // Not implemented.
  void operator=(const vtkGeoSampleArcs&);  // Not implemented.
};

#endif
