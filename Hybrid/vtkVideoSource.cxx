/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to David G. Gobbi who developed this class.

Copyright (c) 1993-1999 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include <string.h>
#include <time.h>
#include <iostream.h>
#include "vtkVideoSource.h"
#include "vtkObjectFactory.h"

//---------------------------------------------------------------
// Important FrameBufferMutex rules:
// 
// The frame grabs are generally done asynchronously, and it is necessary
// to ensure that when the frame buffer is valid when it is being written 
// to or read from
//
// The following information can only be changed within a mutex lock,
// and the lock must not be released until the frame buffer agrees with the
// information.
//
// FrameBuffer
// FrameBufferTimeStamps
// FrameBufferSize
// FrameBufferIndex
// FrameBufferExtent
// FrameBufferBitsPerPixel
// FrameBufferRowAlignment
// GrabOnUpdate
//
// After one of the above has been changed, and before the mutex is released,
// the following must be called to update the frame buffer:
//
// UpdateFrameBuffer()
//
// Likewise, the following function must only be called from within a
// mutex lock because it modifies FrameBufferIndex:
//
// AdvanceFrameBuffer()
//
// Any methods which might be called asynchronously must lock the 
// mutex before reading the above information, and you must be very 
// careful when accessing any information except for the above.
// These methods include the following:
//
// InternalGrab()
//
// Finally, when Execute() is reading from the FrameBuffer it must do
// so from within a mutex lock.  Otherwise tearing artifacts might result.

//----------------------------------------------------------------------------
vtkVideoSource* vtkVideoSource::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVideoSource");
  if(ret)
    {
    return (vtkVideoSource*)ret;
    }

  return new vtkVideoSource;
}

//----------------------------------------------------------------------------
vtkVideoSource::vtkVideoSource()
{
  this->Initialized = 0;

  this->AutoAdvance = 1;

  this->FrameSize[0] = 320;
  this->FrameSize[1] = 240;
  this->FrameSize[2] = 1;

  this->Playing = 0;

  this->FrameRate = 30;

  this->GrabOnUpdate = 0;

  this->OutputNeedsInitialization = 1;
  this->FrameGrabbed = 0;

  this->OutputFormat = VTK_LUMINANCE;
  this->NumberOfScalarComponents = 1;

  this->NumberOfOutputFrames = 1;

  this->Preview = 0;
  this->Opacity = 1.0;

  int idx;
  for (idx = 0; idx < 3; idx++)
    {
    this->ClipRegion[idx*2] = 0;
    this->ClipRegion[idx*2+1] = VTK_INT_MAX;
    this->OutputWholeExtent[idx*2] = 0;
    this->OutputWholeExtent[idx*2+1] = -1;
    this->DataSpacing[idx] = 1.0;
    this->DataOrigin[idx] = 0.0;
    }

  for (idx = 0; idx < 6; idx++)
    {
    this->LastOutputExtent[idx] = 0;
    }
  this->LastNumberOfScalarComponents = 0;

  this->FlipFrames = 0;

  this->PlayerThreader = vtkMultiThreader::New();
  this->PlayerThreadId = -1;

  this->FrameBufferMutex = vtkMutexLock::New();

  this->FrameBufferSize = 0;
  this->FrameBuffer = NULL;
  this->FrameBufferTimeStamps = NULL;
  this->FrameBufferIndex = 0;
  this->SetFrameBufferSize(1);

  this->FrameBufferBitsPerPixel = 8;
  this->FrameBufferRowAlignment = 1;
}

//----------------------------------------------------------------------------
vtkVideoSource::~vtkVideoSource()
{ 
  // we certainly don't want to access a virtual 
  // function after the subclass has destructed!!
  this->vtkVideoSource::ReleaseSystemResources();

  this->SetFrameBufferSize(0);
  this->FrameBufferMutex->Delete();
  this->PlayerThreader->Delete();
}

//----------------------------------------------------------------------------
void vtkVideoSource::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkImageSource::PrintSelf(os,indent);
  
  os << indent << "FrameSize: (" << this->FrameSize[0] << ", " 
     << this->FrameSize[1] << ", " << this->FrameSize[2] << ")\n";

  os << indent << "ClipRegion: (" << this->ClipRegion[0];
  for (idx = 1; idx < 6; ++idx)
    {
    os << ", " << this->ClipRegion[idx];
    }
  os << ")\n";
  
  os << indent << "DataSpacing: (" << this->DataSpacing[0];
  for (idx = 1; idx < 3; ++idx)
    {
    os << ", " << this->DataSpacing[idx];
    }
  os << ")\n";
  
  os << indent << "DataOrigin: (" << this->DataOrigin[0];
  for (idx = 1; idx < 3; ++idx)
    {
    os << ", " << this->DataOrigin[idx];
    }
  os << ")\n";

  os << indent << "OutputFormat: " <<
    (this->OutputFormat == VTK_RGBA ? "RGBA" :
     (this->OutputFormat == VTK_RGB ? "RGB" :
      (this->OutputFormat == VTK_LUMINANCE_ALPHA ? "LuminanceAlpha" :
       (this->OutputFormat == VTK_LUMINANCE ? "Luminance" : "Unknown"))))
     << "\n";

  os << indent << "OutputWholeExtent: (" << this->OutputWholeExtent[0];
  for (idx = 1; idx < 6; ++idx)
    {
    os << ", " << this->OutputWholeExtent[idx];
    }
  os << ")\n";
  
  os << indent << "FrameRate: " << this->FrameRate << "\n";

  os << indent << "Playing: " << (this->Playing ? "On\n" : "Off\n");

  os << indent << "FrameBufferSize: " << this->FrameBufferSize << "\n";

  os << indent << "NumberOfOutputFrames: " << this->NumberOfOutputFrames << "\n";

  os << indent << "AutoAdvance: " << (this->AutoAdvance ? "On\n" : "Off\n");

  os << indent << "GrabOnUpdate: " << (this->GrabOnUpdate ? "On\n" : "Off\n");
  
  os << indent << "Opacity: " << this->Opacity << "\n";

  os << indent << "Preview: " << (this->Preview ? "On\n" : "Off\n");
}

//----------------------------------------------------------------------------
// Update the FrameBuffers according to any changes in the FrameBuffer*
// information. 
// This function should always be called from within a FrameBufferMutex lock
// and should never be called asynchronously.
// It sets up the FrameBufferExtent
void vtkVideoSource::UpdateFrameBuffer()
{
  int i, oldExt;
  int ext[3];
  vtkScalars *buffer;

  // clip the ClipRegion with the FrameSize
  for (i = 0; i < 3; i++)
    {
    oldExt = this->FrameBufferExtent[2*i+1] - this->FrameBufferExtent[2*i] + 1;
    this->FrameBufferExtent[2*i] = ((this->ClipRegion[2*i] > 0) 
			     ? this->ClipRegion[2*i] : 0);  
    this->FrameBufferExtent[2*i+1] = ((this->ClipRegion[2*i+1] < 
				       this->FrameSize[i]-1) 
			     ? this->ClipRegion[2*i+1] : this->FrameSize[i]-1);

    ext[i] = this->FrameBufferExtent[2*i+1] - this->FrameBufferExtent[2*i] + 1;
    if (ext[i] < 0)
      {
      this->FrameBufferExtent[2*i] = 0;
      this->FrameBufferExtent[2*i+1] = -1;
      ext[i] = 0;
      }

    if (oldExt > ext[i])
      { // dimensions of framebuffer changed
      this->OutputNeedsInitialization = 1;
      }
    }

  // total number of bytes required for the framebuffer
  int bytesPerRow = (ext[0]*this->FrameBufferBitsPerPixel+7)/8;
  bytesPerRow += bytesPerRow % this->FrameBufferRowAlignment;
  int totalSize = bytesPerRow * ext[1] * ext[2];

  i = this->FrameBufferSize;

  while (--i >= 0)
    {
    buffer = (vtkScalars *)this->FrameBuffer[i];
    if (buffer->GetDataType() != VTK_UNSIGNED_CHAR ||
	buffer->GetNumberOfComponents() != 1 ||
	buffer->GetNumberOfScalars() != totalSize)
      {
      buffer->SetDataTypeToUnsignedChar();
      buffer->SetNumberOfComponents(1);
      buffer->SetNumberOfScalars(totalSize);
      }
    }
}

//----------------------------------------------------------------------------
// Initialize() should be overridden to initialize the hardware frame grabber
void vtkVideoSource::Initialize()
{
  if (this->Initialized)
    {
    return;
    }
  this->Initialized = 1;

  this->UpdateFrameBuffer();
}

//----------------------------------------------------------------------------
// ReleaseSystemResources() should be overridden to release the hardware
void vtkVideoSource::ReleaseSystemResources()
{
  if (this->Playing)
    {
    this->Stop();
    }

  this->Initialized = 0;
}

//----------------------------------------------------------------------------
void vtkVideoSource::SetFrameSize(int x, int y, int z)
{
  if (x == this->FrameSize[0] && 
      y == this->FrameSize[1] && 
      z == this->FrameSize[2])
    {
    return;
    }

  if (x < 1 || y < 1 || z < 1) 
    {
    vtkErrorMacro(<< "SetFrameSize: Illegal frame size");
    return;
    }

  this->FrameSize[0] = x;
  this->FrameSize[1] = y;
  this->FrameSize[2] = z;

  if (this->Initialized) 
    {
    this->FrameBufferMutex->Lock();
    this->UpdateFrameBuffer();
    this->FrameBufferMutex->Unlock();
    }

  this->Modified();
}
    
//----------------------------------------------------------------------------
void vtkVideoSource::SetClipRegion(int x0, int x1, int y0, int y1, 
				   int z0, int z1)
{
  if (this->ClipRegion[0] != x0 || this->ClipRegion[1] != x1 ||
      this->ClipRegion[2] != y0 || this->ClipRegion[3] != y1 ||
      this->ClipRegion[4] != z0 || this->ClipRegion[5] != z1)
    {
    this->ClipRegion[0] = x0; this->ClipRegion[1] = x1;
    this->ClipRegion[2] = y0; this->ClipRegion[3] = y1;
    this->ClipRegion[4] = z0; this->ClipRegion[5] = z1;

    if (this->Initialized) 
      { // modify the FrameBufferExtent
      this->FrameBufferMutex->Lock();
      this->UpdateFrameBuffer();
      this->FrameBufferMutex->Unlock();
      }
    }
}

//----------------------------------------------------------------------------
// The grab function, which should (of course) be overridden to do
// the appropriate hardware stuff.  This function should never be
// called asynchronously.
void vtkVideoSource::Grab(int numFrames)
{
  if (numFrames < 1)
    {
    vtkErrorMacro(<< "Grab: # of frames must be at least 1");
    }

  // ensure that the hardware is initialized.
  this->Initialize();

  int f;
  for (f = 0; f < numFrames; f++) 
    {
    this->InternalGrab();
    }
}

//----------------------------------------------------------------------------
// Copy pseudo-random noise into the frames.  This function may be called
// asynchronously.
void vtkVideoSource::InternalGrab()
{
  int i,index;
  static int randsave = 0;
  int randNum;
  unsigned char *ptr;
  int *lptr;

  // get a thread lock on the frame buffer
  this->FrameBufferMutex->Lock();

  if (this->AutoAdvance)
    { 
    this->AdvanceFrameBuffer(1);
    }

  index = this->FrameBufferIndex % this->FrameBufferSize;
  while (index < 0)
    {
    index += this->FrameBufferSize;
    }

  int bytesPerRow = ((this->FrameBufferExtent[1]-this->FrameBufferExtent[0]+1)*
                     this->FrameBufferBitsPerPixel + 7)/8;
  bytesPerRow += bytesPerRow % this->FrameBufferRowAlignment;
  int totalSize = bytesPerRow * 
                   (this->FrameBufferExtent[3]-this->FrameBufferExtent[2]+1) *
                   (this->FrameBufferExtent[5]-this->FrameBufferExtent[4]+1);

  randNum = randsave;

  // copy 'noise' into the frame buffer
  ptr = (unsigned char *)((vtkScalars *)this->\
			  FrameBuffer[index])->GetVoidPointer(0);

  lptr = (int *)(((((long)ptr) + 3)/4)*4);
  i = totalSize/4;

  while (--i >= 0)
    {
    randNum = 1664525*randNum + 1013904223;
    *lptr++ = randNum;
    }
  unsigned char *ptr1 = ptr + 4;
  i = (totalSize-4)/16;
  while (--i >= 0)
    {
    randNum = 1664525*randNum + 1013904223;
    *ptr1 = randNum;
    ptr1 += 16;
    }
  randsave = randNum;

  this->FrameBufferTimeStamps[index] = vtkTimerLog::GetCurrentTime();

  this->Modified();

  this->FrameBufferMutex->Unlock();
}

//----------------------------------------------------------------------------
// this function runs in an alternate thread to do grabs

static void *vtkVideoSourceGrabThread(struct ThreadInfoStruct *data)
{
  vtkVideoSource *self = (vtkVideoSource *)(data->UserData);
 
  int activeFlag;

  double currentTime = vtkTimerLog::GetCurrentTime();
  for (;;)
    {
    // do our psudo-capture
    self->InternalGrab();
    
    // sleep until the next frame rolls around

    double oldtime = currentTime;
    //fprintf(stderr,"time: %10.6f\n",time);

    for (;;)
      {
      // check to see if we are being told to quit 
      data->ActiveFlagLock->Lock();
      activeFlag = *(data->ActiveFlag);
      data->ActiveFlagLock->Unlock();

      if (activeFlag == 0)
	{
	return NULL;
	}

      currentTime = vtkTimerLog::GetCurrentTime();
      // get the time remaining until the next frame
      float rate = self->GetFrameRate();
      double remaining = 0.1;
      if (rate > 0)
	{
	remaining = 1.0/rate - (currentTime - oldtime);
	}

      if (remaining < 0)
	{
	break;
	}
      // don't hold up other thread for more than 0.1 sec
      if (remaining > 0.1)
	{
	remaining = 0.1;
	}

      // sleep according to OS preference
#ifdef _WIN32
      // was having trouble with wish.exe stack overflows,
      // using Sleep() instead of vtkTimerLog::Sleep() seemed
      // to fix.
      Sleep((int)(1000*remaining));
#else
      vtkTimerLog::Sleep((int)(1000*remaining));
#endif
      }
    }
}

//----------------------------------------------------------------------------
// Set the source to grab continuously.  
// You should override this as appropriate for your device.  
void vtkVideoSource::Play()
{
  if (!this->Playing)
    {
    this->Initialize();

    this->Playing = 1;
    this->Modified();
    this->PlayerThreadId = 
      this->PlayerThreader->SpawnThread((vtkThreadFunctionType)\
					&vtkVideoSourceGrabThread,this);
    }
}
    
//----------------------------------------------------------------------------
// Stop continuous grabbing.  You will have to override this if your
// class overrides Play()
void vtkVideoSource::Stop()
{
  if (this->Playing)
    {
    this->PlayerThreader->TerminateThread(this->PlayerThreadId);
    this->PlayerThreadId = -1;
    this->Playing = 0;
    this->Modified();
    }
} 

//----------------------------------------------------------------------------
void vtkVideoSource::SetGrabOnUpdate(int yesno)
{
  if (this->GrabOnUpdate == yesno)
    {
    return;
    }

  this->FrameBufferMutex->Lock();
  this->GrabOnUpdate = yesno;
  this->FrameBufferMutex->Unlock();

  this->Modified();
}

//----------------------------------------------------------------------------
// Override this and provide checks to ensure an appropriate number
// of components was asked for (i.e. 1 for greyscale, 3 for RGB,
// or 4 for RGBA)
void vtkVideoSource::SetOutputFormat(int format)
{
  if (format == this->OutputFormat)
    {
    return;
    }

  this->OutputFormat = format;

  // convert color format to number of scalar components
  int numComponents;

  switch (this->OutputFormat)
    {
    case VTK_RGBA:
      numComponents = 4;
      break;
    case VTK_RGB:
      numComponents = 3;
      break;
    case VTK_LUMINANCE_ALPHA:
      numComponents = 2;
      break;
    case VTK_LUMINANCE:
      numComponents = 1;
      break;
    default:
      vtkErrorMacro(<< "SetOutputFormat: Unrecognized color format.");
      break;
    }
  this->NumberOfScalarComponents = numComponents;

  if (this->FrameBufferBitsPerPixel != numComponents*8)
    {
    this->FrameBufferMutex->Lock();
    this->FrameBufferBitsPerPixel = numComponents*8;
    if (this->Initialized)
      {
      this->UpdateFrameBuffer();
      }
    this->FrameBufferMutex->Unlock();
    }

  this->Modified();
}

//----------------------------------------------------------------------------
// set or change the circular buffer size
// you will have to override this if you want the buffers 
// to be device-specific (i.e. something other than vtkScalars)
void vtkVideoSource::SetFrameBufferSize(int bufsize)
{
  int i;
  void **framebuffer;
  double *timestamps;

  if (bufsize < 0)
    {
    vtkErrorMacro(<< "SetFrameBufferSize: There must be at least one framebuffer");
    }

  if (bufsize == this->FrameBufferSize)
    {
    return;
    }

  this->FrameBufferMutex->Lock();

  if (this->FrameBuffer == 0)
    {
    if (bufsize > 0)
      {
      this->FrameBufferIndex = 0;
      this->FrameBuffer = new void *[bufsize];
      this->FrameBufferTimeStamps = new double[bufsize];
      for (i = 0; i < bufsize; i++)
	{
	this->FrameBuffer[i] = (void *)vtkScalars::New();
	this->FrameBufferTimeStamps[i] = 0.0;
	} 
      this->FrameBufferSize = bufsize;
      this->Modified();
      }
    }
  else 
    {
    if (bufsize > 0)
      {
      framebuffer = new void *[bufsize];
      timestamps = new double[bufsize];
      }
    // create new image buffers if necessary
    for (i = 0; i < bufsize - this->FrameBufferSize; i++)
      {
      framebuffer[i] = (void *)vtkScalars::New();
      timestamps[i] = 0.0;
      }
    // copy over old image buffers
    for (; i < bufsize; i++)
      {
      framebuffer[i] = this->FrameBuffer[i-(bufsize-this->FrameBufferSize)];
      }

    // delete image buffers we no longer need
    for (i = 0; i < this->FrameBufferSize-bufsize; i++)
      {
      ((vtkScalars *)this->FrameBuffer[i])->Delete();
      }

    delete [] this->FrameBuffer;
    this->FrameBuffer = framebuffer;
    delete [] this->FrameBufferTimeStamps;
    this->FrameBufferTimeStamps = timestamps;

    if (bufsize > 0)
      {
      this->FrameBufferIndex = this->FrameBufferIndex % bufsize;
      }
    else
      {
      this->FrameBufferIndex = 0;
      }

    this->FrameBufferSize = bufsize;
    this->Modified();
    }

  if (this->Initialized)
    {
    this->UpdateFrameBuffer();
    }

  this->FrameBufferMutex->Unlock();
}

//----------------------------------------------------------------------------
// Rotate the buffers
void vtkVideoSource::Advance(int n)
{ 
  this->FrameBufferMutex->Lock();
  this->AdvanceFrameBuffer(n); 
  this->FrameBufferMutex->Unlock();
  this->Modified(); 
}

//----------------------------------------------------------------------------
// This function MUST be called only from within a FrameBufferMutex->Lock()
void vtkVideoSource::AdvanceFrameBuffer(int n)
{
  int i = (this->FrameBufferIndex - n) % this->FrameBufferSize;
  while (i < 0) 
    {
    i += this->FrameBufferSize;
    }
  this->FrameBufferIndex = i;
}

//----------------------------------------------------------------------------
double vtkVideoSource::GetFrameTimeStamp(int frame)
{ 
  double timeStamp;

  this->FrameBufferMutex->Lock();

  if (this->FrameBufferSize <= 0)
    {
    return 0.0;
    }

  timeStamp = this->FrameBufferTimeStamps[(this->FrameBufferIndex + frame) \
					 % this->FrameBufferSize];
  this->FrameBufferMutex->Unlock();

  return timeStamp;
}

//----------------------------------------------------------------------------
// This is a hack to force a grab on each update
// when this->GrabOnUpdate is set
void vtkVideoSource::UpdateInformation()
{
  if (this->GrabOnUpdate && !this->FrameGrabbed)
    {
    this->Grab();
    this->FrameGrabbed = 1;
    }

  this->vtkImageSource::UpdateInformation();
}

//----------------------------------------------------------------------------
// This method returns the largest data that can be generated.
void vtkVideoSource::ExecuteInformation()
{
  int i;
  int extent[6];

  // ensure that the hardware is initialized.
  this->Initialize();

  for (i = 0; i < 3; i++)
    {
    // initially set extent to the OutputWholeExtent
    extent[2*i] = this->OutputWholeExtent[2*i];
    extent[2*i+1] = this->OutputWholeExtent[2*i+1];
    // if 'flag' is set in output extent, use the FrameBufferExtent instead
    if (extent[2*i+1] < extent[2*i])
      {
      extent[2*i] = 0; 
      extent[2*i+1] = \
	this->FrameBufferExtent[2*i+1] - this->FrameBufferExtent[2*i];
      }
    this->FrameOutputExtent[2*i] = extent[2*i];
    this->FrameOutputExtent[2*i+1] = extent[2*i+1];
    }

  int numFrames = this->NumberOfOutputFrames;
  if (numFrames < 1)
    {
    numFrames = 1;
    }
  if (numFrames > this->FrameBufferSize)
    {
    numFrames = this->FrameBufferSize;
    }

  // multiply Z extent by number of frames to output
  extent[5] = extent[4] + (extent[5]-extent[4]+1) * numFrames - 1;

  this->GetOutput()->SetWholeExtent(extent);
    
  // set the spacing
  this->GetOutput()->SetSpacing(this->DataSpacing);

  // set the origin.
  this->GetOutput()->SetOrigin(this->DataOrigin);

  // set default data type (8 bit greyscale)
  this->GetOutput()->SetScalarType(VTK_UNSIGNED_CHAR);
  this->GetOutput()->SetNumberOfScalarComponents(this->NumberOfScalarComponents);
}

//----------------------------------------------------------------------------
// The UnpackRasterLine method should be overridden if the framebuffer uses
// unusual pixel packing formats, such as XRGB XBRG BGRX BGR etc.
// The version below assumes that the packing of the framebuffer is
// identical to that of the output.
void vtkVideoSource::UnpackRasterLine(char *outPtr, char *rowPtr, 
				      int start, int count)
{
  char *inPtr = rowPtr + start*this->NumberOfScalarComponents;
  memcpy(outPtr,inPtr,count*this->NumberOfScalarComponents);
  if (this->OutputFormat == VTK_RGBA)
    { // RGBA image: need to copy in the opacity
    unsigned char alpha = (unsigned char)(this->Opacity*255);
    int k;
    outPtr += 3;
    for (k = 0; k < count; k++)
      {
      outPtr[4*k] = alpha;
      }
    }
}

//----------------------------------------------------------------------------
// The Execute method is fairly complex, so I would not recommend overriding
// it unless you have to.  Override the UnpackRasterLine() method instead.
// You should only have to override it if you are using something other 
// than 8-bit vtkScalars for the frame buffer.
void vtkVideoSource::Execute(vtkImageData *data)
{
  int i,j;

  // state that we have 'used up' the frame which was just grabbed
  this->FrameGrabbed = 0;

  int outputExtent[6];     // will later be clipped in Z to a single frame
  int saveOutputExtent[6]; // will possibly contain multiple frames
  data->GetExtent(outputExtent);
  for (i = 0; i < 6; i++)
    {
    saveOutputExtent[i] = outputExtent[i];
    }
  // clip to extent to the Z size of one frame  
  outputExtent[4] = this->FrameOutputExtent[4]; 
  outputExtent[5] = this->FrameOutputExtent[5]; 

  int frameExtentX = this->FrameBufferExtent[1]-this->FrameBufferExtent[0]+1;
  int frameExtentY = this->FrameBufferExtent[3]-this->FrameBufferExtent[2]+1;
  int frameExtentZ = this->FrameBufferExtent[5]-this->FrameBufferExtent[4]+1;

  int extentX = outputExtent[1]-outputExtent[0]+1;
  int extentY = outputExtent[3]-outputExtent[2]+1;
  int extentZ = outputExtent[5]-outputExtent[4]+1;

  // if the output is more than a single frame,
  // then the output will cover a partial or full first frame,
  // several full frames, and a partial or full last frame

  // index and Z size of the first frame in the output extent
  int firstFrame = (saveOutputExtent[4]-outputExtent[4])/extentZ;
  int firstOutputExtent4 = saveOutputExtent[4] - extentZ*firstFrame;

  // index and Z size of the final frame in the output extent
  int finalFrame = (saveOutputExtent[5]-outputExtent[4])/extentZ;
  int finalOutputExtent5 = saveOutputExtent[5] - extentZ*finalFrame;

  char *outPtr = (char *)data->GetScalarPointer();
  char *outPtrTmp;

  int inIncY = (frameExtentX*this->FrameBufferBitsPerPixel + 7)/8;
  inIncY += inIncY % this->FrameBufferRowAlignment;
  int inIncZ = inIncY*frameExtentY;

  int outIncX = this->NumberOfScalarComponents;
  int outIncY = outIncX*extentX;
  int outIncZ = outIncY*extentY;

  int inPadX = 0;
  int inPadY = 0;
  int inPadZ; // do inPadZ later

  int outPadX = -outputExtent[0];
  int outPadY = -outputExtent[2];
  int outPadZ;  // do outPadZ later

  if (outPadX < 0)
    {
    inPadX -= outPadX;
    outPadX = 0;
    }

  if (outPadY < 0)
    {
    inPadY -= outPadY;
    outPadY = 0;
    }

  int outX = frameExtentX - inPadX; 
  int outY = frameExtentY - inPadY; 
  int outZ; // do outZ later

  if (outX > extentX - outPadX)
    {
    outX = extentX - outPadX;
    }
  if (outY > extentY - outPadY)
    {
    outY = extentY - outPadY;
    }

  // if output extent has changed, need to initialize output to black
  for (i = 0; i < 3; i++)
    {
    if (saveOutputExtent[i] != this->LastOutputExtent[i])
      {
      this->LastOutputExtent[i] = saveOutputExtent[i];
      this->OutputNeedsInitialization = 1;
      }
    }

  // ditto for number of scalar components
  if (data->GetNumberOfScalarComponents() != 
      this->LastNumberOfScalarComponents)
    {
    this->LastNumberOfScalarComponents = data->GetNumberOfScalarComponents();
    this->OutputNeedsInitialization = 1;
    }

  // initialize output to zero only when necessary
  if (this->OutputNeedsInitialization)
    {
    memset(outPtr,0,
	   (saveOutputExtent[1]-saveOutputExtent[0]+1)*
	   (saveOutputExtent[3]-saveOutputExtent[2]+1)*
	   (saveOutputExtent[5]-saveOutputExtent[4]+1)*outIncX);
    this->OutputNeedsInitialization = 0;
    } 

  // we have to modify the outputExtent of the first frame,
  // because it might be complete (it will be restored after
  // the first frame has been copied to the output)
  int saveOutputExtent4 = outputExtent[4];
  outputExtent[4] = firstOutputExtent4;

  this->FrameBufferMutex->Lock();

  int index = this->FrameBufferIndex;
  int frame;
  for (frame = firstFrame; frame <= finalFrame; frame++)
    {
    if (frame == finalFrame)
      {
      outputExtent[5] = finalOutputExtent5;
      } 
    
    vtkScalars *frameBuffer = (vtkScalars *) \
      this->FrameBuffer[(index + frame) % this->FrameBufferSize];

    char *inPtr = (char *)frameBuffer->GetVoidPointer(0);
    char *inPtrTmp;

    extentZ = outputExtent[5]-outputExtent[4]+1;
    inPadZ = 0;
    outPadZ = -outputExtent[4];
    
    if (outPadZ < 0)
      {
      inPadZ -= outPadZ;
      outPadZ = 0;
      }

    outZ = frameExtentZ - inPadZ;

    if (outZ > extentZ - outPadZ)
      {
      outZ = extentZ - outPadZ;
      }

    if (this->FlipFrames)
      { // apply a vertical flip while copying to output
      outPtr += outIncZ*outPadZ+outIncY*outPadY+outIncX*outPadX;
      inPtr += inIncZ*inPadZ+inIncY*inPadY;

      for (i = 0; i < outZ; i++)
	{
        inPtrTmp = inPtr + inIncY*(this->FrameOutputExtent[3]-outputExtent[3]);
	outPtrTmp = outPtr + outIncY*(extentY - 2*outPadY);
	for (j = 0; j < outY; j++)
	  {
	  outPtrTmp -= outIncY;
	  if (outX > 0)
	    {
	    this->UnpackRasterLine(outPtrTmp,inPtrTmp,inPadX,outX);
	    }
	  inPtrTmp += inIncY;
	  }
	outPtr += outIncZ;
	inPtr += inIncZ;
	}
      }
    else
      { // don't apply a vertical flip
      outPtr += outIncZ*outPadZ+outIncY*outPadY+outIncX*outPadX;
      inPtr += inIncZ*inPadZ+inIncY*inPadY;

      for (i = 0; i < outZ; i++)
	{
	inPtrTmp = inPtr;
	outPtrTmp = outPtr;
	for (j = 0; j < outY; j++)
	  {
	  if (outX > 0) 
	    {
	    this->UnpackRasterLine(outPtrTmp,inPtrTmp,inPadX,outX);
	    }
	  outPtrTmp += outIncY;
	  inPtrTmp += inIncY;
	  }
	outPtr += outIncZ;
	inPtr += inIncZ;
	}
      }
    // restore the output extent once the first frame is done
    outputExtent[4] = saveOutputExtent4;
    }

  this->FrameBufferMutex->Unlock();
}
