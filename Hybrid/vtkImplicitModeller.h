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
// .NAME vtkImplicitModeller - compute distance from input geometry on structured point dataset
// .SECTION Description
// vtkImplicitModeller is a filter that computes the distance from the input
// geometry to the points of an output structured point set. This distance
// function can then be "contoured" to generate new, offset surfaces from
// the original geometry. An important feature of this object is
// "capping". If capping is turned on, after the implicit model is created,
// the values on the boundary of the structured points dataset are set to
// the cap value. This is used to force closure of the resulting contoured
// surface. Note, however, that large cap values can generate weird surface
// normals in those cells adjacent to the boundary of the dataset. Using
// smaller cap value will reduce this effect.
//
// Another important ivar is MaximumDistance. This controls how far into the
// volume the distance function is computed from the input geometry.  Small
// values give significant increases in performance. However, there can
// strange sampling effects at the extreme range of the MaximumDistance.
//
// In order to properly execute and sample the input data, a rectangular
// region in space must be defined (this is the ivar ModelBounds).  If not
// explicitly defined, the model bounds will be computed. Note that to avoid
// boundary effects, it is possible to adjust the model bounds (i.e., using
// the AdjustBounds and AdjustDistance ivars) to strictly contain the
// sampled data.
//
// This filter has one other unusual capability: it is possible to append
// data in a sequence of operations to generate a single output. This is
// useful when you have multiple datasets and want to create a
// conglomeration of all the data.  However, the user must be careful to
// either specify the ModelBounds or specify the first item such that its
// bounds completely contain all other items.  This is because the 
// rectangular region of the output can not be changed after the 1st Append.
//
// The ProcessMode ivar controls the method used within the Append function
// (where the actual work is done regardless if the Append function is
// explicitly called) to compute the implicit model.  If set to work in voxel
// mode, each voxel is visited once.  If set to cell mode, each cell is visited
// once.  Tests have shown once per voxel to be faster when there are a 
// lot of cells (at least a thousand?); relative performance improvement 
// increases with addition cells. Primitives should not be stripped for best
// performance of the voxel mode.  Also, if explicitly using the Append feature
// many times, the cell mode will probably be better because each voxel will be
// visited each Append.  Append the data before input if possible when using
// the voxel mode.
//
// Further performance improvement is now possible using the PerVoxel process
// mode on multi-processor machines (the mode is now multithreaded).  Each
// thread processes a different "slab" of the output.  Also, if the input is 
// vtkPolyData, it is appropriately clipped for each thread; that is, each 
// thread only considers the input which could affect its slab of the output.

// .SECTION See Also
// vtkSampleFunction vtkContourFilter

#ifndef __vtkImplicitModeller_h
#define __vtkImplicitModeller_h

#include "vtkDataSetToStructuredPointsFilter.h"

#define VTK_VOXEL_MODE   0
#define VTK_CELL_MODE    1

class vtkMultiThreader;
class vtkExtractGeometry;

class VTK_EXPORT vtkImplicitModeller : public vtkDataSetToStructuredPointsFilter 
{
public:
  vtkImplicitModeller();
  const char *GetClassName() {return "vtkImplicitModeller";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with sample dimensions=(50,50,50), and so that model bounds are
  // automatically computed from the input. Capping is turned on with CapValue
  // equal to a large positive number.
  static vtkImplicitModeller *New() {return new vtkImplicitModeller;};

  // Description:
  // Compute ModelBounds from input geometry.
  float ComputeModelBounds();

  // Description:
  // Set/Get the i-j-k dimensions on which to sample distance function.
  vtkGetVectorMacro(SampleDimensions,int,3);
  void SetSampleDimensions(int i, int j, int k);
  void SetSampleDimensions(int dim[3]);

  // Description:
  // Set / get the distance away from surface of input geometry to
  // sample. Smaller values make large increases in performance.
  vtkSetClampMacro(MaximumDistance,float,0.0,1.0);
  vtkGetMacro(MaximumDistance,float);

  // Description:
  // Set / get the region in space in which to perform the sampling. If
  // not specified, it will be computed automatically.
  vtkSetVector6Macro(ModelBounds,float);
  vtkGetVectorMacro(ModelBounds,float,6);

  // Description:
  // Control how the model bounds are computed. If the ivar AdjustBounds
  // is set, then the bounds specified (or computed automatically) is modified
  // by the fraction given by AdjustDistance. This means that the model
  // bounds is expanded in each of the x-y-z directions.
  vtkSetMacro(AdjustBounds,int);
  vtkGetMacro(AdjustBounds,int);
  vtkBooleanMacro(AdjustBounds,int);
  
  // Description:
  // Specify the amount to grow the model bounds (if the ivar AdjustBounds
  // is set). The value is a fraction of the maximum length of the sides
  // of the box specified by the model bounds.
  vtkSetClampMacro(AdjustDistance,float,-1.0,1.0);
  vtkGetMacro(AdjustDistance,float);

  // Description:
  // The outer boundary of the structured point set can be assigned a 
  // particular value. This can be used to close or "cap" all surfaces.
  vtkSetMacro(Capping,int);
  vtkGetMacro(Capping,int);
  vtkBooleanMacro(Capping,int);
  
  // Description:
  // Specify the capping value to use. The CapValue is also used as an
  // initial distance value at each point in the dataset.
  vtkSetMacro(CapValue,float);
  vtkGetMacro(CapValue,float);

  // Description:
  // Specify whether to visit each cell once per append or each voxel once
  // per append.  Some tests have shown once per voxel to be faster
  // when there are a lot of cells (at least a thousand?); relative
  // performance improvement increases with addition cells.  Primitives
  // should not be stripped for best performance of the voxel mode.
  vtkSetClampMacro(ProcessMode, int, 0, 1);
  vtkGetMacro(ProcessMode, int);
  void SetProcessModeToPerVoxel() {this->SetProcessMode(VTK_VOXEL_MODE);}
  void SetProcessModeToPerCell()  {this->SetProcessMode(VTK_CELL_MODE);}
  char *GetProcessModeAsString(void);

  // Description:
  // Specify the level of the locator to use when using the per voxel
  // process mode.
  vtkSetMacro(LocatorMaxLevel,int);
  vtkGetMacro(LocatorMaxLevel,int);

  // Description:
  // Set / Get the number of threads used during Per-Voxel processing mode
  vtkSetMacro( NumberOfThreads, int );
  vtkGetMacro( NumberOfThreads, int );

  // Description:
  // Special update methods handles possibility of appending data.
  void Update();

  // Description:
  // Initialize the filter for appending data. You must invoke the
  // StartAppend() method before doing successive Appends(). It's also a
  // good idea to manually specify the model bounds; otherwise the input
  // bounds for the data will be used.
  void StartAppend();

  // Description:
  // Append a data set to the existing output. To use this function,
  // you'll have to invoke the StartAppend() method before doing
  // successive appends. It's also a good idea to specify the model
  // bounds; otherwise the input model bounds is used. When you've
  // finished appending, use the EndAppend() method.
  void Append(vtkDataSet *input);

  // Description:
  // Method completes the append process.
  void EndAppend();

protected:
  void Execute();
  void Cap(vtkScalars *s);

  vtkMultiThreader *Threader;
  int              NumberOfThreads;

  int SampleDimensions[3];
  float MaximumDistance;
  float ModelBounds[6];
  int Capping;
  float CapValue;
  int DataAppended;
  int AdjustBounds;
  float AdjustDistance;
  int ProcessMode;
  int LocatorMaxLevel;

  int BoundsComputed; // flag to limit to one ComputeModelBounds per StartAppend
  float InternalMaxDistance; // the max distance computed during that one call
};

#endif


