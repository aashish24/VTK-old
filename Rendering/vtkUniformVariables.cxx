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

#include "vtkUniformVariables.h"
#include "vtkgl.h"
#include <assert.h>
#include "vtkObjectFactory.h"

#include <vtksys/stl/map>

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkUniformVariables, "$Revision$");
vtkStandardNewMacro(vtkUniformVariables);

class ltstr
{
public:
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

class vtkUniform
{
public:
  vtkUniform()
    {
      this->Name=0;
    }
  
  const char *GetName() const
    {
      return this->Name;
    }
  
  void SetName(const char *n)
    {
      if(this->Name==0 && n==0)
        {
        return;
        }
      if(this->Name!=0 && n!=0 && strcmp(this->Name,n)==0)
        {
        return;
        }
      if(this->Name!=0)
        {
        delete[] this->Name;
        }
      if(n!=0) // copy
        {
         size_t l=strlen(n)+1;
         this->Name=new char[l];
         strncpy(this->Name,n,l);
        }
      else
        {
        this->Name=0;
        }
    }
  
  virtual ~vtkUniform()
    {
      if(this->Name!=0)
        {
        delete[] Name;
        }
    }
  
  virtual void Send(int location)=0;
  
protected:
  char *Name;
};

class vtkUniformVectorInt : public vtkUniform
{
public:
  vtkUniformVectorInt(int size,
                      int *values)
    {
      this->Size=size;
      this->Values=new int[size];
      int i=0;
      while(i<size)
        {
        this->Values[i]=values[i];
        ++i;
        }
    }
  virtual ~vtkUniformVectorInt()
    {
      delete[] this->Values;
    }
  
  int GetSize()
    {
      return this->Size;
    }
  
   virtual void Send(int location)
    {
      switch(this->Size)
        {
        case 1:
          vtkgl::Uniform1i(location,this->Values[0]);
          break;
        case 2:
          vtkgl::Uniform2i(location,this->Values[0],this->Values[1]);
          break;
        case 3:
          vtkgl::Uniform3i(location,this->Values[0],this->Values[1],
                           this->Values[2]);
          break;
        case 4:
           vtkgl::Uniform4i(location,this->Values[0],this->Values[1],
                            this->Values[2],this->Values[3]);
          break;
        }
    }
  
protected:
  int Size;
  int *Values;
};

class vtkUniformVectorFloat : public vtkUniform
{
public:
  vtkUniformVectorFloat(int size,
                      float *values)
    {
      this->Size=size;
      this->Values=new float[size];
      int i=0;
      while(i<size)
        {
        this->Values[i]=values[i];
        ++i;
        }
    }
  virtual ~vtkUniformVectorFloat()
    {
      delete[] this->Values;
    }
  
  int GetSize()
    {
      return this->Size;
    }
  
   virtual void Send(int location)
    {
      switch(this->Size)
        {
        case 1:
          vtkgl::Uniform1f(location,this->Values[0]);
          break;
        case 2:
          vtkgl::Uniform2f(location,this->Values[0],this->Values[1]);
          break;
        case 3:
          vtkgl::Uniform3f(location,this->Values[0],this->Values[1],
                           this->Values[2]);
          break;
        case 4:
           vtkgl::Uniform4f(location,this->Values[0],this->Values[1],
                            this->Values[2],this->Values[3]);
          break;
        }
    }
  
protected:
  int Size;
  float *Values;
};



// rows or columns are 2,3,4.
class vtkUniformMatrix : public vtkUniform
{
public:
  vtkUniformMatrix(int rows,
                   int columns,
                   float *values)
    {
      this->Rows=rows;
      this->Columns=columns;
      this->Values=new float[rows*columns];
      int i=0;
      while(i<rows)
        {
        int j=0;
        while(j<columns)
          {
          int index=i*columns+j;
          this->Values[index]=values[index];
          ++j;
          }
        ++i;
        }
    }
  
  int GetRows()
    {
      return this->Rows;
    }
  
  int GetColumns()
    {
      return this->Columns;
    }
  
  virtual void Send(int location)
    {
      switch(this->Rows)
        {
        case 2:
          switch(this->Columns)
            {
            case 2:
              vtkgl::UniformMatrix2fv(location,1,GL_FALSE,this->Values);
              break;
            case 3:
              vtkgl::UniformMatrix2x3fv(location,1,GL_FALSE,this->Values);
              break;
            case 4:
              vtkgl::UniformMatrix2x4fv(location,1,GL_FALSE,this->Values);
              break;
            }
          break;
        case 3:
           switch(this->Columns)
            {
            case 2:
              vtkgl::UniformMatrix3x2fv(location,1,GL_FALSE,this->Values);
              break;
            case 3:
              vtkgl::UniformMatrix3fv(location,1,GL_FALSE,this->Values);
              break;
            case 4:
              vtkgl::UniformMatrix3x4fv(location,1,GL_FALSE,this->Values);
              break;
            }
           break;
        case 4:
          switch(this->Columns)
            {
            case 2:
              vtkgl::UniformMatrix4x2fv(location,1,GL_FALSE,this->Values);
              break;
            case 3:
              vtkgl::UniformMatrix4x3fv(location,1,GL_FALSE,this->Values);
              break;
            case 4:
              vtkgl::UniformMatrix4fv(location,1,GL_FALSE,this->Values);
              break;
            }
          break;
        }
    }
  
protected:
  float *Values;
  int Rows;
  int Columns;
};

typedef vtksys_stl::map<const char *, vtkUniform *, ltstr> UniformMap;
typedef vtksys_stl::map<const char *, vtkUniform *, ltstr>::iterator UniformMapIt;

class vtkUniformVariablesMap
{
public:
  UniformMap Map;
  UniformMapIt It; // used for external iteration.
};

// ----------------------------------------------------------------------------
vtkUniformVariables::vtkUniformVariables()
{
  this->Map=new vtkUniformVariablesMap;
}

// ----------------------------------------------------------------------------
vtkUniformVariables::~vtkUniformVariables()
{
  delete this->Map;
}

// ----------------------------------------------------------------------------
void vtkUniformVariables::SetUniformi(const char *name,
                                      int numberOfComponents,
                                      int *value)
{
  assert("pre: name_exists" && name!=0);
  assert("pre: value_exists" && value!=0);
  assert("pre: valid_numberOfComponents" && numberOfComponents>=1 && numberOfComponents<=4);

  UniformMapIt cur=this->Map->Map.find(name);
  vtkUniform *u=0;
  u=new vtkUniformVectorInt(numberOfComponents,value);
  u->SetName(name);
  
  vtksys_stl::pair<const char *, vtkUniform *> p;
  p.first=name;
  p.second=u;
  
  this->Map->Map.insert(p);
  
}

// ----------------------------------------------------------------------------
void vtkUniformVariables::SetUniformf(const char *name,
                                      int numberOfComponents,
                                      float *value)
{
  assert("pre: name_exists" && name!=0);
  assert("pre: value_exists" && value!=0);
  assert("pre: valid_numberOfComponents" && numberOfComponents>=1 && numberOfComponents<=4);

  UniformMapIt cur=this->Map->Map.find(name);
  
  vtkUniform *u=0;
  u=new vtkUniformVectorFloat(numberOfComponents,value);
  u->SetName(name);
  
  vtksys_stl::pair<const char *, vtkUniform *> p;
  p.first=name;
  p.second=u;
  
  this->Map->Map.insert(p);
}

// ----------------------------------------------------------------------------
void vtkUniformVariables::SetUniformMatrix(const char *name,
                                           int rows,
                                           int colums,
                                           float *value)
{
  assert("pre: name_exists" && name!=0);
  assert("pre: value_exists" && value!=0);
  assert("pre: valid_rows" && rows>=2 && rows<=4);
  assert("pre: valid_colums" && colums>=2 && colums<=4);

  UniformMapIt cur=this->Map->Map.find(name);
  
  vtkUniform *u=0;
  
  
  
  u=new vtkUniformMatrix(rows,colums,value);
  u->SetName(name);
  vtksys_stl::pair<const char *, vtkUniform *> p;
  p.first=name;
  p.second=u;
  this->Map->Map.insert(p);
}

// ----------------------------------------------------------------------------
// Description:
// Remove uniform `name' from the list.
void vtkUniformVariables::RemoveUniform(const char *name)
{
  UniformMapIt cur=this->Map->Map.find(name);
  if(cur!=this->Map->Map.end())
    {
    this->Map->Map.erase(cur);
    }
}

// ----------------------------------------------------------------------------
// Description:
// \pre need a valid OpenGL context and a shader program in use.
void vtkUniformVariables::Send(const char *name,
                               int uniformIndex)
{
  UniformMapIt cur=this->Map->Map.find(name);
  (*cur).second->Send(uniformIndex);
}

// ----------------------------------------------------------------------------
// Description:
// Place the internal cursor on the first uniform.
void vtkUniformVariables::Start()
{
  this->Map->It=this->Map->Map.begin();
}
  
// ----------------------------------------------------------------------------
// Description:
// Is the iteration done?
bool vtkUniformVariables::IsAtEnd()
{
  return this->Map->It==this->Map->Map.end();
}
  
// ----------------------------------------------------------------------------
// Description:
// Name of the uniform at the current cursor position.
// \pre not_done: !this->IsAtEnd()
const char *vtkUniformVariables::GetCurrentName()
{
  assert("pre: not_done" && !this->IsAtEnd());
  return (*this->Map->It).first;
}
  
// ----------------------------------------------------------------------------
// Description:
// \pre need a valid OpenGL context and a shader program in use.
// \pre not_done: !this->IsAtEnd()
void vtkUniformVariables::SendCurrentUniform(int uniformIndex)
{
  assert("pre: not_done" && !this->IsAtEnd());
  (*this->Map->It).second->Send(uniformIndex);
}

// ----------------------------------------------------------------------------
// Description:
// Move the cursor to the next uniform.
// \pre not_done: !this->IsAtEnd()
void vtkUniformVariables::Next()
{
  assert("pre: not_done" && !this->IsAtEnd());
  ++this->Map->It;
}

// ----------------------------------------------------------------------------
void vtkUniformVariables::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
