/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkImageQuantizeRGBToIndex.h"
#include "vtkTimerLog.h"
#include <math.h>
#include <stdlib.h>
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageQuantizeRGBToIndex* vtkImageQuantizeRGBToIndex::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageQuantizeRGBToIndex");
  if(ret)
    {
    return (vtkImageQuantizeRGBToIndex*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageQuantizeRGBToIndex;
}




class vtkColorQuantizeNode
{
public:
  vtkColorQuantizeNode() 
    { this->Axis = -1; this->SplitPoint = -1; this->Index = -1; 
      this->Child1 = NULL; this->Child2 = NULL;
      this->StdDev[0] = this->StdDev[1] = this->StdDev[2] = 0.0;
      this->Histogram[0] = this->Histogram[1] = this->Histogram[2] = NULL; 
      this->Image = NULL;
      this->Bounds[0] = 0; this->Bounds[1] = 256; 
      this->Bounds[2] = 0; this->Bounds[3] = 256; 
      this->Bounds[4] = 0; this->Bounds[5] = 256; };

  ~vtkColorQuantizeNode()
    { if ( this->Histogram[0] ) { delete []this->Histogram[0]; }
      if ( this->Histogram[1] ) { delete []this->Histogram[1]; }
      if ( this->Histogram[2] ) { delete []this->Histogram[2]; }
      if ( this->Child1 ) { delete this->Child1; }
      if ( this->Child2 ) { delete this->Child2; } };

  void SetImageExtent( int v[6] ) 
    { memcpy( this->ImageExtent, v, 6*sizeof(int) ); };

  void SetImageIncrement( int v[3] )
    { memcpy( this->ImageIncrement, v, 3*sizeof(int) ); };

  void SetImageType( float type ) { this->ImageType = (int)type; };

  void SetImage( void *image ) { this->Image = image; };

  int  GetAxis(       ) { return this->Axis; };
  void SetAxis( int v ) { this->Axis = v; };

  int  GetSplitPoint(       ) { return this->SplitPoint; };
  void SetSplitPoint( int v ) { this->SplitPoint = v; };

  int *GetBounds(          ) { return this->Bounds; };
  void SetBounds( int v[6] ) { memcpy( this->Bounds, v, 6*sizeof(int) ); };

  int  GetIndex(       ) { return this->Index; };
  void SetIndex( int v ) { this->Index = v; };

  float GetStdDev( int axis ) { return this->StdDev[axis]; };
  void  ComputeStdDev();

  int GetCount() { return this->Count; };

  float GetMean( int axis ) { return this->Mean[axis]; };

  void Divide( int axis, int nextIndex );

  vtkColorQuantizeNode *GetChild1() { return this->Child1; };
  void SetChild1( vtkColorQuantizeNode *n ) { this->Child1 = n; };

  vtkColorQuantizeNode *GetChild2() { return this->Child2; };
  void SetChild2( vtkColorQuantizeNode *n ) { this->Child2 = n; };

  int GetIndex( int c[3] ) 
    {  if ( this->Index>=0 ) {return this->Index;}
       if ( c[this->Axis]>this->SplitPoint ) 
	 {return this->Child2->GetIndex(c);}
       return this->Child1->GetIndex(c); };
    

  void GetAverageColor( int c[3] ) 
    { if ( this->AverageCount ) {
      c[0] = (int)(this->AverageColor[0] / this->AverageCount);
      c[1] = (int)(this->AverageColor[1] / this->AverageCount);
      c[2] = (int)(this->AverageColor[2] / this->AverageCount); } };

  void StartColorAveraging() 
    {if (this->Child1) 
      {
      this->Child1->StartColorAveraging(); this->Child2->StartColorAveraging();
      }
    else
      {
      this->AverageCount = 0;
      this->AverageColor[0] = 
	this->AverageColor[1] = this->AverageColor[2] = 0.0;
      }
    };

  void AddColor( int c[3] )
    { this->AverageCount++; this->AverageColor[0] += c[0];
      this->AverageColor[1] += c[1]; this->AverageColor[2] += c[2]; };

protected:
  int                  Axis;
  int                  SplitPoint;
  int                  Bounds[6];
  int                  Index;
  float                StdDev[3];
  float                Median[3];
  float                Mean[3];
  int                  Count;
  int                  AverageCount;
  float                AverageColor[3];
  int                  ImageIncrement[3];
  int                  ImageExtent[6];
  int                  ImageType;
  void                 *Image;
  int                  *Histogram[3];
  vtkColorQuantizeNode *Child1, *Child2;
};

template <class T>
static void vtkImageQuantizeRGBToIndexHistogram( T *inPtr,
						 int extent[6],
						 int inIncrement[3],
						 int type,
						 int bounds[6],
						 int *histogram[3] )
{
  T      *rgbPtr, v[3];
  int    x, y, z, c;
  int    value[3];
  int    max[3];

  max[0] = bounds[1] - bounds[0] + 1;
  max[1] = bounds[3] - bounds[2] + 1;
  max[2] = bounds[5] - bounds[4] + 1;

  for ( c = 0; c < 3; c++ )
    {
    for ( x = 0; x < max[c]; x++ )
      {
      histogram[c][x] = 0;
      }
    }

  // Generate the histogram
  rgbPtr = inPtr;
  for (z = extent[4]; z <= extent[5]; z++)
    {
    for (y = extent[2]; y <= extent[3]; y++)
      {
      for (x = extent[0]; x <= extent[1]; x++)
	{
	if ( type == VTK_UNSIGNED_CHAR )
	  {
	  v[0] = *(rgbPtr++) - bounds[0];
	  v[1] = *(rgbPtr++) - bounds[2];
	  v[2] = *(rgbPtr++) - bounds[4];
	  if ( v[0] < max[0] && v[1] < max[1] && v[2] < max[2] )
	    {
	    histogram[0][(unsigned char)v[0]]++;  
	    histogram[1][(unsigned char)v[1]]++;  
	    histogram[2][(unsigned char)v[2]]++;  
	    }
	  }
	else if ( type == VTK_UNSIGNED_SHORT )
	  {
	  v[0] = (((unsigned short)(*(rgbPtr++)))>>8) - bounds[0];
	  v[1] = (((unsigned short)(*(rgbPtr++)))>>8) - bounds[2];
	  v[2] = (((unsigned short)(*(rgbPtr++)))>>8) - bounds[4];
	  if ( v[0] < max[0] && v[1] < max[1] && v[2] < max[2] )
	    {
	    histogram[0][(unsigned short)v[0]]++;  
	    histogram[1][(unsigned short)v[1]]++;  
	    histogram[2][(unsigned short)v[2]]++;  
	    }
	  }
	else
	  {
	  value[0] = (int)( *(rgbPtr++) * 255.5 ) - bounds[0];
	  value[1] = (int)( *(rgbPtr++) * 255.5 ) - bounds[2];
	  value[2] = (int)( *(rgbPtr++) * 255.5 ) - bounds[4];
	  if ( v[0] < max[0] && v[1] < max[1] && v[2] < max[2] )
	    {
	    histogram[0][value[0]]++;
	    histogram[1][value[1]]++;
	    histogram[2][value[2]]++;
	    }
	  }
	rgbPtr += inIncrement[0];
	}
      rgbPtr += inIncrement[1];
      }
    rgbPtr += inIncrement[2];
    }
}

// This templated function executes the filter for supported types of data.
template <class T>
static void vtkImageQuantizeRGBToIndexExecute(vtkImageQuantizeRGBToIndex *self,
					      vtkImageData *inData, T *inPtr,
					      vtkImageData *outData, 
					      unsigned short *outPtr)
{
  int                  extent[6];
  int                  inIncrement[3], outIncrement[3];
  T                    *rgbPtr;
  unsigned short       *indexPtr;
  int                  x, y, z, c;
  int                  type;
  vtkColorQuantizeNode *root, *tmp;
  vtkColorQuantizeNode *leafNodes[65536];
  int                  numLeafNodes;
  int                  maxdevAxis, maxdevLeafNode;
  float                maxdev, dev;
  int                  leaf, axis;
  int                  cannotDivideFurther;
  vtkLookupTable       *lut;
  float                color[4];
  int                  rgb[3];
  vtkTimerLog          *timer;
  int                  totalCount;
  float                weight;

  timer = vtkTimerLog::New();
  timer->StartTimer();
  type = self->GetInputType();

  // need extent to get increments. 
  // in and out extents are the same
  inData->GetExtent( extent );

  inData->GetContinuousIncrements(extent, inIncrement[0], 
				  inIncrement[1], inIncrement[2]);
  outData->GetContinuousIncrements(extent, outIncrement[0], 
				  outIncrement[1], outIncrement[2]);
  
  timer->StopTimer();

  self->SetInitializeExecuteTime( timer->GetElapsedTime() );
  timer->StartTimer();

  // Build the tree  
  // Create the root node - it is our only leaf node
  root = new vtkColorQuantizeNode;
  root->SetIndex( 0 );
  root->SetImageExtent( extent );
  root->SetImageIncrement( inIncrement );
  root->SetImageType( type );
  root->SetImage( inPtr );
  root->ComputeStdDev();
  leafNodes[0] = root;
  numLeafNodes = 1;

  cannotDivideFurther = 0;

  totalCount = 
    (extent[1] - extent[0] + 1) *
    (extent[3] - extent[2] + 1) *
    (extent[5] - extent[4] + 1);

  // Loop until we've added enough leaf nodes or we can't add any more
  while ( numLeafNodes < self->GetNumberOfColors() && !cannotDivideFurther )
    {
    // Find leaf node / axis with maximum deviation 
    maxdev = 0.0;
    for ( leaf = 0; leaf < numLeafNodes; leaf++ )
      {
      for ( axis = 0; axis < 3; axis++ )
	{
	dev = leafNodes[leaf]->GetStdDev( axis );
	weight = (float)(leafNodes[leaf]->GetCount())/(float)(totalCount);
	dev *= weight;
	if ( dev > maxdev )
	  {
	  maxdevAxis     = axis;
	  maxdevLeafNode = leaf;
	  maxdev         = dev;
	  }
	}
      }
    if ( maxdev == 0.0 )
      {
      cannotDivideFurther = 1;
      }
    else
      {
      leafNodes[maxdevLeafNode]->Divide( maxdevAxis, numLeafNodes ); 
      leafNodes[numLeafNodes]   = leafNodes[maxdevLeafNode]->GetChild1();
      leafNodes[maxdevLeafNode] = leafNodes[maxdevLeafNode]->GetChild2();
      numLeafNodes++;      
      }
    
    self->UpdateProgress(0.6667*numLeafNodes/self->GetNumberOfColors());
    }

  timer->StopTimer();
  self->SetBuildTreeExecuteTime( timer->GetElapsedTime() );
  timer->StartTimer();

  root->StartColorAveraging();

  // Fill in the indices in the output image
  indexPtr = outPtr;
  rgbPtr   = inPtr;
  for (z = extent[4]; z <= extent[5]; z++)
    {
    for (y = extent[2]; !self->AbortExecute && y <= extent[3]; y++)
      {
      for (x = extent[0]; x <= extent[1]; x++)
	{
	for (c = 0; c < 3; c++)
	  {
	  if ( type == VTK_UNSIGNED_CHAR )
	    {
	    rgb[c]  = (int)(*rgbPtr);
	    }
	  else if ( type == VTK_UNSIGNED_SHORT )
	    {
	    rgb[c] = ((unsigned short)(*rgbPtr))>>8;
	    }
	  else
	    {
	    rgb[c] = (int)(*rgbPtr * 255.5);
	    }
	  rgbPtr++;	  
	  }
	tmp = root;
	while( 1 )
	  {
	  if ( tmp->GetIndex() != -1 )
	    {
	    *indexPtr = tmp->GetIndex();
	    break;
	    }
	  if ( rgb[tmp->GetAxis()] > tmp->GetSplitPoint() )
	    {
	    tmp = tmp->GetChild2();
	    }
	  else
	    {
	    tmp = tmp->GetChild1();
	    }
	  }
	tmp->AddColor( rgb );
	indexPtr++;

	rgbPtr   += inIncrement[0];
	indexPtr += outIncrement[0];
	}
      rgbPtr   += inIncrement[1];
      indexPtr += outIncrement[1];
      }
    rgbPtr   += inIncrement[2];
    indexPtr += outIncrement[2];
    }

  self->UpdateProgress(0.90);

  // Fill in the lookup table
  lut = self->GetLookupTable();
  lut->SetNumberOfTableValues( numLeafNodes );
  lut->SetNumberOfColors( numLeafNodes );
  lut->SetTableRange( 0, numLeafNodes-1 );
  color[3] = 1.0;
  for ( leaf = 0; leaf < numLeafNodes; leaf++ )
    {
    leafNodes[leaf]->GetAverageColor( rgb );
    color[0] = rgb[0] / 255.0;
    color[1] = rgb[1] / 255.0;
    color[2] = rgb[2] / 255.0;
    lut->SetTableValue( leafNodes[leaf]->GetIndex(), color );
    }


  timer->StopTimer();
  self->SetLookupIndexExecuteTime( timer->GetElapsedTime() );
  timer->Delete();

  delete root;
}

	
void vtkColorQuantizeNode::ComputeStdDev()
{
  int   i, j;
  float mean;
  int   count, medianCount;

  // Create space for histogram
  this->Histogram[0] = new int[this->Bounds[1] - this->Bounds[0] + 1];
  this->Histogram[1] = new int[this->Bounds[3] - this->Bounds[2] + 1];
  this->Histogram[2] = new int[this->Bounds[5] - this->Bounds[4] + 1];

  // Create histogram
  switch (this->ImageType)
    {
    case VTK_DOUBLE:
      vtkImageQuantizeRGBToIndexHistogram( (double *)this->Image, 
					   this->ImageExtent, 
					   this->ImageIncrement,
					   this->ImageType,
					   this->Bounds, this->Histogram );
      break;
    case VTK_FLOAT:
      vtkImageQuantizeRGBToIndexHistogram( (float *)this->Image, 
					   this->ImageExtent, 
					   this->ImageIncrement,
					   this->ImageType,
					   this->Bounds, this->Histogram );
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageQuantizeRGBToIndexHistogram( (unsigned short *)this->Image, 
					   this->ImageExtent, 
					   this->ImageIncrement,
					   this->ImageType,
					   this->Bounds, this->Histogram );
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageQuantizeRGBToIndexHistogram( (unsigned char *)this->Image, 
					   this->ImageExtent, 
					   this->ImageIncrement,
					   this->ImageType,
					   this->Bounds, this->Histogram );
      break;
    }


  // Compute for r, g, and b
  for ( i = 0; i < 3; i++ )
    {
    // Compute the mean 
    mean  = 0;
    count = 0;
    for ( j = 0; j <= (this->Bounds[i*2 + 1] - this->Bounds[i*2]); j++ )
      {
      count += this->Histogram[i][j];
      mean  += this->Histogram[i][j] * (j + this->Bounds[i*2]);
      }
    mean /= (float)count;
    this->Mean[i] = mean;

    // Must have some minimum distance to subdivide - if we
    // are below this distance limit, don't compute a 
    // standard deviation since we don't want to subdivide this
    // node along this axis. Set the deviation to 0.0 and continue.
    if ( this->Bounds[i*2 + 1] == this->Bounds[i*2] )
      {
      this->StdDev[i] = 0.0;
      continue;
      }


    // Where is the median?
    medianCount = count / 2;

    // Initialize the median to unset
    this->Median[i] = -1;

    // Compute the standard deviation and the location of the median
    this->StdDev[i] = 0;
    count = 0;
    for ( j = 0; j <= (this->Bounds[i*2 + 1] - this->Bounds[i*2]); j++ )
      {
      count += this->Histogram[i][j];
      this->StdDev[i] += (float)this->Histogram[i][j] * 
	((float)j+this->Bounds[i*2]-mean) * 
	((float)j+this->Bounds[i*2]-mean); 
      if ( this->Median[i] == -1 && count > medianCount )
	{
	this->Median[i] = j + this->Bounds[i*2];
	}
      }

    // If our median is at the upper bound, bump down by one. This will
    // help in the cases where we have a distance of 2 in this dimension,
    // and just over half the entries are in the second bucket. We
    // still want to divide - the division needs to be at the first 
    // bucket.
    if ( this->Median[i] == this->Bounds[i*2 + 1] )
      {
      this->Median[i]--;
      }

    // Do the final division and square root to get the standard deviation
    this->StdDev[i] /= (float)count;
    this->StdDev[i] = sqrt( this->StdDev[i] );
    }

  // Should all be the same - just take the last one
  this->Count = count;
}

void vtkColorQuantizeNode::Divide( int axis, int nextIndex ) 
{
  int newBounds[6];

  this->Child1 = new vtkColorQuantizeNode;
  this->Child2 = new vtkColorQuantizeNode;

  memcpy( newBounds, this->Bounds, 6*sizeof(int) );

  newBounds[axis*2 + 1] = (int)(this->Median[axis]);  
  this->Child1->SetBounds( newBounds );

  newBounds[axis*2] = (int)(this->Median[axis] + 1);
  newBounds[axis*2 + 1] = (int)(this->Bounds[axis*2 + 1]);
  this->Child2->SetBounds( newBounds );

  this->SplitPoint = (int)(this->Median[axis]);
  this->Axis = axis;

  this->Child1->SetIndex( this->Index );
  this->Child2->SetIndex( nextIndex );
  this->Index = -1;

  delete [] this->Histogram[0];
  delete [] this->Histogram[1];
  delete [] this->Histogram[2];

  this->Histogram[0] = NULL;
  this->Histogram[1] = NULL;
  this->Histogram[2] = NULL;

  this->Child1->SetImageExtent( this->ImageExtent );
  this->Child1->SetImageIncrement( this->ImageIncrement );
  this->Child1->SetImageType( this->ImageType );
  this->Child1->SetImage( this->Image );

  this->Child2->SetImageExtent( this->ImageExtent );
  this->Child2->SetImageIncrement( this->ImageIncrement );
  this->Child2->SetImageType( this->ImageType );
  this->Child2->SetImage( this->Image );

  this->Child1->ComputeStdDev();
  this->Child2->ComputeStdDev();
}

// Constructor sets default values
vtkImageQuantizeRGBToIndex::vtkImageQuantizeRGBToIndex()
{
  this->LookupTable = vtkLookupTable::New();
  this->NumberOfColors = 256;
  
  this->InitializeExecuteTime = 0.0;
  this->BuildTreeExecuteTime = 0.0;
  this->LookupIndexExecuteTime = 0.0;
}

// Destructor deletes used resources
vtkImageQuantizeRGBToIndex::~vtkImageQuantizeRGBToIndex()
{
  if ( this->LookupTable )
    {
    this->LookupTable->Delete();
    }
}

// This method is passed an input and output Data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the Datas data types.
void vtkImageQuantizeRGBToIndex::Execute(vtkImageData *inData, 
					 vtkImageData *outData)
{
  void *inPtr;
  void *outPtr;
  
  inPtr = inData->GetScalarPointer();
  outPtr = outData->GetScalarPointer();
  
  // Input must be 3 components (rgb)
  if (this->GetInput()->GetNumberOfScalarComponents() != 3)
    {
    vtkErrorMacro("This filter can handles only 3 components");
    return;
    }

  // this filter expects that output is type unsigned short.
  if (outData->GetScalarType() != VTK_UNSIGNED_SHORT)
    {
    vtkErrorMacro(<< "Execute: out ScalarType " << outData->GetScalarType()
		  << " must be unsigned short\n");
    return;
    }

  this->InputType = inData->GetScalarType();

  switch ( this->InputType )
    {
    case VTK_DOUBLE:
      vtkImageQuantizeRGBToIndexExecute(this, 
			  inData, (double *)(inPtr), 
			  outData, (unsigned short *)(outPtr));
      break;
    case VTK_FLOAT:
      vtkImageQuantizeRGBToIndexExecute(this, 
			  inData, (float *)(inPtr), 
			  outData, (unsigned short *)(outPtr));
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageQuantizeRGBToIndexExecute(this, 
			  inData, (unsigned short *)(inPtr), 
			  outData, (unsigned short *)(outPtr));
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageQuantizeRGBToIndexExecute(this, 
			  inData, (unsigned char *)(inPtr), 
			  outData, (unsigned short *)(outPtr));
      break;
    default:
      vtkErrorMacro(<< "Execute: This ScalarType is not handled");
      return;
    }
}


// Change the output type and number of components
void vtkImageQuantizeRGBToIndex::ExecuteInformation(
                    vtkImageData *vtkNotUsed(inData), vtkImageData *outData)
{
  outData->SetNumberOfScalarComponents(1);
  outData->SetScalarType(VTK_UNSIGNED_SHORT);
}

// Get ALL of the input.
void vtkImageQuantizeRGBToIndex::ComputeRequiredInputUpdateExtent(int inExt[6],
								 int outExt[6])
{
  int *wholeExtent;

  wholeExtent = this->GetInput()->GetWholeExtent();
  memcpy(inExt, wholeExtent, 6*sizeof(int));
}

void vtkImageQuantizeRGBToIndex::ModifyOutputUpdateExtent()
{
  int wholeExtent[8];
  
  // Filter superclass has no control of intercept cache update.
  // a work around
  if (this->Bypass)
    {
    return;
    }
  
  this->GetOutput()->GetWholeExtent(wholeExtent);
  this->GetOutput()->SetUpdateExtent(wholeExtent);
}

void vtkImageQuantizeRGBToIndex::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);

  // Input Type is internal so we dont prit it
  //os << indent << "InputType: " << this->InputType << endl;

  os << indent << "Number Of Colors: " << this->NumberOfColors << endl;
  os << indent << "Lookup Table: " << endl << *this->LookupTable;
  os << indent << "Execute Time (in initialize stage): " << 
    this->InitializeExecuteTime << endl;
  os << indent << "Execute Time (in build tree stage): " << 
    this->BuildTreeExecuteTime << endl;
  os << indent << "Execute Time (in lookup index stage): " << 
    this->LookupIndexExecuteTime << endl;
}

