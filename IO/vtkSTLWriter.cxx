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
#include "vtkSTLWriter.hh"
#include "vtkPolygon.hh"
#include "vtkByteSwap.hh"

vtkSTLWriter::vtkSTLWriter()
{
  this->Filename = NULL;
  this->WriteMode = VTK_STL_ASCII;
}

vtkSTLWriter::~vtkSTLWriter()
{
  if ( this->Filename ) delete [] this->Filename;
}

// Description:
// Specify the input data or filter.
void vtkSTLWriter::SetInput(vtkPolyData *input)
{
  if ( this->Input != input )
    {
    vtkDebugMacro(<<" setting Input to " << (void *)input);
    this->Input = (vtkDataSet *) input;
    this->Modified();
    }
}

void vtkSTLWriter::WriteData()
{
  vtkPoints *pts;
  vtkCellArray *polys;
  vtkPolyData *input=(vtkPolyData *)this->Input;

  if ( (pts = input->GetPoints()) == NULL ||
  (polys = input->GetPolys()) == NULL )
    {
    vtkErrorMacro(<<"No data to write!");
    return;
    }

  if ( this->Filename == NULL)
    {
    vtkErrorMacro(<< "Please specify filename to write");
    return;
    }

  if ( this->WriteMode == VTK_STL_BINARY ) this->WriteBinarySTL(pts,polys);
  else this->WriteAsciiSTL(pts,polys);
}

static char header[]="Visualization Toolkit generated SLA File                                        ";

void vtkSTLWriter::WriteAsciiSTL(vtkPoints *pts, vtkCellArray *polys)
{
  FILE *fp;
  float n[3], *v1, *v2, *v3;
  int npts, *indx;
  vtkPolygon poly;

  if ((fp = fopen(this->Filename, "w")) == NULL)
    {
    vtkErrorMacro(<< "Couldn't open file: " << this->Filename);
    return;
    }
//
//  Write header
//
  vtkDebugMacro("Writing ASCII sla file");
  fprintf (fp, "%80s\n", header);
//
//  Write out triangle polygons.  In not a triangle polygon, only first 
//  three vertices are written.
//
  for (polys->InitTraversal(); polys->GetNextCell(npts,indx); )
    {
    v1 = pts->GetPoint(indx[0]);
    v2 = pts->GetPoint(indx[1]);
    v3 = pts->GetPoint(indx[2]);

    poly.ComputeNormal(pts, npts, indx, n);

    fprintf (fp, " FACET NORMAL %.6g %.6g %.6g\n  OUTER LOOP\n",
            n[0], n[1], n[2]);

    fprintf (fp, "   VERTEX %.6g %.6g %.6g\n", v1[0], v1[1], v1[2]);
    fprintf (fp, "   VERTEX %.6g %.6g %.6g\n", v2[0], v2[1], v2[2]);
    fprintf (fp, "   VERTEX %.6g %.6g %.6g\n", v3[0], v3[1], v3[2]);

    fprintf (fp, "  ENDLOOP\n ENDFACET\n");
    }
  fprintf (fp, "ENDSOLID\n");
}

void vtkSTLWriter::WriteBinarySTL(vtkPoints *pts, vtkCellArray *polys)
{
  FILE *fp;
  float n[3], *v1, *v2, *v3;
  int npts, *indx;
  vtkPolygon poly;
  vtkByteSwap swap;
  unsigned long ulint;
  unsigned short ibuff2=0;

  if ((fp = fopen(this->Filename, "wb")) == NULL)
    {
    vtkErrorMacro(<< "Couldn't open file: " << this->Filename);
    return;
    }
//
//  Write header
//
  vtkDebugMacro("Writing ASCII sla file");
  fwrite (header, 1, 80, fp);

  ulint = (unsigned long int) polys->GetNumberOfCells();
  swap.Swap4(&ulint);
  fwrite (&ulint, 1, 4, fp);
//
//  Write out triangle polygons.  In not a triangle polygon, only first 
//  three vertices are written.
//
  for (polys->InitTraversal(); polys->GetNextCell(npts,indx); )
    {
    v1 = pts->GetPoint(indx[0]);
    v2 = pts->GetPoint(indx[1]);
    v3 = pts->GetPoint(indx[2]);

    poly.ComputeNormal(pts, npts, indx, n);
    swap.Swap4(n); swap.Swap4(n+1); swap.Swap4(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = v1[0];  n[1] = v1[1];  n[2] = v1[2]; 
    swap.Swap4(n); swap.Swap4(n+1); swap.Swap4(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = v2[0];  n[1] = v2[1];  n[2] = v2[2]; 
    swap.Swap4(n); swap.Swap4(n+1); swap.Swap4(n+2);
    fwrite (n, 4, 3, fp);

    n[0] = v3[0];  n[1] = v3[1];  n[2] = v3[2]; 
    swap.Swap4(n); swap.Swap4(n+1); swap.Swap4(n+2);
    fwrite (n, 4, 3, fp);

    fwrite (&ibuff2, 2, 1, fp);
    }
}

void vtkSTLWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkWriter::PrintSelf(os,indent);
 
  os << indent << "Filename: " << this->Filename << "\n";

  if ( this->WriteMode == VTK_STL_ASCII  )
    os << indent << "Write Mode: ASCII\n";
  else
    os << indent << "Write Mode: BINARY\n";

}

