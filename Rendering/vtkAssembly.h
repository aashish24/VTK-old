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
// .NAME vtkAssembly - create hierarchies of actors
// .SECTION Description
// vtkAssembly is an object that groups actors and other assemblies into
// a tree-like hierarchy. The actors and assemblies can then be transformed
// together by transforming just the root assembly of the hierarchy.
//
// A vtkAssembly object can be used in place of an vtkActor since it is a 
// subclass of vtkActor. The difference is that vtkAssembly maintains a list
// of actor instances (its "parts") that form the assembly. Then, any 
// operation that modifies the parent assembly will modify all its parts.
// Note that this process is recursive: you can create groups consisting
// of assemblies and/or actors to arbitrary depth.
//
// Actor's (or assemblies) that compose an assembly need not be added to 
// a renderer's list of actors, as long as the parent assembly is in the
// list of actors. This is because they are automatically renderered 
// during the hierarchical traversal process.
//
// Since a vtkAssembly object is a derived class of vtkActor, it has
// properties and possibly a mapper. During the rendering process, if a
// mapper is associated with the assembly, it is rendered with these
// properties. Otherwise, the properties have no effect (i.e., on the
// children of the assembly).

// .SECTION Caveats
// Collections of assemblies are slower to render than an equivalent list
// of actors. This is because to support arbitrary nesting of assemblies, 
// the state of the assemblies (i.e., transformation matrices) must
// be propagated through the assembly hierarchy. 
//
// Assemblies can consist of hierarchies of assemblies, where one actor or
// assembly used in one hierarchy is also used in other hierarchies. However, 
// make that there are no cycles (e.g., parent->child->parent), this will
// cause program failure.
 
// .SECTION See Also
// vtkActor vtkTransform vtkMapper vtkPolyDataMapper

#ifndef __vtkAssembly_h
#define __vtkAssembly_h

#include "vtkActor.h"

class vtkAssemblyPaths;

class VTK_EXPORT vtkAssembly : public vtkActor
{
public:
  vtkAssembly();
  ~vtkAssembly();
  static vtkAssembly *New() {return new vtkAssembly;};
  const char *GetClassName() {return "vtkAssembly";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add a part to the list of parts.
  void AddPart(vtkActor *);

  // Description:
  // Remove a part from the list of parts,
  void RemovePart(vtkActor *);

  // Description:
  // Return the parts of this asembly.
  vtkActorCollection *GetParts();

  // Description:
  // Render this assembly and all its parts. 
  // The rendering process is recursive.
  // Note that a mapper need not be defined. If not defined, then no geometry 
  // will be drawn for this assembly. This allows you to create "logical"
  // assemblies; that is, assemblies that only serve to group and transform
  // its parts.
  void RenderOpaqueGeometry(vtkViewport *ren);
  void RenderTranslucentGeometry(vtkViewport *ren);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter RenderWindow could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkRenderWindow *);

  // Description:
  // Methods to traverse the parts of an assembly. Each part (starting from
  // the root) will appear properly transformed and with the correct
  // properties (depending upon the ApplyProperty and ApplyTransform ivars).
  // Note that the part appears as an actor. These methods should be contrasted
  // to those that traverse the list of parts using GetParts(). 
  // GetParts() returns
  // a list of children of this assembly, not necessarily with the correct
  // transformation or properties. To use these methods - first invoke 
  // InitPartTraversal() followed by repeated calls to GetNextPart(). 
  // GetNextPart() returns a NULL pointer when the list is exhausted.
  void InitPartTraversal();
  vtkActor *GetNextPart();
  int GetNumberOfParts();

  // Description:
  // Build assembly paths from this current assembly. Paths consist of
  // an ordered sequence of actors, with transformations properly concatenated.
  void BuildPaths(vtkAssemblyPaths *paths, vtkActorCollection *path);

  // Description:
  // Recursively apply properties to parts.
  void ApplyProperties(); 

  // Description:
  // Get the bounds for the assembly as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
  float *GetBounds();

  // Description:
  // Override default GetMTime method to also consider all of the
  // assembly's parts.
  unsigned long int GetMTime();

  // Description:
  // For legacy compatibility. Do not use.
  vtkAssembly &operator=(const vtkAssembly& assembly);

protected:
  vtkActorCollection *Parts;

  // stuff that follows is used to build the assembly hierarchy
  vtkAssemblyPaths *Paths;
  vtkTimeStamp PathTime;

  void UpdatePaths(); //apply transformations and properties recursively
  void DeletePaths(); //delete the paths

};

// Description:
// Get the list of parts for this assembly.
inline vtkActorCollection *vtkAssembly::GetParts() {return this->Parts;}

#endif




