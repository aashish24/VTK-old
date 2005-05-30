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
#ifndef __vtkXMLCInterface_h
#define __vtkXMLCInterface_h

#include <stdio.h>  /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif /*cplusplus*/


/*
 * do the memory allocation
 */
void vtkXML_Initialize();

/*
 * Set the full path to the file
 */
void vtkXML_SetFileName(const char* filename);

/*
 * datatype is defined in vtkSystemIncludes
 * array is a pointer to the memory containing the points
 * size is the number of points (array is of lenght := size*3)
 */
void vtkXML_SetPoints   (int datatype, void* array, size_t size);

/*
 * datatype is defined in vtkSystemIncludes
 * array is a pointer to the memory containing the point data
 * size is the number of points (array is of lenght := size*numComp)
 */
void vtkXML_SetPointData(int datatype, void* array, size_t size, int numComp);

/*
 * datatype is defined in vtkSystemIncludes
 * array is a pointer to the memory containing the cell data
 * size is the number of celldata (array is of lenght := size*numComp)
 */
void vtkXML_SetCellData(int datatype, void* array, size_t size, int numComp);

/*
 */
void vtkXML_SetCellArray(int* array, int ncells, size_t size);

/*
 * Set the maximum number of time steps the file will have, the program can quit
 * earlier but can never write more time steps than the maximum
 */
void vtkXML_SetNumberOfTimeSteps(int n);

/*
 * Prepare for writting (basically write the XML file header)
 */
void vtkXML_Start();

/*
 * Write a new time step
 */
void vtkXML_WriteNextTime(double t);

/*
 * Prepare for writting (basically write the XML file footer)
 */
void vtkXML_Stop();

#ifdef __cplusplus
};
#endif /*cplusplus*/

#endif
