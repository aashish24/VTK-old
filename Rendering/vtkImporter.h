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
// .NAME vtkImporter - importer abstract class
// .SECTION Description
// vtkImporter is an abstract class that specifies the protocol for
// importing actors, cameras, lights and proeperties into a
// vtkRenderWindow. The following takes place:
// 1) Create a RenderWindow and Renderer if none is provided.
// 2) Open the import file
// 3) Import the Actors
// 4) Import the cameras
// 5) Import the lights
// 6) Import the Properties
// 7) Close the import file
//
// Subclasses optionally implement the ImportActors, ImportCameras,
// ImportLights and ImportProperties methods. An ImportBegin and
// ImportEnd can optionally be provided to perform Importer-specific
// initialization and termination.  The Read method initiates the import
// process. If a RenderWindow is provided, its Renderer will contained
// the imported objects. If the RenderWindow has no Renderer, one is
// created. If no RenderWindow is provided, both a RenderWindow and
// Renderer will be created. Both the RenderWindow and Renderer can be
// accessed using Get methods.

// .SECTION See Also
// vtk3DSImporter vtkExporter

#ifndef __vtkImporter_h
#define __vtkImporter_h

#include <stdio.h>
#include "vtkObject.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkPolyData.h"
#include "vtkActorCollection.h"
#include "vtkLightCollection.h"

class VTK_EXPORT vtkImporter : public vtkObject
{
public:
  static vtkImporter *New() {return new vtkImporter;};
  const char *GetClassName() {return "vtkImporter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the name of the file to read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description
  // Get the renderer that contains the imported actors, cameras and
  // lights.
  vtkGetObjectMacro(Renderer,vtkRenderer);

  // Description
  // Set the vtkRenderWindow to contain the imported actors, cameras and
  // lights, If no vtkRenderWindow is set, one will be created and can be
  // obtained with the GetRenderWindow method. If the vtkRenderWindow has been
  // specified, the first vtkRenderer it has will be used to import the
  // objects. If the vtkRenderWindow has no Renderer, one will be created and
  // can be accessed using GetRenderer.
  vtkSetObjectMacro(RenderWindow,vtkRenderWindow);
  vtkGetObjectMacro(RenderWindow,vtkRenderWindow);

  // Description:
  // Set/Get the computation of normals. If on, imported geometry will
  // be run through vtkPolyDataNormals.
  vtkSetMacro(ComputeNormals,int);
  vtkGetMacro(ComputeNormals,int);
  vtkBooleanMacro(ComputeNormals,int);

  // Description
  // Import the actors, cameras, lights and properties into a vtkRenderWindow.
  void Read();
  void Update() {this->Read();};
  
  // Description:
  // Return the file pointer to the open file.
  FILE *GetFileFD() {return this->FileFD;};

protected:
  vtkImporter();
  ~vtkImporter();
  vtkImporter(const vtkImporter&) {};
  void operator=(const vtkImporter&) {};

  int OpenImportFile();
  void CloseImportFile();
  virtual int ImportBegin () {return 1;};
  virtual void ImportActors (vtkRenderer *vtkNotUsed(renderer)) {};
  virtual void ImportCameras (vtkRenderer *vtkNotUsed(renderer)) {};
  virtual void ImportLights (vtkRenderer *vtkNotUsed(renderer)) {};
  virtual void ImportProperties (vtkRenderer *vtkNotUsed(renderer)) {};
  virtual void ImportEnd () {};

  char *FileName;
  FILE *FileFD;
  vtkRenderer *Renderer;
  vtkRenderWindow *RenderWindow;
  int ComputeNormals;

};

#endif




