/*=========================================================================
  
  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  
Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkLODActor - an actor that supports multiple levels of detail
// .SECTION Description
// vtkLODActor is an actor that stores multiple Levels of Detail and can
// automatically switch between them. It selects which level of detail
// to use based on how much time it has been allocated to render. 
// Currently a very simple method of TotalTime/NumberOfActors is used.
// In the future this should be modified to dynamically allocate the
// rendering time between different actors based on their needs.
// There are three levels of detail by default. The top level is just
// the normal data.  The lowest level of detail is a simple bounding
// box outline of the actor. The middle level of detail is a point
// cloud of a fixed number of points that have been randomly sampled
// from the Mappers input data.  Point attributes are copied over to
// the point cloud.  These two lower levels of detail are accomplished by
// creating instances of a vtkOutlineFilter, vtkGlyph3D, and vtkPointSource.
// Additional levels of detail can be add using the AddLODMapper method.

// .SECTION see also
// vtkActor vtkRenderer

#ifndef __vtkLODActor_h
#define __vtkLODActor_h

#include "vtkActor.h"
#include "vtkMaskPoints.h"
#include "vtkOutlineFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkGlyph3D.h"
#include "vtkPointSource.h"
#include "vtkMapperCollection.h"

class VTK_EXPORT vtkLODActor : public vtkActor
{
public:
  const char *GetClassName() {return "vtkLODActor";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Creates a vtkLODActor with the following defaults: origin(0,0,0) 
  // position=(0,0,0) scale=(1,1,1) visibility=1 pickable=1 dragable=1
  // orientation=(0,0,0). NumberOfCloudPoints is set to 150.
  static vtkLODActor *New() {return new vtkLODActor;};

  // Description:
  // This causes the actor to be rendered. It, in turn, will render the actor's
  // property and then mapper.  
  virtual void Render(vtkRenderer *, vtkMapper *);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Add another level of detail.  They do not have to be in any order
  // of complexity.
  void AddLODMapper(vtkMapper *mapper);

  // Description:
  // Set/Get the number of random points for the point cloud.
  vtkGetMacro(NumberOfCloudPoints,int);
  vtkSetMacro(NumberOfCloudPoints,int);

  // Description:
  // All the mappers for differnt LODs are stored here.
  // The order is not important.
  vtkGetObjectMacro(LODMappers, vtkMapperCollection);

  // Description:
  // When this objects gets modified, this method also modifies the object.
  void Modified();
  
  // Description:
  // Used to construct assembly paths and perform part traversal.
  void BuildPaths(vtkAssemblyPaths *paths, vtkActorCollection *path);
  
protected:
  vtkLODActor();
  ~vtkLODActor();
  vtkLODActor(const vtkLODActor&) {};
  void operator=(const vtkLODActor&) {};

  vtkActor            *Device;
  vtkMapperCollection *LODMappers;

  // stuff for creating our own LOD mappers
  vtkPointSource      *PointSource;
  vtkGlyph3D          *Glyph3D;
  vtkMaskPoints       *MaskPoints;
  vtkOutlineFilter    *OutlineFilter;
  vtkTimeStamp        BuildTime;
  int                 NumberOfCloudPoints;
  vtkPolyDataMapper   *LowMapper;
  vtkPolyDataMapper   *MediumMapper;

  void CreateOwnLODs();
  void UpdateOwnLODs();
  void DeleteOwnLODs();
};

#endif


