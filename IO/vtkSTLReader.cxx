/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include <ctype.h>
#include "vtkSTLReader.h"
#include "vtkByteSwap.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkSTLReader* vtkSTLReader::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSTLReader");
  if(ret)
    {
    return (vtkSTLReader*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSTLReader;
}




#define VTK_ASCII 0
#define VTK_BINARY 1

// Construct object with merging set to true.
vtkSTLReader::vtkSTLReader()
{
  this->FileName = NULL;
  this->Merging = 1;
  this->Locator = NULL;
}

vtkSTLReader::~vtkSTLReader()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    }
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
}

// Overload standard modified time function. If locator is modified,
// then this object is modified as well.
unsigned long vtkSTLReader::GetMTime()
{
  unsigned long mTime1=this->vtkPolyDataSource::GetMTime();
  unsigned long mTime2;
  
  if (this->Locator)
    {
    mTime2 = this->Locator->GetMTime();
    mTime1 = ( mTime1 > mTime2 ? mTime1 : mTime2 );
    }

  return mTime1;
}


void vtkSTLReader::Execute()
{
  FILE *fp;
  vtkPoints *newPts, *mergedPts;
  vtkCellArray *newPolys, *mergedPolys;
  vtkPolyData *output = this->GetOutput();
  
  if (!this->FileName)
    {
    vtkErrorMacro(<<"A FileName must be specified.");
    return;
    }

  //
  // Initialize
  //
  if ((fp = fopen(this->FileName, "r")) == NULL)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return;
    }

  newPts = vtkPoints::New();
  newPts->Allocate(5000,10000);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(10000,20000);
  //
  // Depending upon file type, read differently
  //
  if ( this->GetSTLFileType(fp) == VTK_ASCII )
    {
    if ( this->ReadASCIISTL(fp,newPts,newPolys) )
      {
      return;
      }
    }
  else
    {
    fclose(fp);
    fp = fopen(this->FileName, "rb");
    if ( this->ReadBinarySTL(fp,newPts,newPolys) )
      {
      return;
      }
    }

  vtkDebugMacro(<< "Read: " 
               << newPts->GetNumberOfPoints() << " points, "
               << newPolys->GetNumberOfCells() << " triangles");

  fclose(fp);
  //
  // If merging is on, create hash table and merge points/triangles.
  //
  if ( this->Merging )
    {
    int npts, *pts, i, nodes[3];
    float *x;

    mergedPts = vtkPoints::New();
    mergedPts->Allocate(newPts->GetNumberOfPoints()/2);
    mergedPolys = vtkCellArray::New();
    mergedPolys->Allocate(newPolys->GetSize());

    if ( this->Locator == NULL )
      {
      this->CreateDefaultLocator();
      }
    this->Locator->InitPointInsertion (mergedPts, newPts->GetBounds());

    for (newPolys->InitTraversal(); newPolys->GetNextCell(npts,pts); )
      {
      for (i=0; i < 3; i++) 
        {
        x = newPts->GetPoint(pts[i]);
	this->Locator->InsertUniquePoint(x, nodes[i]);
        }

      if ( nodes[0] != nodes[1] &&
	   nodes[0] != nodes[2] && 
	   nodes[1] != nodes[2] )
	{
        mergedPolys->InsertNextCell(3,nodes);
	}
      }

      newPts->Delete();
      newPolys->Delete();

      vtkDebugMacro(<< "Merged to: " 
                   << mergedPts->GetNumberOfPoints() << " points, " 
                   << mergedPolys->GetNumberOfCells() << " triangles");
    }
  else
    {
    mergedPts = newPts;
    mergedPolys = newPolys;
    }
//
// Update ourselves
//
  output->SetPoints(mergedPts);
  mergedPts->Delete();

  output->SetPolys(mergedPolys);
  mergedPolys->Delete();

  if (this->Locator)
    {
    this->Locator->Initialize(); //free storage
    }

  output->Squeeze();
}

int vtkSTLReader::ReadBinarySTL(FILE *fp, vtkPoints *newPts, vtkCellArray *newPolys)
{
  int i, numTris, pts[3];
  unsigned long   ulint;
  unsigned short  ibuff2;
  char    header[81];
  typedef struct  {float  n[3], v1[3], v2[3], v3[3];} facet_t;
  facet_t facet;

  vtkDebugMacro(<< " Reading BINARY STL file");
//
//  File is read to obtain raw information as well as bounding box
//
  fread (header, 1, 80, fp);
  fread (&ulint, 1, 4, fp);
  vtkByteSwap::Swap4LE(&ulint);
//
// Many .stl files contain bogus count.  Hence we will ignore and read 
//   until end of file.
//
  if ( (numTris = (int) ulint) <= 0 )
    {
    vtkDebugMacro(<< "Bad binary count: attempting to correct (" << numTris << ")");
    }

  for ( i=0; fread(&facet,48,1,fp) > 0; i++ )
    {
    fread(&ibuff2,2,1,fp); //read extra junk

    vtkByteSwap::Swap4LE (facet.n);
    vtkByteSwap::Swap4LE (facet.n+1);
    vtkByteSwap::Swap4LE (facet.n+2);

    vtkByteSwap::Swap4LE (facet.v1);
    vtkByteSwap::Swap4LE (facet.v1+1);
    vtkByteSwap::Swap4LE (facet.v1+2);
    pts[0] = newPts->InsertNextPoint(facet.v1);

    vtkByteSwap::Swap4LE (facet.v2);
    vtkByteSwap::Swap4LE (facet.v2+1);
    vtkByteSwap::Swap4LE (facet.v2+2);
    pts[1] = newPts->InsertNextPoint(facet.v2);

    vtkByteSwap::Swap4LE (facet.v3);
    vtkByteSwap::Swap4LE (facet.v3+1);
    vtkByteSwap::Swap4LE (facet.v3+2);
    pts[2] = newPts->InsertNextPoint(facet.v3);

    newPolys->InsertNextCell(3,pts);

    if ( (i % 5000) == 0 && i != 0 )
      {
      vtkDebugMacro(<< "triangle# " << i);
      this->UpdateProgress((i%50000)/50000.0);
      }
    }

  return 0;
}

int vtkSTLReader::ReadASCIISTL(FILE *fp, vtkPoints *newPts, vtkCellArray *newPolys)
{
  char line[256];
  float x[3];
  int pts[3];

  vtkDebugMacro(<< " Reading ASCII STL file");
//
//  Ingest header and junk to get to first vertex
//
  fgets (line, 255, fp);
/*
 *  Go into loop, reading  facet normal and vertices
 */
  while (fscanf(fp,"%*s %*s %f %f %f\n", x, x+1, x+2)!=EOF) 
    {
    fgets (line, 255, fp);
    fscanf (fp, "%*s %f %f %f\n", x,x+1,x+2);
    pts[0] = newPts->InsertNextPoint(x);
    fscanf (fp, "%*s %f %f %f\n", x,x+1,x+2);
    pts[1] = newPts->InsertNextPoint(x);
    fscanf (fp, "%*s %f %f %f\n", x,x+1,x+2);
    pts[2] = newPts->InsertNextPoint(x);
    fgets (line, 255, fp); /* end loop */
    fgets (line, 255, fp); /* end facet */

    newPolys->InsertNextCell(3,pts);

    if ( (newPolys->GetNumberOfCells() % 5000) == 0 )
      {
      vtkDebugMacro(<< "triangle# " << newPolys->GetNumberOfCells());
      this->UpdateProgress((newPolys->GetNumberOfCells()%50000)/50000.0);
      }
    }

  return 0;
}

int vtkSTLReader::GetSTLFileType(FILE *fp)
{
  unsigned char header[256];
  int type, i;
	int numChars;

//
//  Read a little from the file to figure what type it is.
//
	/* skip 255 characters so we are past any first line comment */
  numChars = fread ((unsigned char *)header, 1, 255, fp);
  for (i = 0, type=VTK_ASCII; i< numChars && type == VTK_ASCII; i++) // don't test \0
    {
		if (header[i] > 127)
		  {
		  type = VTK_BINARY;
		  }
    }

		//
// Reset file for reading
//
  rewind (fp);
  return type;
}

// Specify a spatial locator for merging points. By
// default an instance of vtkMergePoints is used.
void vtkSTLReader::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator == locator ) 
    {
    return;
    }
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  if ( locator )
    {
    locator->Register(this);
    }
  this->Locator = locator;
  this->Modified();
}

void vtkSTLReader::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    }
}

void vtkSTLReader::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataSource::PrintSelf(os,indent);

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";

  os << indent << "Merging: " << (this->Merging ? "On\n" : "Off\n");
  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }
}
