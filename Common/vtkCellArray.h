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
// .NAME vtkCellArray - object to represent cell connectivity
// .SECTION Description
// vtkCellArray is a supporting object that explicitly represents cell 
// connectivity. The cell array structure is a raw integer list
// of the form: (n,id1,id2,...,idn, n,id1,id2,...,idn, ...)
// where n is the number of points in the cell, and id is a zero-offset index 
// into an associated point list.
//
// Advantages of this data structure are its compactness, simplicity, and 
// easy interface to external data.  However, it is totally inadequate for 
// random access.  This functionality (when necessary) is accomplished by 
// using the vtkCellTypes and vtkCellLinks objects to extend the definition of 
// the data structure.
//
// .SECTION See Also
// vtkCellTypes vtkCellLinks

#ifndef __vtkCellArray_h
#define __vtkCellArray_h

#include "vtkIntArray.h"
#include "vtkCell.h"

class VTK_EXPORT vtkCellArray : public vtkObject
{
public:
  const char *GetClassName() {return "vtkCellArray";};

  // Description:
  // Instantiate cell array (connectivity list).
  static vtkCellArray *New() {return new vtkCellArray;};

  // Description:
  // Allocate memory and set the size to extend by.
  int Allocate(const int sz, const int ext=1000) 
    {return this->Ia->Allocate(sz,ext);};

  // Description:
  // Free any memory and reset to an empty state.
  void Initialize() {this->Ia->Initialize();};

  // Description:
  // Get the number of cells in the array.
  int GetNumberOfCells() {return this->NumberOfCells;};

  // Description:
  // Utility routines help manage memory of cell array. EstimateSize()
  // returns a value used to initialize and allocate memory for array based
  // on number of cells and maximum number of points making up cell.  If 
  // every cell is the same size (in terms of number of points), then the 
  // memory estimate is guaranteed exact. (If not exact, use Squeeze() to
  // reclaim any extra memory.)
  int EstimateSize(int numCells, int maxPtsPerCell)
  {return numCells*(1+maxPtsPerCell);};

  // Description:
  // A cell traversal methods that is more efficient than vtkDataSet traversal
  // methods.  InitTraversal() initializes the traversal of the list of cells.
  void InitTraversal() {this->TraversalLocation=0;};

  // Description:
  // A cell traversal methods that is more efficient than vtkDataSet traversal
  // methods.  GetNextCell() gets the next cell in the list. If end of list
  // is encountered, 0 is returned.
  int GetNextCell(int& npts, int* &pts);

  // Description:
  // Get the size of the allocated connectivity array.
  int GetSize() {return this->Ia->GetSize();};
  
  // Description:
  // Get the total number of entries (i.e., data values) in the connectivity 
  // array. This may be much less than the allocated size (i.e., return value 
  // from GetSize().)
  int GetNumberOfConnectivityEntries() {return this->Ia->GetMaxId()+1;};

  // Description:
  // Internal method used to retrieve a cell given an offset into
  // the internal array.
  void GetCell(int loc, int &npts, int* &pts);

  // Description:
  // Insert a cell object. Return the cell id of the cell.
  int InsertNextCell(vtkCell *cell);

  // Description:
  // Create a cell by specifying the number of points and an array of point
  // id's.  Return the cell id of the cell.
  int InsertNextCell(int npts, int* pts);

  // Description:
  // Create a cell by specifying a list of point ids. Return the cell id of
  // the cell.
  int InsertNextCell(vtkIdList *pts);

  // Description:
  // Create cells by specifying count, and then adding points one at a time
  // using method InsertCellPoint(). If you don't know the count initially,
  // use the method UpdateCellCount() to complete the cell. Return the cell
  // id of the cell.
  int InsertNextCell(int npts);

  // Description:
  // Used in conjunction with InsertNextCell(int npts) to add another point
  // to the list of cells.
  void InsertCellPoint(int id);

  // Description:
  // Used in conjunction with InsertNextCell(int npts) and InsertCellPoint() to
  // update the number of points defining the cell.
  void UpdateCellCount(int npts);

  // Description:
  // Computes the current insertion location within the internal array. 
  // Used in conjunction with GetCell(int loc,...).
  int GetInsertLocation(int npts) {return (this->InsertLocation - npts - 1);};
  
  // Description:
  // Computes the current traversal location within the internal array. Used 
  // in conjunction with GetCell(int loc,...).
  int GetTraversalLocation(int npts) {return(this->TraversalLocation-npts-1);};
  
  // Description:
  // Special method inverts ordering of current cell. Must be called
  // carefully or the cell topology may be corrupted.
  void ReverseCell(int loc);

  // Description:
  // Replace the point ids of the cell with a different list of point ids.
  void ReplaceCell(int loc, int npts, int *pts);

  // Description:
  // Returns the size of the largest cell. The size is the number of points
  // defining the cell.
  int GetMaxCellSize();

  // Description:
  // Get pointer to array of cell data.
  int *GetPointer() { return this->Ia->GetPointer(0);};

  // Description:
  // Get pointer to data array for purpose of direct writes of data. Size is the
  // total storage consumed by the cell array. ncells is the number of cells
  // represented in the array.
  int *WritePointer(const int ncells, const int size);

  // Description:
  // Define multiple cells by providing a connectivity list. The list is in
  // the form (npts,p0,p1,...p(npts-1), repeated for each cell). Be careful
  // using this method because it discards the old cells, and anything
  // refering these cells becomes invalid (for example, if BuildCells() has
  // been called see vtkPolyData).  The traversal location is reset to the
  // beginning of the list; the insertion location is set to the end of the
  // list.
  void SetCells(int ncells, vtkIntArray *cells);
  
  // Description:
  // Perform a deep copy (no reference countsing) of the given cell array.
  void DeepCopy(vtkCellArray *ca);

  // Description:
  // Return the underlying data as a data array.
  vtkDataArray *GetData() {return this->Ia;};

  // Description:
  // Reuse list. Reset to initial condition.
  void Reset();

  // Description:
  // Reclaim any extra memory.
  void Squeeze() {this->Ia->Squeeze();}

  // Description:
  // Return the memory in kilobytes consumed by this cell array. Used to
  // support streaming and reading/writing data. The value returned is
  // guaranteed to be greater than or equal to the memory required to
  // actually represent the data represented by this object. The 
  // information returned is valid only after the pipeline has 
  // been updated.
  unsigned long GetActualMemorySize();
  
  // Description:
  // For legacy compatibility. Do not use.
  int InsertNextCell(vtkIdList &pts);
  
protected:
  vtkCellArray();
  vtkCellArray (const int sz, const int ext=1000);
  ~vtkCellArray();
  vtkCellArray(const vtkCellArray&) {};
  void operator=(const vtkCellArray&) {};

  int NumberOfCells;
  int InsertLocation;     //keep track of current insertion point
  int TraversalLocation;   //keep track of traversal position
  vtkIntArray *Ia;
};


inline int vtkCellArray::InsertNextCell(int npts, int* pts)
{
  int i = this->Ia->GetMaxId() + 1;
  int *ptr = this->Ia->WritePointer(i,npts+1);
  
  for ( *ptr++ = npts, i = 0; i < npts; i++)
    {
    *ptr++ = *pts++;
    }

  this->NumberOfCells++;
  this->InsertLocation += npts + 1;

  return this->NumberOfCells - 1;
}

inline int vtkCellArray::InsertNextCell(vtkIdList *pts)
{
  int npts = pts->GetNumberOfIds();
  int i = this->Ia->GetMaxId() + 1;
  int *ptr = this->Ia->WritePointer(i,npts+1);
  
  for ( *ptr++ = npts, i = 0; i < npts; i++)
    {
    *ptr++ = pts->GetId(i);
    }

  this->NumberOfCells++;
  this->InsertLocation += npts + 1;

  return this->NumberOfCells - 1;
}

inline int vtkCellArray::InsertNextCell(int npts)
{
  this->InsertLocation = this->Ia->InsertNextValue(npts) + 1;
  this->NumberOfCells++;

  return this->NumberOfCells - 1;
}

inline void vtkCellArray::InsertCellPoint(int id) 
{
  this->Ia->InsertValue(this->InsertLocation++,id);
}

inline void vtkCellArray::UpdateCellCount(int npts) 
{
  this->Ia->SetValue(this->InsertLocation-npts-1, npts);
}

inline int vtkCellArray::InsertNextCell(vtkCell *cell)
{
  int npts = cell->GetNumberOfPoints();
  int i = this->Ia->GetMaxId() + 1;
  int *ptr = this->Ia->WritePointer(i,npts+1);
  
  for ( *ptr++ = npts, i = 0; i < npts; i++)
    {
    *ptr++ = cell->PointIds->GetId(i);
    }

  this->NumberOfCells++;
  this->InsertLocation += npts + 1;

  return this->NumberOfCells - 1;
}


inline void vtkCellArray::Reset() 
{
  this->NumberOfCells = 0;
  this->InsertLocation = 0;
  this->TraversalLocation = 0;
  this->Ia->Reset();
}


inline int vtkCellArray::GetNextCell(int& npts, int* &pts)
{
  if ( this->Ia->GetMaxId() >= 0 && 
  this->TraversalLocation <= this->Ia->GetMaxId() ) 
    {
    npts = this->Ia->GetValue(this->TraversalLocation++);
    pts = this->Ia->GetPointer(this->TraversalLocation);
    this->TraversalLocation += npts;
    return 1;
    }
  else
    {
    return 0;
    }
}

inline void vtkCellArray::GetCell(int loc, int &npts, int* &pts)
{
  npts=this->Ia->GetValue(loc++); pts=this->Ia->GetPointer(loc);
}


inline void vtkCellArray::ReverseCell(int loc)
{
  int i, tmp;
  int npts=this->Ia->GetValue(loc);
  int *pts=this->Ia->GetPointer(loc+1);
  for (i=0; i < (npts/2); i++) 
    {
    tmp = pts[i];
    pts[i] = pts[npts-i-1];
    pts[npts-i-1] = tmp;
    }
}

inline void vtkCellArray::ReplaceCell(int loc, int npts, int *pts)
{
  int *oldPts=this->Ia->GetPointer(loc+1);
  for (int i=0; i < npts; i++)
    {
    oldPts[i] = pts[i];
    }
}

inline int *vtkCellArray::WritePointer(const int ncells, const int size)
{
  this->NumberOfCells = ncells;
  this->InsertLocation = 0;
  this->TraversalLocation = 0;
  return this->Ia->WritePointer(0,size);
}

#endif
