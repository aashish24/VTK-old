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
// .NAME vtkStructuredGrid - topologically regular array of data
// .SECTION Description
// vtkStructuredGrid is a data object that is a concrete implementation of
// vtkDataSet. vtkStructuredGrid represents a geometric structure that is a
// topologically regular array of points. The topology is that of a cube that
// has been subdivided into a regular array of smaller cubes. Each point/cell
// can be addressed with i-j-k indices. Examples include finite difference 
// grids.

#ifndef __vtkStructuredGrid_h
#define __vtkStructuredGrid_h

#include "vtkPointSet.h"
#include "vtkStructuredData.h"
class vtkVertex;
class vtkLine;
class vtkQuad;
class vtkHexahedron;
class vtkStructuredExtent;
class vtkStructuredInformation;


class VTK_EXPORT vtkStructuredGrid : public vtkPointSet {
public:
  static vtkStructuredGrid *New() {return new vtkStructuredGrid;};

  const char *GetClassName() {return "vtkStructuredGrid";};
  void PrintSelf(ostream& os, vtkIndent indent);
 
  // Description:
  // Return what type of dataset this is.
  int GetDataObjectType() {return VTK_STRUCTURED_GRID;};

  // Description:
  // Create a similar type object
  vtkDataObject *MakeObject() {return new vtkStructuredGrid;};

  // Description:
  // Copy the geometric and topological structure of an input poly data object.
  void CopyStructure(vtkDataSet *ds);

  // Description:
  // Standard vtkDataSet API methods. See vtkDataSet for more information.
  int GetNumberOfPoints() {return vtkPointSet::GetNumberOfPoints();};
  float *GetPoint(int ptId) {return this->vtkPointSet::GetPoint(ptId);};
  void GetPoint(int ptId, float p[3]) {this->vtkPointSet::GetPoint(ptId,p);};
  vtkCell *GetCell(int cellId);
  void GetCell(int cellId, vtkGenericCell *cell);
  void GetCellBounds(int cellId, float bounds[6]);
  int GetCellType(int cellId);
  int GetNumberOfCells();
  void GetCellPoints(int cellId, vtkIdList *ptIds);
  void GetPointCells(int ptId, vtkIdList *cellIds)
    {vtkStructuredData::GetPointCells(ptId,cellIds,this->Dimensions);}
  void Initialize();
  int GetMaxCellSize() {return 8;}; //hexahedron is the largest

  // Description:
  // following methods are specific to structured grid
  void SetDimensions(int i, int j, int k);
  void SetDimensions(int dim[3]);

  // Description:
  // Get dimensions of this structured points dataset.
  vtkGetVectorMacro(Dimensions,int,3);

  // Description:
  // Return the dimensionality of the data.
  int GetDataDimension();

  // Description:
  // Methods for supporting blanking of cells.
  void BlankingOn();
  void BlankingOff();
  int GetBlanking() {return this->Blanking;};
  void BlankPoint(int ptId);
  void UnBlankPoint(int ptId);
  
  // Description:
  // Return non-zero value if specified point is visible.
  int IsPointVisible(int ptId);

  // Description:
  // For legacy compatibility. Do not use.
  void GetCellPoints(int cellId, vtkIdList &ptIds)
    {this->GetCellPoints(cellId, &ptIds);}
  void GetPointCells(int ptId, vtkIdList &cellIds)
    {this->GetPointCells(ptId, &cellIds);}

  // ----------- Stuff for streaming ---------------

  // Description:
  // Set/Get the whole extent of the data.
  void SetWholeExtent(int extent[6]);
  void SetWholeExtent(int xMin, int xMax,
		      int yMin, int yMax, int zMin, int zMax);
  void GetWholeExtent(int extent[6]);
  int *GetWholeExtent();
  void GetWholeExtent(int &xMin, int &xMax,
		      int &yMin, int &yMax, int &zMin, int &zMax);

  // Description:
  // This extent is used to request just a piece of the grid.
  // If the UpdateExtent is set before Update is called, then
  // the Update call may only generate the portion of the data 
  // requested.  The source has the option of generating more 
  // than the requested extent.  If it does, then it will
  // modify the UpdateExtent value to reflect the actual extent
  // in the data.
  void SetUpdateExtent(int extent[6]);
  void SetUpdateExtent(int xMin, int xMax,
		       int yMin, int yMax, int zMin, int zMax);
  void SetUpdateExtentToWholeExtent();
  int *GetUpdateExtent();
  void GetUpdateExtent(int ext[6]);

  // Description:
  // The generic way of specifying the update extent.
  // it blocks up the request. (taken from vtkGridSynchronizedTemplates)
  void SetUpdateExtent(int idx, int numPieces);

  // Description:
  // This extent reflects what is in the structured grid currently.
  // it is up to the source to set this during its update.
  void SetExtent(int extent[6]);
  void SetExtent(int xMin, int xMax,
		 int yMin, int yMax, int zMin, int zMax);
  int *GetExtent();

  // Description:
  // Warning: This is still in develoment.  DataSetToDataSetFilters use
  // CopyUpdateExtent to pass the update extents up the pipeline.
  vtkStructuredExtent *GetStructuredUpdateExtent() {return (vtkStructuredExtent*)this->UpdateExtent;}
  
  // Description:
  // Returns the structured grid specific information object.
  // We should be able to eventually get rid of CopyInformation method.
  vtkStructuredInformation *GetStructuredInformation()
    {return (vtkStructuredInformation*)(this->Information);}

  // Description:
  // Return the amount of memory for the update piece.
  unsigned long GetEstimatedUpdateMemorySize();  
  
  // Description:
  // Return the actual size of the data in kilobytes. This number
  // is valid only after the pipeline has updated. The memory size
  // returned is guaranteed to be greater than or equal to the
  // memory required to represent the data (e.g., extra space in
  // arrays, etc. are not included in the return value). THIS METHOD
  // IS THREAD SAFE.
  unsigned long GetActualMemorySize();
  
protected:
  vtkStructuredGrid();
  ~vtkStructuredGrid();
  vtkStructuredGrid(const vtkStructuredGrid& sg);
  void operator=(const vtkStructuredGrid&) {};

  // for the GetCell method
  vtkVertex *Vertex;
  vtkLine *Line;
  vtkQuad *Quad;  
  vtkHexahedron *Hexahedron;
  
  int Dimensions[3];
  int DataDescription;
  int Blanking;
  vtkScalars *PointVisibility;
  void AllocatePointVisibility();

  // -------- stuff for streaming ------------

  // The extent of what is currently in the structured grid.
  int Extent[6];
  
  // Called by superclass to limit UpdateExtent to be less than or equal
  // to the WholeExtent.  It assumes that UpdateInformation has been 
  // called.
  int ClipUpdateExtentWithWholeExtent();
};


inline int vtkStructuredGrid::GetNumberOfCells() 
{
  int nCells=1;
  int i;

  for (i=0; i<3; i++)
    {
    if (this->Dimensions[i] > 1)
      {
      nCells *= (this->Dimensions[i]-1);
      }
    }

  return nCells;
}

inline int vtkStructuredGrid::GetDataDimension()
{
  return vtkStructuredData::GetDataDimension(this->DataDescription);
}


inline int vtkStructuredGrid::IsPointVisible(int ptId) 
{
  if (!this->Blanking)
    {
    return 1;
    }
  else
    {
    return (int) this->PointVisibility->GetScalar(ptId);
    }
}

#endif






