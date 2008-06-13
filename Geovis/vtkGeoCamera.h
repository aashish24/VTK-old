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

// .NAME vtkGeoCamera - Geo interface to a camera.
// .SECTION Description I wanted to hide the normal vtkCamera API
// so I did not make this a subclass.  The camera is a helper object.
// You can get a pointer to the camera, but it should be treated like
// a const.

// View up of the camera is restricted so there is no roll relative
// to the earth.  I am going to keep view up of the camera orthogonalized to
// avoid the singularity that exists when the camera is pointing straight down.
// In this case, view up is the same as heading.
// 
// The state of the view is specified by the vector:
// (Longitude,Latitude,Distance,Heading,Tilt).
//   Longitude in degrees: (-180->180)
//     Relative to absolute coordinates.
//   Latitude in degrees: (-90->90)
//     Relative to Longitude.
//   Distance in Meters
//     Relative to Longitude and Latitude.
//     above sea level   ???? should we make this from center of earth ????
//                       ???? what about equatorial bulge ????
//   Heading in degrees:  (-180->180)
//     Relative to Logitude and Latitude.
//     0 is north. 
//     90 is east.       ???? what is the standard ????
//     180 is south.
//     -90 is west.  
//   Tilt in degrees: (0->90)
//     Relative to Longitude, Latitude, Distance and Heading.
//  
//   
// Transformation:
//   Post concatenate.
//   All rotations use right hand rule and are around (0,0,0) (earth center).
//   (0,0,0,0,0) is this rectilinear point (0, EarthRadius, 0)
//               pointing (0,0,1), view up (0,1,0).
//   
//   Rotate Tilt       around x axis,             
//   Rotate Heading    around -y axis Center,     
//   Translate EarthRadius in y direction.
//   Rotate Latitude   around x axis by Latitude, 
//   Rotate Longitude  around z axis (earth axis),
//   

// .SECTION See Also
// vtkGeoInteractorStyle vtkCamera 
   
#ifndef __vtkGeoCamera_h
#define __vtkGeoCamera_h

#include "vtkSmartPointer.h" // for SP
#include "vtkCamera.h"
#include "vtkTransform.h" // for SP

class vtkGeoTerrainNode;


class VTK_GEOVIS_EXPORT vtkGeoCamera : public vtkObject
{
public:
  static vtkGeoCamera *New();
  vtkTypeRevisionMacro(vtkGeoCamera, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Longitude is in degrees: (-180->180)
  //   Relative to absolute coordinates.
  //  Rotate Longitude  around z axis (earth axis),
  void SetLongitude(double longitude);
  vtkGetMacro(Longitude,double);

  // Description:
  //  Latitude is in degrees: (-90->90)
  //    Relative to Longitude.
  //  Rotate Latitude   around x axis by Latitude, 
  void SetLatitude(double latitude);
  vtkGetMacro(Latitude,double);

  // Description:
  // Distance is in Meters
  //   Relative to Longitude and Latitude.
  //   above sea level   ???? should we make this from center of earth ????
  //                    ???? what about equatorial bulge ????
  void SetDistance(double Distance);
  vtkGetMacro(Distance,double);

  // Description:
  // Heading is in degrees:  (-180->180)
  //   Relative to Logitude and Latitude.
  //   0 is north. 
  //   90 is east.       ???? what is the standard ????
  //   180 is south.
  //   -90 is west.  
  //  Rotate Heading    around -y axis Center,     
  void SetHeading(double heading);
  vtkGetMacro(Heading,double);

  // Description:
  // Tilt is also know as pitch.
  // Tilt is in degrees: (0->90)
  //   Relative to Longitude, Latitude, and Heading.
  // Rotate Tilt       around x axis,             
  void SetTilt(double tilt);
  vtkGetMacro(Tilt,double);
  
  // Description:
  // This vtk camera is updated to match this geo cameras state.
  // It should be treated as a const and should not be modified.
  vtkCamera* GetVTKCamera() {return this->VTKCamera;}

  // Description:
  // We precompute some values to speed up update of the terrain.
  // Unfortunately, they have to be manually/explicitely updated
  // when the camera or renderer size changes.
  void InitializeNodeAnalysis(int rendererSize[2]);

  // Description:
  // This method estimates how much of the view is covered by the sphere.
  // Returns a value from 0 to 1.
  double GetNodeCoverage(vtkGeoTerrainNode* node);
  
  // Decription:
  // Whether to lock the heading a particular value,
  // or to let the heading "roam free" when performing
  // latitude and longitude changes.
  vtkGetMacro(LockHeading, bool);
  vtkSetMacro(LockHeading, bool);
  vtkBooleanMacro(LockHeading, bool);
  
protected:
  vtkGeoCamera();
  ~vtkGeoCamera();

  void UpdateVTKCamera();
  void UpdateAngleRanges();
  
//BTX
  vtkSmartPointer<vtkCamera> VTKCamera;
  vtkSmartPointer<vtkTransform> Transform;
//ETX
  
  double Longitude;
  double Latitude;
  double Distance;
  double Heading;
  double Tilt;  
  bool   LockHeading;

  // Values precomputed to make updating terrain mode efficient.
  // The vislibility of many terrain nodes is analyzed every render.
  double ForwardNormal[3];
  double RightNormal[3];
  double UpNormal[3];
  double Aspect[2];
  
  // Frustum planes is better than other options for culling spheres.
  double LeftPlaneNormal[3];
  double RightPlaneNormal[3];
  double DownPlaneNormal[3];
  double UpPlaneNormal[3];
  
private:
  vtkGeoCamera(const vtkGeoCamera&);  // Not implemented.
  void operator=(const vtkGeoCamera&);  // Not implemented.
};

#endif
