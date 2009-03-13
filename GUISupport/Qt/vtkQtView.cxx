/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2009 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkQtView.h"

#include <QWidget>
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkQtView, "$Revision$");

//----------------------------------------------------------------------------
vtkQtView::vtkQtView()
{

}

//----------------------------------------------------------------------------
vtkQtView::~vtkQtView()
{

}

//----------------------------------------------------------------------------
void vtkQtView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

