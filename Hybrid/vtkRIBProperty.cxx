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
#include "vtkRIBProperty.h"

vtkRIBProperty::vtkRIBProperty ()
{
  this->Declarations = NULL;
  this->Parameters = NULL;
  this->SurfaceShader = new char[strlen("plastic") + 1];
  strcpy (this->SurfaceShader, "plastic");
  this->DisplacementShader = NULL;
  // create a vtkProperty that can be rendered
  this->Property = vtkProperty::New ();
}

vtkRIBProperty::~vtkRIBProperty()
{
  if (this->SurfaceShader)
    {
    delete [] this->SurfaceShader;
    }
  if (this->DisplacementShader)
    {
    delete [] this->DisplacementShader;
    }
  if (this->Declarations)
    {
    delete [] this->Declarations;
    }
  if (this->Property)
    {
    this->Property->Delete ();
    }
  if (this->Parameters)
    {
    delete [] this->Parameters;
    }
}

void vtkRIBProperty::Render(vtkActor *anActor, vtkRenderer *ren)
{
  int ref;
  
  // Copy this property's ivars into the property to be rendered
  ref = this->Property->GetReferenceCount();
  this->Property->DeepCopy(this);
  this->Property->SetDeleteMethod(NULL);
  this->Property->SetReferenceCount(ref);
  
  // Render the property
  this->Property->Render (anActor, ren);
}

void vtkRIBProperty::SetVariable (char *variable, char *value)
{
  if (this->Declarations)
    {
    delete [] this->Declarations;
    }

  // format of line is: Declare "variable" "type"\n
  this->Declarations = new char [strlen ("Declare ") +
	                      strlen (variable) +
			      strlen (value) + 
			      8];

  sprintf (this->Declarations, "Declare \"%s\" \"%s\"\n", variable, value);
  this->Modified ();
}

void vtkRIBProperty::AddVariable (char *variable, char *value)
{
  if (this->Declarations == NULL)
    {
    this->SetVariable (variable, value);
    }
  else
    {
    char *newVariable = new char [strlen ("Declare ") +
	                          strlen (variable) +
		   	          strlen (value) + 
			          8];

    sprintf (newVariable, "Declare \"%s\" \"%s\"\n", variable, value);
    char *oldDeclarations = this->Declarations;

    this->Declarations = new char [strlen (oldDeclarations) + strlen (newVariable) + 1];
    strcpy (this->Declarations, oldDeclarations);
    strcat (this->Declarations, newVariable);
    delete [] oldDeclarations;
    delete [] newVariable;
    this->Modified ();
    }
}

void vtkRIBProperty::SetParameter (char *parameter, char *value)
{
  if (this->Parameters)
    {
    delete [] this->Parameters;
    }

  // format of line is: "parameter" "value"
  this->Parameters = new char [strlen (parameter) +
			      strlen (value) + 
			      7];

  sprintf (this->Parameters, " \"%s\" [%s]", parameter, value);
  this->Modified ();
}

void vtkRIBProperty::AddParameter (char *Parameter, char *value)
{
  if (this->Parameters == NULL)
    {
    this->SetParameter (Parameter, value);
    }
  else
    {
    char *newParameter = new char [strlen (Parameter) +
		   	          strlen (value) + 
			          7];

    sprintf (newParameter, " \"%s\" [%s]", Parameter, value);
    char *oldParameters = this->Parameters;

    this->Parameters = new char [strlen (oldParameters) + strlen (newParameter) + 1];
    strcpy (this->Parameters, oldParameters);
    strcat (this->Parameters, newParameter);
    delete [] oldParameters;
    delete [] newParameter;
    this->Modified ();
    }
}

char *vtkRIBProperty::GetParameters ()
{
  return this->Parameters;
}

char *vtkRIBProperty::GetDeclarations ()
{
  return this->Declarations;
}

void vtkRIBProperty::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkProperty::PrintSelf(os,indent);
 
  if (this->SurfaceShader)
    {
    os << indent << "SurfaceShader: " << this->SurfaceShader << "\n";
    }
  else
    {
    os << indent << "SurfaceShader: (none)\n";
    }
  if (this->DisplacementShader)
    {
    os << indent << "DisplacementShader: " << this->DisplacementShader << "\n";
    }
  else
    {
    os << indent << "DisplacementShader: (none)\n";
    }
  if (this->Declarations)
    {
    os << indent << "Declarations: " << this->Declarations;
    }
  else
    {
    os << indent << "Declarations: (none)\n";
    }
  if (this->Parameters)
    {
    os << indent << "Parameters: " << this->Parameters;
    }
  else
    {
    os << indent << "Parameters: (none)\n";
    }

}

