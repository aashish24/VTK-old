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

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _INTEGRAL_MAX_BITS
#undef _INTEGRAL_MAX_BITS
#endif
#define _INTEGRAL_MAX_BITS 64

#ifdef _WIN32
#include "vtkSetGet.h"
#include "vtkWin32Header.h"

// include stdmutex for borland
#ifndef _MSC_VER
#include <stdmutex.h>
#endif

#include "vtkObject.h"
HANDLE vtkGlobalMutex = NULL;
#define VTK_GET_MUTEX() WaitForSingleObject(vtkGlobalMutex,INFINITE)
#define VTK_RELEASE_MUTEX() ReleaseMutex(vtkGlobalMutex)
#include <mapiform.h>
#else
#include <thread.h>
#include <synch.h>
mutex_t vtkGlobalMutex;
#define VTK_GET_MUTEX()  mutex_lock(&vtkGlobalMutex)
#define VTK_RELEASE_MUTEX() mutex_unlock(&vtkGlobalMutex)
#endif

#include "vtkJavaUtil.h"

int vtkJavaIdCount = 1;

//#define VTKJAVADEBUG

class vtkHashNode 
{
public:
  vtkHashNode *next;
  void *key;
  void *value;
};


class vtkHashTable
{
public:
  vtkHashTable();
  vtkHashNode *(nodes[64]);
  void AddHashEntry(void *key,void *value);
  void *GetHashTableValue(void *key);
  void DeleteHashEntry(void *key);
};

vtkHashTable::vtkHashTable()
{
  int i;
  for (i = 0; i < 64; i++)
    {
    this->nodes[i] = NULL;
    }
}

vtkHashTable *vtkInstanceLookup = NULL;
vtkHashTable *vtkPointerLookup = NULL;
vtkHashTable *vtkTypecastLookup = NULL;
vtkHashTable *vtkDeleteLookup = NULL;

void vtkHashTable::AddHashEntry(void *key,void *value)
{
  vtkHashNode *pos;
  vtkHashNode *newpos;
  int loc;
  
  newpos = new vtkHashNode;
  newpos->key = key;
  newpos->value = value;
  newpos->next = NULL;
  
  loc = (((unsigned long)key) & 0x03f0) / 16;
  
  pos = this->nodes[loc];
  if (!pos)
    {
    this->nodes[loc] = newpos;
    return;
    }
  while (pos->next)
    {
    pos = pos->next;
    }
  pos->next = newpos;
}

void *vtkHashTable::GetHashTableValue(void *key)
{
  vtkHashNode *pos;
  int loc = (((unsigned long)key) & 0x03f0) / 16;
  
  pos = this->nodes[loc];

  if (!pos)
    {
    return NULL;
    }
  while ((pos)&&(pos->key != key))
    {
    pos = pos->next;
    }
  if (pos)
    {
    return pos->value;
    }
  return NULL;
}

void vtkHashTable::DeleteHashEntry(void *key)
{
  vtkHashNode *pos;
  vtkHashNode *prev = NULL;
  int loc = (((unsigned long)key) & 0x03f0) / 16;
  
  pos = this->nodes[loc];

  while ((pos)&&(pos->key != key))
    {
    prev = pos;
    pos = pos->next;
    }
  if (pos)
    {
    // we found this object
    if (prev)
      {
      prev->next = pos->next;
      }
    else
      {
      this->nodes[loc] = pos->next;
      }
    delete pos;
    }
}

int vtkJavaGetId(JNIEnv *env,jobject obj)
{
  jfieldID id;
  int result;
    
  id = env->GetFieldID(env->GetObjectClass(obj),"vtkId","I");
  
  result = (int)env->GetIntField(obj,id);
  return result;
}

void vtkJavaSetId(JNIEnv *env,jobject obj, int newVal)
{
  jfieldID id;
  jint jNewVal = (jint)newVal;
    
  id = env->GetFieldID(env->GetObjectClass(obj),"vtkId","I");
  
  env->SetIntField(obj,id,jNewVal);
}

// add an object to the hash table
void vtkJavaAddObjectToHash(JNIEnv *env, jobject obj, void *ptr,
			    void *tcFunc,int deleteMe)
{ 
  if (!vtkInstanceLookup)
    {
    vtkInstanceLookup = new vtkHashTable;
    vtkTypecastLookup = new vtkHashTable;
    vtkPointerLookup = new vtkHashTable;
    vtkDeleteLookup = new vtkHashTable;
#ifdef _WIN32
    vtkGlobalMutex = CreateMutex(NULL, FALSE, NULL);
#endif
    }

VTK_GET_MUTEX();

#ifdef VTKJAVADEBUG
  vtkGenericWarningMacro("Adding an object to hash ptr = " << ptr);
#endif  
  // lets make sure it isn't already there
  if (vtkJavaGetId(env,obj))
    {
#ifdef VTKJAVADEBUG
    vtkGenericWarningMacro("Attempt to add an object to the hash when one already exists!!!");
#endif
    VTK_RELEASE_MUTEX();
    return;
    }

  // get a unique id for this object
  // just use vtkJavaIdCount and then increment
  // to handle loop around make sure the id isn't currently in use
  while (vtkInstanceLookup->GetHashTableValue((void *)vtkJavaIdCount))
    {
    vtkJavaIdCount++;
    if (vtkJavaIdCount > 268435456) vtkJavaIdCount = 1;
    }

  vtkInstanceLookup->AddHashEntry((void *)vtkJavaIdCount,ptr);
  vtkTypecastLookup->AddHashEntry((void *)vtkJavaIdCount,tcFunc);
  vtkPointerLookup->AddHashEntry(ptr,(void *)env->NewGlobalRef(obj));
  vtkDeleteLookup->AddHashEntry((void *)vtkJavaIdCount,(void *)deleteMe);

  vtkJavaSetId(env,obj,vtkJavaIdCount);
  
#ifdef VTKJAVADEBUG
  vtkGenericWarningMacro("Added object to hash id= " << vtkJavaIdCount << " " << ptr);
#endif  
  vtkJavaIdCount++;
  VTK_RELEASE_MUTEX();
}

// should we delete this object
int vtkJavaShouldIDeleteObject(JNIEnv *env,jobject obj)
{
  int id = vtkJavaGetId(env,obj);
  
  VTK_GET_MUTEX();
  if ((int)(vtkDeleteLookup->GetHashTableValue((void *)id)))
    {
#ifdef VTKJAVADEBUG
    vtkGenericWarningMacro("Decided to delete id = " << id);
#endif
    vtkJavaDeleteObjectFromHash(env, id);
    VTK_RELEASE_MUTEX();
    return 1;
    }

#ifdef VTKJAVADEBUG
  vtkGenericWarningMacro("Decided to NOT delete id = " << id);
#endif
  vtkJavaDeleteObjectFromHash(env, id);
  VTK_RELEASE_MUTEX();
  return 0;
}


// delete an object from the hash
void vtkJavaDeleteObjectFromHash(JNIEnv *env, int id)
{
  void *ptr;
  void *vptr;
  
  ptr = vtkInstanceLookup->GetHashTableValue((void *)id);
  if (!ptr) 
    {
#ifdef VTKJAVADEBUG
    vtkGenericWarningMacro("Attempt to delete an object that doesnt exist!!!");
#endif  
    return;
    }
  vtkInstanceLookup->DeleteHashEntry((void *)id);
  vtkTypecastLookup->DeleteHashEntry((void *)id);
  vptr = vtkPointerLookup->GetHashTableValue(ptr);
  env->DeleteGlobalRef((jobject)&vptr);
  vtkPointerLookup->DeleteHashEntry(ptr);
  vtkDeleteLookup->DeleteHashEntry((void *)id);
}

jobject vtkJavaGetObjectFromPointer(void *ptr)
{
  jobject obj;

#ifdef VTKJAVADEBUG
  vtkGenericWarningMacro("Checking into pointer " << ptr);
#endif  
  obj = (jobject)vtkPointerLookup->GetHashTableValue((jobject *)ptr);
#ifdef VTKJAVADEBUG
  vtkGenericWarningMacro("Checking into pointer " << ptr << " obj = " << obj);
#endif  
  return obj;
}

void *vtkJavaGetPointerFromObject(JNIEnv *env, jobject obj, char *result_type)
{
  void *ptr;
  void *(*command)(void *,char *);
  int id;
  
  id = vtkJavaGetId(env,obj);
  ptr = vtkInstanceLookup->GetHashTableValue((void *)id);
  command = (void *(*)(void *,char *))vtkTypecastLookup->GetHashTableValue((void *)id);

#ifdef VTKJAVADEBUG
  vtkGenericWarningMacro("Checking into id " << id << " ptr = " << ptr);
#endif  

  if (!ptr)
    {
    return NULL;
    }
  
  if (command(ptr,result_type))
    {
#ifdef VTKJAVADEBUG
    vtkGenericWarningMacro("Got id= " << id << " ptr= " << ptr << " " << result_type);
#endif  
    return command(ptr,result_type);
    }
  else
    {
#ifdef VTKJAVADEBUG
    vtkGenericWarningMacro("vtk bad argument, type conversion failed.");
#endif  
    return NULL;
    }
}

jarray vtkJavaMakeJArrayOfDoubleFromDouble(JNIEnv *env, double *ptr, int size)
{
  jdoubleArray ret;
  int i;
  jdouble *array;

  ret = env->NewDoubleArray(size);
  if (ret == 0)
    {
    // should throw an exception here
    return 0;
    }

  array = env->GetDoubleArrayElements(ret,NULL);

  // copy the data
  for (i = 0; i < size; i++)
    {
    array[i] = ptr[i];
    }
  
  env->ReleaseDoubleArrayElements(ret,array,0);
  return ret;
}

jarray vtkJavaMakeJArrayOfDoubleFromFloat(JNIEnv *env, float *ptr, int size)
{
  jdoubleArray ret;
  int i;
  jdouble *array;

  ret = env->NewDoubleArray(size);
  if (ret == 0)
    {
    // should throw an exception here
    return 0;
    }

  array = env->GetDoubleArrayElements(ret,NULL);

  // copy the data
  for (i = 0; i < size; i++)
    {
    array[i] = ptr[i];
    }
  
  env->ReleaseDoubleArrayElements(ret,array,0);
  return ret;
}

jarray vtkJavaMakeJArrayOfIntFromInt(JNIEnv *env, int *ptr, int size)
{
  jintArray ret;
  int i;
  jint *array;

  ret = env->NewIntArray(size);
  if (ret == 0)
    {
    // should throw an exception here
    return 0;
    }

  array = env->GetIntArrayElements(ret,NULL);

  // copy the data
  for (i = 0; i < size; i++)
    {
    array[i] = ptr[i];
    }
  
  env->ReleaseIntArrayElements(ret,array,0);
  return ret;
}

char *vtkJavaUTFToChar(JNIEnv *env,jstring in)
{
  char *result;
  const char *inBytes;
  int length, i;
  int resultLength = 1;
  
  length = env->GetStringUTFLength(in);
  inBytes = env->GetStringUTFChars(in,NULL);
  
  for (i = 0; i < length; i++)
    {
    if ((inBytes[i] >= 0)&&(inBytes[i] < 128 )) resultLength++;
    }
  result = new char [resultLength];

  resultLength = 0; // the 0 versus 1 up above is on purpose
  for (i = 0; i < length; i++)
    {
    if ((inBytes[i] >= 0)&&(inBytes[i] < 128 ))
      {
      result[resultLength] = inBytes[i];
      resultLength++;
      }
    }
  result[resultLength] = '\0';
  env->ReleaseStringUTFChars(in,inBytes);
  return result;
}

jstring vtkJavaMakeJavaString(JNIEnv *env, const char *in)
{
  jstring result;
  char *utf;
  int inLength, utfLength, i;
  
  inLength = strlen(in);
  utfLength = inLength + 2;
  utf = new char [utfLength];
  
  for (i = 0; i < inLength; i++)
    {
    utf[i] = in[i];
    }
  utf[inLength] = 0xC0;
  utf[inLength+1] = 0x80;
  result = env->NewStringUTF(utf);

  // do we need to free utf here ? Does JNI make a copy ?
  
  return result;
}
