/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "vlUGridR.hh"

vlUnstructuredGridReader::vlUnstructuredGridReader()
{
}

vlUnstructuredGridReader::~vlUnstructuredGridReader()
{
}

unsigned long int vlUnstructuredGridReader::GetMTime()
{
  unsigned long dtime = this->vlUnstructuredGridSource::GetMTime();
  unsigned long rtime = this->Reader.GetMTime();
  return (dtime > rtime ? dtime : rtime);
}

// Description:
// Specify file name of vl polygonal data file to read.
void vlUnstructuredGridReader::SetFilename(char *name) 
{
  this->Reader.SetFilename(name);
}
char *vlUnstructuredGridReader::GetFilename() 
{
  return this->Reader.GetFilename();
}

// Description:
// Get the type of file (ASCII or BINARY)
int vlUnstructuredGridReader::GetFileType() 
{
  return this->Reader.GetFileType();
}

// Description:
// Set the name of the scalar data to extract. If not specified, first 
// scalar data encountered is extracted.
void vlUnstructuredGridReader::SetScalarsName(char *name) 
{
  this->Reader.SetScalarsName(name);
}
char *vlUnstructuredGridReader::GetScalarsName() 
{
  return this->Reader.GetScalarsName();
}

// Description:
// Set the name of the vector data to extract. If not specified, first 
// vector data encountered is extracted.
void vlUnstructuredGridReader::SetVectorsName(char *name) 
{
  this->Reader.SetVectorsName(name);
}
char *vlUnstructuredGridReader::GetVectorsName() 
{
  return this->Reader.GetVectorsName();
}

// Description:
// Set the name of the tensor data to extract. If not specified, first 
// tensor data encountered is extracted.
void vlUnstructuredGridReader::SetTensorsName(char *name) 
{
  this->Reader.SetTensorsName(name);
}
char *vlUnstructuredGridReader::GetTensorsName() 
{
  return this->Reader.GetTensorsName();
}

// Description:
// Set the name of the normal data to extract. If not specified, first 
// normal data encountered is extracted.
void vlUnstructuredGridReader::SetNormalsName(char *name) 
{
  this->Reader.SetNormalsName(name);
}
char *vlUnstructuredGridReader::GetNormalsName() 
{
  return this->Reader.GetNormalsName();
}

// Description:
// Set the name of the texture coordinate data to extract. If not specified,
// first texture coordinate data encountered is extracted.
void vlUnstructuredGridReader::SetTCoordsName(char *name) 
{
  this->Reader.SetTCoordsName(name);
}
char *vlUnstructuredGridReader::GetTCoordsName() 
{
  return this->Reader.GetTCoordsName();
}

// Description:
// Set the name of the lookup table data to extract. If not specified, uses 
// lookup table named by scalar. Otherwise, this specification supersedes.
void vlUnstructuredGridReader::SetLookupTableName(char *name) 
{
  this->Reader.SetLookupTableName(name);
}
char *vlUnstructuredGridReader::GetLookupTableName() 
{
  return this->Reader.GetLookupTableName();
}

void vlUnstructuredGridReader::Execute()
{
  FILE *fp;
  int numPts=0;
  int retStat;
  char line[257];
  int npts, size, ncells;
  vlCellArray *cells=NULL;
  int *types=NULL;

  vlDebugMacro(<<"Reading vl unsstructured grid...");
  this->Initialize();

  if ( !(fp=this->Reader.OpenVLFile(this->Debug)) ||
  ! this->Reader.ReadHeader(fp,this->Debug) )
      return;
//
// Read unstructured grid specific stuff
//
  if ( (retStat=fscanf(fp,"%256s",line)) == EOF || retStat < 1 ) 
    goto PREMATURE;

  if ( !strncmp(this->Reader.LowerCase(line),"dataset",(unsigned long)7) )
    {
//
// Make sure we're reading right type of geometry
//
    if ( (retStat=fscanf(fp,"%256s",line)) == EOF || retStat < 1 ) 
      goto PREMATURE;

    if ( strncmp(this->Reader.LowerCase(line),"unstructured_grid",17) )
      {
      vlErrorMacro(<< "Cannot read dataset type: " << line);
      return;
      }
//
// Might find points, cells, and cell types
//
    while (1)
      {
      if ( (retStat=fscanf(fp,"%256s",line)) == EOF || retStat < 1 ) 
        goto PREMATURE;

      if ( ! strncmp(this->Reader.LowerCase(line),"points",6) )
        {
        if ( (retStat=fscanf(fp,"%d", &numPts)) == EOF || retStat < 1 ) 
          goto PREMATURE;

        this->Reader.ReadPoints(fp, (vlPointSet *)this, numPts);
        }

      else if ( ! strncmp(line,"cells",5) )
        {
        if ( (retStat=fscanf(fp,"%d %d", &ncells, &size)) == EOF || retStat < 2 ) 
          goto PREMATURE;

        cells = new vlCellArray;
        this->Reader.ReadCells(fp, size, cells->WritePtr(ncells,size));
        cells->WrotePtr();
        if ( cells && types ) this->SetCells(types, cells);
        }

      else if ( ! strncmp(line,"cell_types",5) )
        {
        if ( (retStat=fscanf(fp,"%d", &ncells)) == EOF || retStat < 1 ) 
          goto PREMATURE;

        types = new int[ncells];
        if ( this->Reader.GetFileType() == BINARY )
          {
          if ( fgets(line,256,fp) == NULL ) goto PREMATURE; //suck up newline
          if ( fread(types,sizeof(int),ncells,fp) != ncells ) goto PREMATURE;
          }
        else //ascii
          {
          for (int i=0; i<size; i++)
            {
            if ((retStat=fscanf(fp,"%d",types[i])) == EOF || retStat < 1) 
              goto PREMATURE;
            }
          }
        if ( cells && types ) this->SetCells(types, cells);
        }

      else if ( ! strncmp(line, "point_data", 10) )
        {
        if ( (retStat=fscanf(fp,"%d", &npts)) == EOF || retStat < 1 ) 
          goto PREMATURE;
        
        if ( npts != numPts )
          {
          vlErrorMacro(<<"Number of points don't match!");
          return;
          }

        break; //out of this loop
        }

      else
        goto UNRECOGNIZED;

      }
    }

  else if ( !strncmp(line, "point_data", 10) )
    {
    if ( (retStat=fscanf(fp,"%d", &numPts)) == EOF || retStat < 1 ) 
      goto PREMATURE;

    numPts = 0;
    vlWarningMacro(<<"Not reading any dataset geometry...");
    }

  else 
    goto UNRECOGNIZED;

//
// Now read the point data
//
  this->Reader.ReadPointData(fp, (vlDataSet *)this, numPts, this->Debug);
  if ( types ) delete [] types;
  return;

  PREMATURE:
    vlErrorMacro(<< "Premature EOF");
    return;

  UNRECOGNIZED:
    vlErrorMacro(<< "Unrecognized keyord: " << line);
    return;
}

void vlUnstructuredGridReader::PrintSelf(ostream& os, vlIndent indent)
{
  vlUnstructuredGridSource::PrintSelf(os,indent);
  this->Reader.PrintSelf(os,indent.GetNextIndent());
}
