/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME vtkPicker - select an actor by shooting a ray into graphics window
// .SECTION Description
// vtkPicker is used to select actors by shooting a ray into graphics window
// and intersecting with actor's bounding box. The ray is defined from a 
// point defined in window (or pixel) coordinates, and a point located from 
// the camera's position.
//    vtkPicker may return more than one actor, since more than one bounding 
// box may be intersected. VlPicker returns the list of actors that were hit, 
// the pick coordinates in world and untransformed mapper space, and the 
// actor and mapper that are "closest" to the camera. The closest actor is 
// the one whose center point (i.e., center of bounding box) projected on the
// the ray is closest to the camera.
// .SECTION Caveats
// vtkPicker and its subclasses will not pick actors that are "unpickable" 
// (see vtkActor) or are fully transparent.
// .SECTION See Also
// vtkPicker is used for quick picking. If you desire to pick points
// or cells, use the subclass vtkPointPicker or vtkCellPicker, respectively.

#ifndef __vtkPicker_h
#define __vtkPicker_h

#include "Object.hh"
#include "Renderer.hh"
#include "Actor.hh"
#include "ActorC.hh"
#include "Mapper.hh"
#include "Trans.hh"

class vtkPicker : public vtkObject
{
public:
  vtkPicker();
  ~vtkPicker() {};
  char *GetClassName() {return "vtkPicker";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the renderer in which pick event occurred.
  vtkGetObjectMacro(Renderer,vtkRenderer);

  // Description:
  // Get the selection point in screen (pixel) coordinates. The third
  // value is related to z-buffer depth. (Normally should be =0).
  vtkGetVectorMacro(SelectionPoint,float,3);

  // Description:
  // Specify tolerance for performing pick operation. Tolerance is specified
  // as fraction of rendering window size. (Rendering window size is measured
  // across diagonal).
  vtkSetMacro(Tolerance,float);
  vtkGetMacro(Tolerance,float);

  // Description:
  // Return position in global coordinates of pick point.
  vtkGetVectorMacro(PickPosition,float,3);

  // Description:
  // Return position in mapper (i.e., non-transformed) coordinates of 
  // pick point.
  vtkGetVectorMacro(MapperPosition,float,3);

  // Description:
  // Return actor that was picked.
  vtkGetObjectMacro(Actor,vtkActor);

  // Description:
  // Return mapper that was picked.
  vtkGetObjectMacro(Mapper,vtkMapper);

  // Description:
  // Get a pointer to the dataset that was picked. If nothing was picked then
  // NULL is returned.
  vtkGetObjectMacro(DataSet,vtkDataSet);

  vtkActorCollection *GetActors();

  int Pick(float selectionX, float selectionY, float selectionZ, 
           vtkRenderer *renderer);  
  int Pick(float selectionPt[3], vtkRenderer *renderer);  

protected:
  void MarkPicked(vtkActor *a, vtkMapper *m, float tMin, float mapperPos[3]);
  virtual void IntersectWithLine(float p1[3], float p2[3], float tol, vtkActor *a, vtkMapper *m);
  virtual void Initialize();

  vtkRenderer *Renderer; //pick occurred in this renderer's viewport
  float SelectionPoint[3]; //selection point in window (pixel) coordinates
  float Tolerance; //tolerance for computation (% of window)
  float PickPosition[3]; //selection point in world coordinates
  float MapperPosition[3]; //selection point in untransformed coordinates
  vtkActor *Actor; //selected actor
  vtkMapper *Mapper; //selected mapper
  vtkDataSet *DataSet; //selected dataset
  float GlobalTMin; //parametric coordinate along pick ray where hit occured
  vtkTransform Transform; //use to perform ray transformation
  vtkActorCollection Actors; //candidate actors (based on bounding box)
};

inline vtkActorCollection* vtkPicker::GetActors() {return &(this->Actors);}

inline int vtkPicker::Pick(float selectionPt[3], vtkRenderer *renderer)
{
  return this->Pick(selectionPt[0], selectionPt[1], selectionPt[2], renderer);
}

#endif


