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
#ifndef __vlActor_hh
#define __vlActor_hh

#include "Property.hh"
#include "Mapper.hh"

class vlRenderer;
class vlMapper;

class vlActor : public vlObject
{
 public:
  vlActor();
  ~vlActor();
  void Render(vlRenderer *ren);
  int GetVisibility();
  void SetVisibility(int flag);
  void GetCompositeMatrix(float mat[4][4]);
  void SetMapper(vlMapper *m);
  vlMapper *GetMapper();
  vlProperty *Property; 

 protected:
  vlMapper *Mapper;
  float Origin[3];
  float Position[3];
  float Orientation[3];
  float Scale[3];
  int   Visibility;
};

#endif

