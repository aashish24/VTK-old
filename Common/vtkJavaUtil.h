/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#ifndef __vtkJavaUtil_h
#define __vtkJavaUtil_h

#include "vtkSystemIncludes.h"
#include <jni.h>

class vtkObject;

extern JNIEXPORT int vtkJavaGetId(JNIEnv *env,jobject obj);

extern JNIEXPORT int vtkJavaRegisterNewObject(JNIEnv *env, jobject obj, void *ptr);
extern JNIEXPORT void vtkJavaRegisterCastFunction(JNIEnv *env, jobject obj, int id, void *tcFunc);               
          
extern JNIEXPORT void *vtkJavaGetPointerFromObject(JNIEnv *env,jobject obj, 
           char *result_type);
extern JNIEXPORT void vtkJavaDeleteObjectFromHash(JNIEnv *env, int id);
extern JNIEXPORT jobject vtkJavaGetObjectFromPointer(void *ptr);
extern JNIEXPORT int  vtkJavaShouldIDeleteObject(JNIEnv *env,jobject obj);
extern JNIEXPORT char *vtkJavaUTFToChar(JNIEnv *env, jstring in);
extern JNIEXPORT jstring vtkJavaMakeJavaString(JNIEnv *env, const char *in);

extern JNIEXPORT jarray vtkJavaMakeJArrayOfDoubleFromFloat(JNIEnv *env,
             float *arr, int size);
extern JNIEXPORT jarray vtkJavaMakeJArrayOfDoubleFromDouble(JNIEnv *env, 
              double *arr, int size);
extern JNIEXPORT jarray vtkJavaMakeJArrayOfIntFromInt(JNIEnv *env, int *arr, int size);
extern JNIEXPORT jarray vtkJavaMakeJArrayOfByteFromUnsignedChar(JNIEnv *env, unsigned char *arr, int size);

extern JNIEXPORT jobject vtkJavaCreateNewJavaStubForObject(JNIEnv *env, vtkObject* obj);
extern JNIEXPORT jobject vtkJavaCreateNewJavaStub(JNIEnv *env,
              const char* fullclassname, void* obj);

// this is the void pointer parm passed to the vtk callback routines on
// behalf of the Java interface for callbacks.
struct vtkJavaVoidFuncArg 
{
  JavaVM *vm;
  jobject  uobj;
  jmethodID mid;
} ;

extern JNIEXPORT void vtkJavaVoidFunc(void *);
extern JNIEXPORT void vtkJavaVoidFuncArgDelete(void *);

#endif
