/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME vtkStreamer - abstract object implements integration of massless particle through vector field
// .SECTION Description
// vtkStreamer is a filter that integrates a massless particle through a vector
// field. The integration is performed using 2cnd order Runge-Kutta method. 
// vtkStreamer often serves as a base class for other classes that perform 
// numerical integration through a vector field (e.g., vtkStreamLine).
//    Note that vtkStreamer can integrate both forward and backward in time, or 
// in both directions. The length of the streamer time) is controlled by 
// specifying an elapsed time. (The elapsed time is the time each particle 
// travels). Otherwise, the integration terminates after exiting the dataset.
//    vtkStreamer integrates through any type of dataset. Thus if the dataset
// contains 2D cells such as polygons or triangles, the integration is
// constrained to lie on the surface defined by the 2D cells.
//    The starting point of streamers may be defined in three different ways.
// Starting from global x-y-z "position" allows you to start a single streamer
// at a specified x-y-z coordinate. Starting from "location" allows you to 
// start at a specified cell, subId, and parametric coordinate. Finally, you 
// may specify a source object to start multiple streamers. If you start 
// streamers using a source object, for each point (that is inside the dataset)
// a streamer is created.
//    vtkStreamer implements the Execute() method that its superclass vtkFilter
// requires. However, its subclasses use this method to generate data, and then
// build their own data.

#ifndef __vtkStreamer_h
#define __vtkStreamer_h

#include "DS2PolyF.hh"

#define INTEGRATE_FORWARD 0
#define INTEGRATE_BACKWARD 1
#define INTEGRATE_BOTH_DIRECTIONS 2

#define START_FROM_POSITION 0
#define START_FROM_LOCATION 1

typedef struct _vtkStreamPoint {
    float   x[3];    // position 
    int     cellId;  // cell
    int     subId;   // cell sub id
    float   p[3];    // parametric coords in cell 
    float   v[3];    // velocity 
    float   speed;   // velocity norm 
    float   s;       // scalar value 
    float   t;       // time travelled so far 
    float   d;       // distance travelled so far 
    float   w[3];    // vorticity (if vorticity is computed)
    float   n[3];    // normal (if vorticity is computed)
} vtkStreamPoint;

//
// Special classes for manipulating data
//
//BTX - begin tcl exclude
//
class vtkStreamArray { //;prevent man page generation
public:
  vtkStreamArray();
  ~vtkStreamArray() {if (this->Array) delete [] this->Array;};
  int GetNumberOfPoints() {return this->MaxId + 1;};
  vtkStreamPoint *GetStreamPoint(int i) {return this->Array + i;};
  vtkStreamPoint *InsertNextStreamPoint() 
    {
    if ( ++this->MaxId >= this->Size ) this->Resize(this->MaxId);
    return this->Array + this->MaxId;
    }
  vtkStreamPoint *Resize(int sz); //reallocates data
  void Reset() {this->MaxId = -1;};

  vtkStreamPoint *Array;  // pointer to data
  int MaxId;             // maximum index inserted thus far
  int Size;       // allocated size of data
  int Extend;     // grow array by this amount
  float Direction;  // integration direction
};
//ETX - end tcl exclude
//

class vtkStreamer : public vtkDataSetToPolyFilter
{
public:
  vtkStreamer();
  ~vtkStreamer() {};
  char *GetClassName() {return "vtkStreamer";};
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetStartLocation(int cellId, int subId, float pcoords[3]);
  void SetStartLocation(int cellId, int subId, float r, float s, float t);
  int GetStartLocation(int& subId, float pcoords[3]);

  void SetStartPosition(float x[3]);
  void SetStartPosition(float x, float y, float z);
  float *GetStartPosition();

  void Update();

  // Description:
  // Specify the source object used to generate starting points.
  vtkSetObjectMacro(Source,vtkDataSet);
  vtkGetObjectMacro(Source,vtkDataSet);

  // Description:
  // Specify the maximum length of the Streamer expressed in elapsed time.
  vtkSetClampMacro(MaximumPropagationTime,float,0.0,LARGE_FLOAT);
  vtkGetMacro(MaximumPropagationTime,float);

  // Description:
  // Specify the direction in which to integrate the Streamer.
  vtkSetClampMacro(IntegrationDirection,int,
                  INTEGRATE_FORWARD,INTEGRATE_BOTH_DIRECTIONS);
  vtkGetMacro(IntegrationDirection,int);

  // Description:
  // Specify a nominal integration step size (expressed as a fraction of
  // the size of each cell).
  vtkSetClampMacro(IntegrationStepLength,float,0.001,0.5);
  vtkGetMacro(IntegrationStepLength,float);

  // Description:
  // Turn on/off the creation of scalar data from velocity magnitude. If off,
  // and input dataset has scalars, input dataset scalars are used.
  vtkSetMacro(SpeedScalars,int);
  vtkGetMacro(SpeedScalars,int);
  vtkBooleanMacro(SpeedScalars,int);

  // Description:
  // Set/get terminal speed (i.e., speed is velocity magnitude).  Terminal 
  // speed is speed at which streamer will terminate propagation.
  vtkSetClampMacro(TerminalSpeed,float,0.0,LARGE_FLOAT);
  vtkGetMacro(TerminalSpeed,float);

  // Description:
  // Turn on/off the computation of vorticity.
  vtkSetMacro(Vorticity,int);
  vtkGetMacro(Vorticity,int);
  vtkBooleanMacro(Vorticity,int);

protected:
  // Integrate data
  void Integrate();

  // Special method for computing streamer vorticity
  void ComputeVorticity();

  // Controls where streamlines start from (either position or location).
  int StartFrom;

  // Starting from cell location
  int StartCell;
  int StartSubId;
  float StartPCoords[3];

  // starting from global x-y-z position
  float StartPosition[3];

  //points used to seed streamlines  
  vtkDataSet *Source; 

  //array of streamers
  vtkStreamArray *Streamers;
  int NumberOfStreamers;

  // length of Streamer is generated by time, or by MaximumSteps
  float MaximumPropagationTime;

  // integration direction
  int IntegrationDirection;

  // the length (fraction of cell size) of integration steps
  float IntegrationStepLength;

  // boolean controls whether vorticity is computed
  int Vorticity;

  // terminal propagation speed
  float TerminalSpeed;

  // boolean controls whether data scalars or velocity magnitude are used
  int SpeedScalars;
};

#endif


