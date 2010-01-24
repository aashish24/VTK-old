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
// .NAME vtkCellPicker - ray-cast cell picker for all kinds of Prop3Ds
// .SECTION Description
// vtkCellPicker will shoot a ray into a 3D scene and return information
// about the first object that the ray hits.  It works for all Prop3Ds.
// For vtkVolume objects, it shoots a ray into the volume and returns
// the point where the ray intersects an isosurface of a chosen opacity.
// For vtkImageActor objects, it intersects the ray with the displayed
// slice. For vtkActor objects, it intersects the actor's polygons.
// If the object's mapper has ClippingPlanes, then it takes the clipping
// into account, and will return the Id of the clipping plane that was
// intersected.
// For all prop types, it returns point and cell information, plus the
// normal of the surface that was intersected at the pick position.  For
// volumes and images, it also returns (i,j,k) coordinates for the point
// and the cell that were picked.  
//
// .SECTION See Also
// vtkPicker vtkPointPicker vtkVolumePicker
//
// .SECTION Thanks
// This class was contributed to VTK by David Gobbi on behalf of Atamai Inc.,
// as an enhancement to the original vtkCellPicker.

#ifndef __vtkCellPicker_h
#define __vtkCellPicker_h

#include "vtkPicker.h"

class vtkMapper;
class vtkTexture;
class vtkAbstractVolumeMapper;
class vtkImageActor;
class vtkPlaneCollection;
class vtkPiecewiseFunction;
class vtkDataArray;
class vtkDoubleArray;
class vtkCell;
class vtkGenericCell;
class vtkImageData;
class vtkAbstractCellLocator;
class vtkCollection;

class VTK_RENDERING_EXPORT vtkCellPicker : public vtkPicker
{
public:
  static vtkCellPicker *New();
  vtkTypeRevisionMacro(vtkCellPicker, vtkPicker);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Perform pick operation with selection point provided. Normally the 
  // first two values are the (x,y) pixel coordinates for the pick, and
  // the third value is z=0. The return value will be non-zero if
  // something was successfully picked.
  virtual int Pick(double selectionX, double selectionY, double selectionZ, 
                   vtkRenderer *renderer);  

  // Description:
  // Add a locator for one of the data sets that will be included in the
  // scene.  This can dramatically increase performance for complex
  // objects.  If you try to add the same locator twice, the second addition
  // will be ignored.  You must either build the locator before doing the
  // pick, or turn on LazyEvaluation in the locator.
  void AddLocator(vtkAbstractCellLocator *locator);

  // Description:
  // Remove a locator that was previously added.  If you try to remove a
  // nonexistent locator, then nothing will happen and no errors will be
  // raised.
  void RemoveLocator(vtkAbstractCellLocator *locator);

  // Description:
  // Remove all locators associated with this picker.
  void RemoveAllLocators();

  // Description:
  // Set the opacity isovalue to use for defining volume surfaces.  The
  // pick will occur at the location along the pick ray where the product
  // of the scalar opacity and gradient opacity is equal to this isovalue.
  vtkSetMacro(VolumeOpacityIsovalue, double);
  vtkGetMacro(VolumeOpacityIsovalue, double);

  // Description:
  // Ignore the gradient opacity function when computing the opacity
  // isovalue.  This parameter is only relevant to volume picking and
  // is on by default.
  vtkSetMacro(IgnoreGradientOpacity, int);
  vtkBooleanMacro(IgnoreGradientOpacity, int);
  vtkGetMacro(IgnoreGradientOpacity, int);

  // Description:
  // Set whether to pick the clipping planes of props that have them.
  // If this is set, then the pick will be done on the clipping planes
  // rather than on the data. The GetClippingPlaneId() method will return
  // the index of the clipping plane of the vtkProp3D that was picked.
  // The picking of vtkImageActors is not influenced by this setting,
  // since they have no clipping planes.
  vtkSetMacro(PickClippingPlanes, int);
  vtkBooleanMacro(PickClippingPlanes, int);
  vtkGetMacro(PickClippingPlanes, int);

  // Description:
  // Get the index of the clipping plane that was intersected during
  // the pick.  This will be set regardless of whether PickClippingPlanes
  // is on.  The result will be -1 if the Prop3D that was picked
  // has no clipping planes, or if the ray didn't intersect the planes.
  vtkGetMacro(ClippingPlaneId, int);

  // Description:
  // Return the normal of the picked surface at the PickPosition.  If no
  // surface was picked, then a vector pointing back at the camera is
  // returned.
  vtkGetVectorMacro(PickNormal, double, 3);

  // Description:
  // Return the normal of the surface at the PickPosition in mapper
  // coordinates.  The result is undefined if no prop was picked.
  vtkGetVector3Macro(MapperNormal, double);

  // Description:
  // Get the structured coordinates of the point at the PickPosition.
  // Only valid for image actors and volumes with vtkImageData.
  vtkGetVector3Macro(PointIJK, int);

  // Description:
  // Get the structured coordinates of the cell at the PickPosition.
  // Only valid for image actors and volumes with vtkImageData.
  // Combine this with the PCoords to get the position within the cell.
  vtkGetVector3Macro(CellIJK, int);

  // Description:
  // Get the id of the picked point. If PointId = -1, nothing was picked.
  // This point will be a member of any cell that is picked.
  vtkGetMacro(PointId, vtkIdType);

  // Description:
  // Get the id of the picked cell. If CellId = -1, nothing was picked.
  vtkGetMacro(CellId, vtkIdType);

  // Description:
  // Get the subId of the picked cell. This is useful, for example, if
  // the data is made of triangle strips. If SubId = -1, nothing was picked.
  vtkGetMacro(SubId, int);

  // Description:
  // Get the parametric coordinates of the picked cell. Only valid if
  // a prop was picked.  The PCoords can be used to interpolate data
  // values within the cell.
  vtkGetVector3Macro(PCoords, double);

  // Description:
  // Get the texture that was picked.  This will be null unless the picked
  // prop has a texture.
  vtkTexture *GetTexture() { return this->Texture; };

  // Description:
  // If this is "On" and if the picked prop has a texture, then the data
  // returned by GetDataSet() will be the texture's data instead of the
  // mapper's data.  The GetPointId(), GetCellId(), GetPCoords() etc. will
  // all return information for use with the texture's data.  If the picked
  // prop does not have any texture, then GetDataSet() will return the
  // mapper's data instead and GetPointId() etc. will return information
  // related to the mapper's data.  The default value of PickTextureData
  // is "Off".
  vtkSetMacro(PickTextureData, int);
  vtkBooleanMacro(PickTextureData, int);
  vtkGetMacro(PickTextureData, int);

protected:
  vtkCellPicker();
  ~vtkCellPicker();

  void Initialize();

  virtual void ResetPickInfo();

  virtual double IntersectWithLine(double p1[3], double p2[3], double tol, 
                                  vtkAssemblyPath *path, vtkProp3D *p, 
                                  vtkAbstractMapper3D *m);

  virtual double IntersectActorWithLine(const double p1[3], const double p2[3],
                                        double t1, double t2, double tol, 
                                        vtkProp3D *prop, vtkMapper *mapper);

  virtual double IntersectVolumeWithLine(const double p1[3],
                                         const double p2[3],
                                         double t1, double t2,
                                         vtkProp3D *prop, 
                                         vtkAbstractVolumeMapper *mapper);

  virtual double IntersectImageActorWithLine(const double p1[3],
                                             const double p2[3],
                                             double t1, double t2,
                                             vtkImageActor *imageActor);

  static int ClipLineWithPlanes(vtkPlaneCollection *planes,
                                const double p1[3], const double p2[3],
                                double &t1, double &t2, int& planeId);

  static int ClipLineWithExtent(const int extent[6],
                                const double x1[3], const double x2[3],
                                double &t1, double &t2, int &planeId);

  static int ComputeSurfaceNormal(vtkDataSet *data, vtkCell *cell,
                                  const double *weights, double normal[3]);

  static int ComputeSurfaceTCoord(vtkDataSet *data, vtkCell *cell,
                                  const double *weights, double tcoord[3]);

  static void TriangleFromStrip(vtkGenericCell *cell, int subId);

  void SetImageDataPickInfo(const double x[3], const int extent[6]);

  double ComputeVolumeOpacity(const int xi[3], const double pcoords[3],
                              vtkImageData *data, vtkDataArray *scalars,
                              vtkPiecewiseFunction *scalarOpacity,
                              vtkPiecewiseFunction *gradientOpacity);

  vtkCollection *Locators;

  double VolumeOpacityIsovalue;
  int IgnoreGradientOpacity;
  int PickClippingPlanes;
  int ClippingPlaneId;

  vtkIdType PointId;
  vtkIdType CellId;
  int SubId;
  double PCoords[3];

  int PointIJK[3];
  int CellIJK[3];

  double PickNormal[3];
  double MapperNormal[3];

  vtkTexture *Texture;
  int PickTextureData;

private:
  void ResetCellPickerInfo();

  vtkGenericCell *Cell; //used to accelerate picking
  vtkDoubleArray *Gradients; //used in volume picking
  
private:
  vtkCellPicker(const vtkCellPicker&);  // Not implemented.
  void operator=(const vtkCellPicker&);  // Not implemented.
};

#endif


