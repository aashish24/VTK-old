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

#include "vtkConfigure.h"

#if defined(_MSC_VER) && !defined(VTK_DISPLAY_WIN32_WARNINGS)
#pragma warning ( disable : 4275 )
#endif

#include "vtkImageIterator.txx"
#include "vtkImageProgressIterator.txx"

#ifndef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION

template class VTK_COMMON_EXPORT vtkImageProgressIterator<char>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<int>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<long>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<short>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<float>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<double>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<unsigned long>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<unsigned short>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<unsigned char>;
template class VTK_COMMON_EXPORT vtkImageProgressIterator<unsigned int>;

#endif

