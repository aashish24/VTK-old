/*=========================================================================

  Program:   OSCAR 
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Description:
---------------------------------------------------------------------------
This file is part of the vis library

- Ken Martin

=========================================================================*/
#ifndef __vlRenderer_hh
#define __vlRenderer_hh

#include "Object.hh"
#include "LightC.hh"
#include "Camera.hh"
#include "ActorC.hh"
#include "GeomPrim.hh"

class vlRenderWindow;

class vlRenderer : public vlObject
{
protected:
  vlCamera *ActiveCamera;
  vlLightCollection Lights;
  vlActorCollection Actors;
  float Ambient[3];  
  float Background[3];  
  int BackLight;
  vlRenderWindow *RenderWindow;
  float DisplayPoint[3];
  float ViewPoint[3];
  float Viewport[4];
  int   Erase;
  float Aspect[2];
  int StereoRender;

public:
  vlRenderer();
  char *GetClassName() {return "vlRenderer";};
  void AddLights(vlLight *);
  void AddActors(vlActor *);
  void SetActiveCamera(vlCamera *);

  vlSetVector3Macro(Background,float);
  vlGetVectorMacro(Background,float);

  vlSetVector2Macro(Aspect,float);
  vlGetVectorMacro(Aspect,float);

  vlSetVector3Macro(Ambient,float);
  vlGetVectorMacro(Ambient,float);

  vlSetMacro(BackLight,int);
  vlGetMacro(BackLight,int);
  vlBooleanMacro(BackLight,int);

  vlSetMacro(Erase,int);
  vlGetMacro(Erase,int);
  vlBooleanMacro(Erase,int);

  vlGetMacro(StereoRender,int);

  virtual void Render() = 0;
  virtual vlGeometryPrimitive *GetPrimitive(char *) = 0;
  
  virtual int UpdateActors(void) = 0;
  virtual int UpdateCameras(void) = 0;
  virtual int UpdateLights(void) = 0;

  void DoCameras();
  void DoLights();
  void DoActors();

  void SetRenderWindow(vlRenderWindow *);
  
  vlSetVector3Macro(DisplayPoint,float);
  vlGetVectorMacro(DisplayPoint,float);

  vlSetVector3Macro(ViewPoint,float);
  vlGetVectorMacro(ViewPoint,float);

  vlSetVector4Macro(Viewport,float);
  vlGetVectorMacro(Viewport,float);

  void DisplayToView();
  void ViewToDisplay();
};

#endif
