/*=========================================================================

  Program:   Java Wrapper for VTK
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file's contents may be copied, reproduced or altered in any way 
without the express written consent of the author.

Copyright (c) Ken Martin 1995

=========================================================================*/

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread.h>
#include <synch.h>
#include "vtkJavaUtil.h"

mutex_t vtkGlobalMutex;

int vtkJavaIdCount = 1;

#define VTKJAVADEBUG

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
  jint jNewVal = newVal;
    
  id = env->GetFieldID(env->GetObjectClass(obj),"vtkId","I");
  
  env->SetIntField(obj,id,jNewVal);
}

// add an object to the hash table
void vtkJavaAddObjectToHash(JNIEnv *env, jobject obj, void *ptr,
			    void *tcFunc,int deleteMe)
{
  mutex_lock(&vtkGlobalMutex);
  
  if (!vtkInstanceLookup)
    {
    vtkInstanceLookup = new vtkHashTable;
    vtkTypecastLookup = new vtkHashTable;
    vtkPointerLookup = new vtkHashTable;
    vtkDeleteLookup = new vtkHashTable;
    }
#ifdef VTKJAVADEBUG
  cerr << "Adding an object to hash ptr = " << ptr << "\n";
#endif  
  // lets make sure it isn't already there
  if (vtkJavaGetId(env,obj))
    {
#ifdef VTKJAVADEBUG
    cerr << "Attempt to add an object to the hash when one already exists!!!\n";
#endif
    mutex_unlock(&vtkGlobalMutex);
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
  cerr << "Added object to hash id= " << vtkJavaIdCount << " " << ptr << "\n";
#endif  
  vtkJavaIdCount++;
  mutex_unlock(&vtkGlobalMutex);
}

// should we delete this object
int vtkJavaShouldIDeleteObject(JNIEnv *env,jobject obj)
{
  int id = vtkJavaGetId(env,obj);
  
  mutex_lock(&vtkGlobalMutex);
  if ((int)(vtkDeleteLookup->GetHashTableValue((void *)id)))
    {
#ifdef VTKJAVADEBUG
    cerr << "Decided to delete id = " << id << "\n";
#endif
    vtkJavaDeleteObjectFromHash(env, id);
    mutex_unlock(&vtkGlobalMutex);
    return 1;
    }

#ifdef VTKJAVADEBUG
  cerr << "Decided to NOT delete id = " << id << "\n";
#endif
  vtkJavaDeleteObjectFromHash(env, id);
  mutex_unlock(&vtkGlobalMutex);
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
    cerr << "Attempt to delete an object that doesnt exist!!!";
#endif  
    return;
    }
  vtkInstanceLookup->DeleteHashEntry((void *)id);
  vtkTypecastLookup->DeleteHashEntry((void *)id);
  vptr = vtkPointerLookup->GetHashTableValue(ptr);
  env->DeleteGlobalRef(&vptr);
  vtkPointerLookup->DeleteHashEntry(ptr);
  vtkDeleteLookup->DeleteHashEntry((void *)id);
}

jobject vtkJavaGetObjectFromPointer(void *ptr)
{
  jobject obj;

#ifdef VTKJAVADEBUG
  cerr << "Checking into pointer " << ptr << "\n";
#endif  
  obj = vtkPointerLookup->GetHashTableValue(ptr);
#ifdef VTKJAVADEBUG
  cerr << "Checking into pointer " << ptr << " obj = " << obj << "\n";
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
  cerr << "Checking into id " << id << " ptr = " << ptr << "\n";
#endif  

  if (!ptr)
    {
    return NULL;
    }
  
  if (command(ptr,result_type))
    {
#ifdef VTKJAVADEBUG
    cerr << "Got id= " << id << " ptr= " << ptr << " " << result_type << "\n";
#endif  
    return command(ptr,result_type);
    }
  else
    {
#ifdef VTKJAVADEBUG
    fprintf(stderr,"vtk bad argument, type conversion failed.\n");
#endif  
    return NULL;
    }
}

jarray vtkJavaMakeJArrayOfDoubleFromDouble(JNIEnv *env, double *ptr, int size)
{
  jarray ret;
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
  jarray ret;
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
  jarray ret;
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
  jbyte *inBytes;
  int length, i;
  int resultLength = 1;
  
  length = env->GetStringUTFLength(in);
  inBytes = env->GetStringUTFChars(in,NULL);
  
  for (i = 0; i < length; i++)
    {
    if (inBytes[i] < 128) resultLength++;
    }
  result = new char [resultLength];

  resultLength = 0; // the 0 versus 1 up above is on purpose
  for (i = 0; i < length; i++)
    {
    if (inBytes[i] < 128) 
      {
      result[resultLength] = inBytes[i];
      resultLength++;
      }
    }
  result[resultLength] = '\0';
  env->ReleaseStringUTFChars(in,inBytes);
  return result;
}

jstring vtkJavaMakeJavaString(JNIEnv *env, char *in)
{
  jstring result;
  jbyte *utf;
  int inLength, utfLength, i;
  
  inLength = strlen(in);
  utfLength = inLength + 2;
  utf = new jbyte [utfLength];
  
  for (i = 0; i < inLength; i++)
    {
    utf[i] = in[i];
    }
  utf[inLength] = 0xC0;
  utf[inLength+1] = 0x80;
  result = env->NewStringUTF(utf,utfLength);

  // do we need to free utf here ? Does JNI make a copy ?
  
  return result;
}
