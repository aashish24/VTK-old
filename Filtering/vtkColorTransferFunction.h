/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkColorTransferFunction - Defines a transfer function for mapping a property to an RGB color value.

// .SECTION Description
// vtkColorTransferFunction encapsulates three vtkPiecewiseFunction instances
// to provide a full RGB transfer function.

// .SECTION see also
// vtkPiecewiseFunction

#ifndef __vtkColorTransferFunction_h
#define __vtkColorTransferFunction_h

#include "vtkScalarsToColors.h"

class vtkPiecewiseFunction;

#define VTK_CTF_RGB   0
#define VTK_CTF_HSV   1

class VTK_FILTERING_EXPORT vtkColorTransferFunction : public vtkScalarsToColors 
{
public:
  static vtkColorTransferFunction *New();
  vtkTypeRevisionMacro(vtkColorTransferFunction,vtkScalarsToColors);
  void DeepCopy( vtkColorTransferFunction *f );

  // Description:
  // Print method for vtkColorTransferFunction
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // How many points are there defining this function?
  int GetSize() {return this->NumberOfPoints;};
  
  // Description:
  // Add a point to the function defined in RGB or HSV
  void AddRGBPoint( float x, float r, float g, float b );
  void AddHSVPoint( float x, float h, float s, float v );

  // Description:
  // Add two points to the function and remove all the points 
  // between them
  void AddRGBSegment( float x1, float r1, float g1, float b1, 
                      float x2, float r2, float g2, float b2 );
  void AddHSVSegment( float x1, float h1, float s1, float v1, 
                      float x2, float h2, float s2, float v2 );
  
  // Description:
  // Remove a point / remove all points
  void RemovePoint( float x );
  void RemoveAllPoints();

  // Description:
  // Returns an RGB color for the specified scalar value (from
  // vtkScalarsToColors)
  float *GetColor(float x) { 
    return vtkScalarsToColors::GetColor(x); }
  void GetColor(float x, float rgb[3]);

  // Description:
  // Get the color components individually.
  float GetRedValue( float x );
  float GetGreenValue( float x );
  float GetBlueValue( float x );

  // Description:
  // Map one value through the lookup table.
  virtual unsigned char *MapValue(float v);

  // Description:
  // Returns min and max position of all function points.
  vtkGetVector2Macro( Range, float );

  // Description:
  // Fills in a table of n function values between x1 and x2
  void GetTable( float x1, float x2, int n, float* table );
  const unsigned char *GetTable( float x1, float x2, int n);

  // Description:
  // Construct a color transfer function from a table. Function range is
  // is set to [x1, x2], each function size is set to size, and function 
  // points are regularly spaced between x1 and x2. Parameter "table" is 
  // assumed to be a block of memory of size [3*size]
  void BuildFunctionFromTable( float x1, float x2, int size, float *table);

  // Description:
  // Sets and gets the clamping value for this transfer function.
  vtkSetClampMacro( Clamping, int, 0, 1 );
  vtkGetMacro( Clamping, int );
  vtkBooleanMacro( Clamping, int );
  
  // Description:
  // How should we interpolate - in RGB, or HSV
  vtkSetClampMacro( ColorSpace, int, VTK_CTF_RGB, VTK_CTF_HSV );
  void SetColorSpaceToRGB(){this->SetColorSpace(VTK_CTF_RGB);};
  void SetColorSpaceToHSV(){this->SetColorSpace(VTK_CTF_HSV);};
  vtkGetMacro( ColorSpace, int );
    
  // Description:
  // Returns a list of all nodes
  float *GetDataPointer() {return this->Function;};

  // Description:
  // map a set of scalars through the lookup table
  virtual void MapScalarsThroughTable2(void *input, unsigned char *output,
                                     int inputDataType, int numberOfValues,
                                     int inputIncrement, int outputIncrement);
  
protected:
  vtkColorTransferFunction();
  ~vtkColorTransferFunction();

  // Determines the function value outside of defined points
  // Zero = always return 0.0 outside of defined points
  // One  = clamp to the lowest value below defined points and
  //        highest value above defined points
  int Clamping;

  // The color space in which interpolation is performed
  int ColorSpace;
  
  // The color function
  float       *Function;
  int         FunctionSize;
  int         NumberOfPoints;

  // conversion methods
  void RGBToHSV( float r, float g, float b, float &h, float &s, float &v );
  void HSVToRGB( float h, float s, float v, float &r, float &g, float &b );
  
  // An evaluated color (0 to 255 RGBA A=255)
  unsigned char UnsignedCharRGBAValue[4];

  // The min and max point locations for all three transfer functions
  float Range[2]; 

  // Transfer functions for each color component
  // Remove after corresponding depricated methods are removed
  vtkPiecewiseFunction  *Red;
  vtkPiecewiseFunction  *Green;
  vtkPiecewiseFunction  *Blue;
  vtkTimeStamp BuildTime;
  unsigned char *Table;
  int TableSize;
  
  // Description:
  // Set the range of scalars being mapped. The set has no functionality
  // in this subclass of vtkScalarsToColors.
  virtual void SetRange(float, float) {};
  void SetRange(float rng[2]) {this->SetRange(rng[0],rng[1]);};


private:
  vtkColorTransferFunction(const vtkColorTransferFunction&);  // Not implemented.
  void operator=(const vtkColorTransferFunction&);  // Not implemented.
};

#endif


