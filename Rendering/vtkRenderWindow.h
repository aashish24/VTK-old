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
// .NAME vtkRenderWindow - create a window for renderers to draw into
// .SECTION Description
// vtkRenderWindow is an abstract object to specify the behavior of a
// rendering window. A rendering window is a window in a graphical user
// interface where renderers draw their images. Methods are provided to 
// synchronize the rendering process, set window size, and control double
// buffering. 

// .SECTION see also
// vtkRenderer vtkRenderMaster vtkRenderWindowInteractor

#ifndef __vtkRenderWindow_hh
#define __vtkRenderWindow_hh

#include "vtkObject.hh"
#include "vtkRendererCollection.hh"

class vtkRenderWindowInteractor;
class vtkLightDevice;
class vtkCameraDevice;
class vtkActorDevice;
class vtkTextureDevice;
class vtkPropertyDevice;

// lets define the diferent types of stereo
#define VTK_STEREO_CRYSTAL_EYES 1
#define VTK_STEREO_RED_BLUE     2

class vtkRenderWindow : public vtkObject
{
public:
  vtkRenderWindow();
  ~vtkRenderWindow();
  char *GetClassName() {return "vtkRenderWindow";};
  void PrintSelf(ostream& os, vtkIndent indent);

  void AddRenderers(vtkRenderer *);
  void RemoveRenderers(vtkRenderer *);
  vtkRendererCollection *GetRenderers() {return &(this->Renderers);};

  virtual void Render();

  // Description:
  // Initialize the rendering process.
  virtual void Start() = 0;

  // Description:
  // A termination method performed at the end of the rendering process
  // to do things like swapping buffers (if necessary) or similar actions.
  virtual void Frame() = 0;

  virtual void SetDisplayId(void *) = 0;
  virtual void SetWindowId(void *) = 0;

  // Description:
  // Performed at the end of the rendering process to generate image.
  // This is typically done right before swapping buffers.
  virtual void CopyResultFrame();

  // Description:
  // Create a device specific renderer. This is the only way to create
  // a renderer that will work. This method is implemented in the
  // subclasses of vtkRenderWindow so that each subclass will return
  // the correct renderer for its graphics library.
  virtual vtkRenderer  *MakeRenderer() = 0;

  // Description:
  // Create a device specific light. This is used by vtkLight to create
  // the correct type of vtkLightDevice.
  virtual vtkLightDevice *MakeLight() = 0;

  // Description:
  // Create a device specific camera. This is used by vtkCamera to create
  // the correct type of vtkCameraDevice.
  virtual vtkCameraDevice    *MakeCamera() = 0;

  // Description:
  // Create a device specific actor. This is used by vtkActor to create
  // the correct type of vtkActorDevice.
  virtual vtkActorDevice    *MakeActor() = 0;

  // Description:
  // Create a device specific property. This is used by vtkProperty to create
  // the correct type of vtkPropertyDevice.
  virtual vtkPropertyDevice    *MakeProperty() = 0;

  // Description:
  // Create a device specific texture. This is used by vtkTexture to create
  // the correct type of vtkTextureDevice.
  virtual vtkTextureDevice     *MakeTexture() = 0;

  // Description:
  // Create an interactor to control renderers in this window. We need
  // to know what type of interactor to create, because we might be in
  // X Windows or MS Windows. 
  virtual vtkRenderWindowInteractor *MakeRenderWindowInteractor() = 0;

  // Description:
  // Set/Get the position in screen coordinates of the rendering window.
  virtual int *GetPosition() = 0;
  virtual void SetPosition(int,int);
  virtual void SetPosition(int a[2]);

  // Description:
  // Set/Get the size of the window in screen coordinates.
  virtual int *GetSize() = 0;
  virtual void SetSize(int,int) = 0;
  virtual void SetSize(int a[2]);

  // Description:
  // Turn on/off rendering full screen window size.
  virtual void SetFullScreen(int) = 0;
  vtkGetMacro(FullScreen,int);
  vtkBooleanMacro(FullScreen,int);

  // Description:
  // Turn on/off window manager borders. Typically, you shouldn't turn the 
  // borders off, because that bypasses the window manager and can cause
  // undesirable behavior.
  vtkSetMacro(Borders,int);
  vtkGetMacro(Borders,int);
  vtkBooleanMacro(Borders,int);

  // Description:
  // Keep track of whether the rendering window has been mapped to screen.
  vtkSetMacro(Mapped,int);
  vtkGetMacro(Mapped,int);
  vtkBooleanMacro(Mapped,int);

  // Description:
  // Turn on/off double buffering.
  vtkSetMacro(DoubleBuffer,int);
  vtkGetMacro(DoubleBuffer,int);
  vtkBooleanMacro(DoubleBuffer,int);

  // Description:
  // Turn on/off stereo rendering.
  vtkGetMacro(StereoRender,int);
  vtkSetMacro(StereoRender,int);
  vtkBooleanMacro(StereoRender,int);

  // Description:
  // Set/Get what type of stereo rendering to use.
  vtkGetMacro(StereoType,int);
  vtkSetMacro(StereoType,int);

  virtual void StereoUpdate();
  virtual void StereoMidpoint();
  virtual void StereoRenderComplete();

  virtual int  GetRemapWindow();
  virtual void WindowRemap() {};
  
  // Description:
  // Turn on/off erasing the screen between images. This allows multiple 
  // exposure sequences if turned on. You will need to turn double 
  // buffering off or make use of the SwapBuffers methods to prevent
  // you from swapping buffers between exposures.
  vtkSetMacro(Erase,int);
  vtkGetMacro(Erase,int);
  vtkBooleanMacro(Erase,int);

  // Description:
  // Turn on/off buffer swapping between images. 
  vtkSetMacro(SwapBuffers,int);
  vtkGetMacro(SwapBuffers,int);
  vtkBooleanMacro(SwapBuffers,int);
  
  // Description:
  // Get name of rendering window
  vtkGetStringMacro(Name);

  // Description:
  // Set/Get the filename used for saving images. See the SaveImageAsPPM 
  // method.
  vtkSetStringMacro(Filename);
  vtkGetStringMacro(Filename);


  // Description:
  // Save the current image as a PPM file.
  virtual void SaveImageAsPPM();


  // Description:
  // Set/Get the pixel data of an image, transmitted as RGBRGBRGB. The
  // front argument indicates if the front buffer should be used or the back 
  // buffer. It is the caller's responsibility to delete the resulting 
  // array. It is very important to realize that the memory in this array
  // is organized from the bottom of the window to the top. The origin
  // of the screen is in the lower left corner. The y axis increases as
  // you go up the screen. So the storage of pixels is from left to right
  // and from bottom to top.
  virtual unsigned char *GetPixelData(int x,int y,int x2,int y2,int front) = 0;
  virtual void SetPixelData(int x,int y,int x2,int y2,unsigned char *,int front) = 0;

  // Description:
  // Set the number of frames for doing antialiasing. The default is
  // zero. Typically five or six will yield reasonable results without
  // taking too long.
  vtkGetMacro(AAFrames,int);
  vtkSetMacro(AAFrames,int);

  // Description:
  // Set the number of frames for doing focal depth. The default is zero.
  // Depending on how your scene is organized you can get away with as
  // few as four frames for focal depth or you might need thirty.
  // One thing to note is that if you are using focal depth frames,
  // then you will not need many (if any) frames for antialiasing. 
  vtkGetMacro(FDFrames,int);
  vtkSetMacro(FDFrames,int);

  // Description:
  // Set the number of sub frames for doing motion blur. The default is zero.
  // Once this is set greater than one, you will no longer see a new frame
  // for every Render().  If you set this to five, you will need to do 
  // five Render() invocations before seeing the result. This isn't
  // very impressive unless something is changing between the Renders.
  vtkGetMacro(SubFrames,int);
  vtkSetMacro(SubFrames,int);

  // Description:
  // Set/Get the desired update rate. This is used with
  // the vtkLODActor class. When using level of detail actors you
  // need to specify what update rate you require. The LODActors then
  // will pick the correct resolution to meet your desired update rate
  // in frames per second. A value of zero indicates that they can use
  // all the time they want to.
  void SetDesiredUpdateRate(float);
  vtkGetMacro(DesiredUpdateRate,float);

protected:
  virtual void DoStereoRender();
  virtual void DoFDRender();
  virtual void DoAARender();

  vtkRendererCollection Renderers;
  char Name[80];
  int Size[2];
  int Position[2];
  int Borders;
  int FullScreen;
  int OldScreen[5];
  int Mapped;
  int DoubleBuffer;
  int StereoRender;
  int StereoType;
  int StereoStatus; // used for keeping track of what's going on
  vtkRenderWindowInteractor *Interactor;
  char *Filename;
  unsigned char* StereoBuffer; // used for red blue stereo
  float *AccumulationBuffer;   // used for many techniques
  int AAFrames;
  int FDFrames;
  int SubFrames;               // number of sub frames
  int CurrentSubFrame;         // what one are we on
  unsigned char* ResultFrame;  // used for any non immediate rendering
  int   Erase;
  int   SwapBuffers;
  float DesiredUpdateRate;
};

#endif


