/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkInstantiator.h"
#include "vtkObjectFactory.h"
#include "vtkDebugLeaks.h"

vtkCxxRevisionMacro(vtkInstantiator, "$Revision$");
vtkStandardNewMacro(vtkInstantiator);

// Node in hash table.
class vtkInstantiatorHashNode
{
public:
  typedef vtkInstantiator::CreateFunction CreateFunction;
  
  vtkInstantiatorHashNode() { this->ClassName = 0; this->Function = 0; }
  
  void SetClassName(const char* className) { this->ClassName = className; }  
  const char* GetClassName() { return this->ClassName; }
  
  void SetFunction(CreateFunction function) { this->Function = function; }
  CreateFunction GetFunction() { return this->Function; }
  
private:
  const char* ClassName;
  CreateFunction Function;
};

// Hash table used by vtkInstantiator.
class vtkInstantiatorHashTable : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkInstantiatorHashTable,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);  
  
  static vtkInstantiatorHashTable* New();
  
  typedef vtkInstantiator::CreateFunction CreateFunction;
  void Insert(const char* className, CreateFunction function);
  void Erase(const char* className, CreateFunction function);
  CreateFunction Find(const char* className);
  
protected:
  vtkInstantiatorHashTable();
  ~vtkInstantiatorHashTable();
  
  unsigned long Hash(const char* s);
  void ExtendBucket(unsigned long bucket);
  const char* AddClassName(const char* className);
  
  vtkInstantiatorHashNode** Buckets;
  unsigned int* BucketCounts;
  unsigned int* BucketSizes;
  unsigned long NumberOfBuckets;
  char** ClassNames;
  unsigned long NumberOfClassNames;
  unsigned long ClassNamesSize;
  
private:
  vtkInstantiatorHashTable(const vtkInstantiatorHashTable&);  // Not implemented.
  void operator=(const vtkInstantiatorHashTable&);  // Not implemented.
};

vtkCxxRevisionMacro(vtkInstantiatorHashTable, "$Revision$");

//----------------------------------------------------------------------------
vtkInstantiatorHashTable* vtkInstantiatorHashTable::New()
{
#ifdef VTK_DEBUG_LEAKS
  vtkDebugLeaks::ConstructClass("vtkInstantiatorHashTable");
#endif    
  // Don't use the object factory because it may not have been
  // initialized when this table is allocated.
  return new vtkInstantiatorHashTable;
}

//----------------------------------------------------------------------------
vtkInstantiatorHashTable::vtkInstantiatorHashTable()
{
  this->NumberOfBuckets = 101;
  this->Buckets = new vtkInstantiatorHashNode*[this->NumberOfBuckets];
  this->BucketCounts = new unsigned int[this->NumberOfBuckets];
  this->BucketSizes = new unsigned int[this->NumberOfBuckets];
  
  unsigned int i;
  for(i=0;i < this->NumberOfBuckets;++i)
    {
    this->BucketCounts[i] = 0;
    this->BucketSizes[i] = 16;
    this->Buckets[i] = new vtkInstantiatorHashNode[this->BucketSizes[i]];
    }
  
  this->NumberOfClassNames = 0;
  this->ClassNamesSize = 256;
  this->ClassNames = new char*[this->ClassNamesSize];
}

//----------------------------------------------------------------------------
vtkInstantiatorHashTable::~vtkInstantiatorHashTable()
{
  unsigned long i;
  for(i=0; i < this->NumberOfBuckets;++i)
    {
    delete [] this->Buckets[i];
    }
  delete [] this->BucketSizes;
  delete [] this->BucketCounts;
  delete [] this->Buckets;    
  
  for(i=0;i < this->NumberOfClassNames;++i)
    {
    delete [] this->ClassNames[i];
    }
  delete [] this->ClassNames;
}

//----------------------------------------------------------------------------
void vtkInstantiatorHashTable::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "NumberOfBuckets: " << this->NumberOfBuckets << "\n";
  unsigned int i;
  float avgBucketSize = 0;
  unsigned int maxBucketSize = 0;
  unsigned int minBucketSize = this->NumberOfClassNames;
  for(i=0;i < this->NumberOfBuckets;++i)
    {
    avgBucketSize += this->BucketCounts[i];
    if(this->BucketCounts[i] > maxBucketSize)
      { maxBucketSize = this->BucketCounts[i]; }
    if(this->BucketCounts[i] < minBucketSize)
      { minBucketSize = this->BucketCounts[i]; }
    }
  avgBucketSize /= float(this->NumberOfBuckets);
  os << indent << "Average Bucket Size: " << avgBucketSize << "\n";
  os << indent << "Minimum Bucket Size: " << minBucketSize << "\n";
  os << indent << "Maximum Bucket Size: " << maxBucketSize << "\n";
}

//----------------------------------------------------------------------------
void vtkInstantiatorHashTable::Insert(const char* className,
                                      CreateFunction function)
{
  unsigned long bucket = this->Hash(className);
  
  if(this->BucketCounts[bucket] == this->BucketSizes[bucket])
    { this->ExtendBucket(bucket); }
  
  // Do not check if the class is already registered.  It is possible
  // that more than one create function will be registered for the
  // same class, and even that the same function is registered more
  // than once.  Each register should have a corresponding unregister.
  // As long as any register has not had its corresponding unregister,
  // we want to allow the class to be created.
  unsigned int pos = this->BucketCounts[bucket]++;
  this->Buckets[bucket][pos].SetClassName(this->AddClassName(className));
  this->Buckets[bucket][pos].SetFunction(function);
}

//----------------------------------------------------------------------------
void vtkInstantiatorHashTable::Erase(const char* className,
                                     CreateFunction function)
{
  unsigned long bucket = this->Hash(className);
  
  // Find the exact registration function we have been given, and
  // remove it only once.  If more than one funcion has been
  // registered for this class, or the same function more than once,
  // each register should have its corresponding unregister.
  unsigned int i;
  for(i=0; i < this->BucketCounts[bucket];++i)
    {
    if(((this->Buckets[bucket][i].GetFunction() == function)
        && (strcmp(this->Buckets[bucket][i].GetClassName(), className) == 0)))
      {
      unsigned int j;
      --this->BucketCounts[bucket];
      for(j=i;j < this->BucketCounts[bucket];++j)
        {
        this->Buckets[bucket][j] = this->Buckets[bucket][j+1];
        }
      return;
      }
    }
}

//----------------------------------------------------------------------------
vtkInstantiatorHashTable::CreateFunction
vtkInstantiatorHashTable::Find(const char* className)
{
  unsigned long bucket = this->Hash(className);
  
  unsigned int i;
  for(i=0; i < this->BucketCounts[bucket];++i)
    {
    if(strcmp(this->Buckets[bucket][i].GetClassName(), className) == 0)
      { return this->Buckets[bucket][i].GetFunction(); }
    }
  return 0;
}

//----------------------------------------------------------------------------
unsigned long vtkInstantiatorHashTable::Hash(const char* s)
{
  unsigned long h = 0;
  for(;*s;++s) { h = 5*h + *s; }
  return h % this->NumberOfBuckets;
}
  
//----------------------------------------------------------------------------
void vtkInstantiatorHashTable::ExtendBucket(unsigned long bucket)
{
  unsigned int newSize = this->BucketSizes[bucket] * 2;
    
  vtkInstantiatorHashNode* newBucket =
    new vtkInstantiatorHashNode[newSize];
    
  unsigned int i;
  for(i=0; i < this->BucketCounts[bucket];++i)
    { newBucket[i] = this->Buckets[bucket][i]; }
    
  delete [] this->Buckets[bucket];
  this->Buckets[bucket] = newBucket;
  this->BucketSizes[bucket] = newSize;
}
  
//----------------------------------------------------------------------------
const char* vtkInstantiatorHashTable::AddClassName(const char* className)
{
  if(this->NumberOfClassNames == this->ClassNamesSize)
    {
    unsigned long newSize = this->ClassNamesSize * 2;
    char** newNames = new char*[newSize];
      
    unsigned long i;
    for(i=0;i < this->NumberOfClassNames;++i)
      { newNames[i] = this->ClassNames[i]; }
      
    delete [] this->ClassNames;
    this->ClassNames = newNames;
    this->ClassNamesSize = newSize;
    }
  
  char* newName = new char[strlen(className)+1];
  strcpy(newName, className);
  this->ClassNames[this->NumberOfClassNames++] = newName;
  
  return newName;
}

//----------------------------------------------------------------------------
// Implementation of actual vtkInstantiator class.
//----------------------------------------------------------------------------
vtkInstantiator::vtkInstantiator()
{
}

//----------------------------------------------------------------------------
vtkInstantiator::~vtkInstantiator()
{
}

//----------------------------------------------------------------------------
void vtkInstantiator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  vtkInstantiator::CreatorTable->PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkObject* vtkInstantiator::CreateInstance(const char* className)
{
  CreateFunction function = vtkInstantiator::CreatorTable->Find(className);
  if(function) { return function(); }
  return 0;
}

//----------------------------------------------------------------------------
void vtkInstantiator::RegisterInstantiator(const char* className,
                                           CreateFunction createFunction)
{
  vtkInstantiator::CreatorTable->Insert(className, createFunction);
}

//----------------------------------------------------------------------------
void vtkInstantiator::UnRegisterInstantiator(const char* className,
                                             CreateFunction createFunction)
{
  vtkInstantiator::CreatorTable->Erase(className, createFunction);
}

//----------------------------------------------------------------------------
void vtkInstantiator::ClassInitialize()
{
  vtkInstantiator::CreatorTable = vtkInstantiatorHashTable::New();
}

//----------------------------------------------------------------------------
void vtkInstantiator::ClassFinalize()
{
  vtkInstantiator::CreatorTable->Delete();
}

//----------------------------------------------------------------------------
vtkInstantiatorInitialize::vtkInstantiatorInitialize()
{
  if(++vtkInstantiatorInitialize::Count == 1)
    { vtkInstantiator::ClassInitialize(); }
}

//----------------------------------------------------------------------------
vtkInstantiatorInitialize::~vtkInstantiatorInitialize()
{
  if(--vtkInstantiatorInitialize::Count == 0)
    { vtkInstantiator::ClassFinalize(); }
}

//----------------------------------------------------------------------------
unsigned int vtkInstantiatorInitialize::Count;
vtkInstantiatorHashTable* vtkInstantiator::CreatorTable;
