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
// .NAME Test of vtkMetaIO / MetaImage
// .SECTION Description
//

#include "vtkImageReader2Factory.h"
#include "vtkImageReader2.h"
#include "vtkMetaImageWriter.h"
#include "vtkOutputWindow.h"
#include "vtkObjectFactory.h"
#include "vtkDataObject.h"
#include "vtkImageData.h"
#include "vtkImageMathematics.h"
#include "vtkSmartPointer.h"


int TestImageReader2Factory(int argc, char *argv[])
{
  vtkOutputWindow::GetInstance()->PromptUserOn();
  if ( argc <= 1 )
    {
    cout << "Usage: " << argv[0] << " <meta image file>" << endl;
    return 1;
    }

  int error = 0;

  vtkSmartPointer< vtkImageReader2Factory > imageFactory = 
    vtkSmartPointer< vtkImageReader2Factory >::New();

  vtkSmartPointer< vtkImageReader2 > imageReader = 
    imageFactory->CreateImageReader2( argv[1] );  

  imageReader->SetFileName( argv[1] );
  imageReader->Update();

  vtkSmartPointer< vtkImageData > image = imageReader->GetOutput();



  cout << "Success!  Error = " << error << endl;

  return 0;
}
