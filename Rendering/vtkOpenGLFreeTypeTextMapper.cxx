/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkOpenGLFreeTypeTextMapper.h"
#include "vtkObjectFactory.h"
#include "vtkgluPickMatrix.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkViewport.h"

#include <sys/stat.h>

#include "vtkfreetypeConfig.h"
#include "vtkftglConfig.h"
#include "FTLibrary.h"
#include "NoSTL/FTCallbackVector.h"
#include "FTGLPixmapFont.h" // Use pixmaps for antialiased fonts
#include "FTGLBitmapFont.h" // Use bitmaps for normal (jaggy but faster) fonts

//----------------------------------------------------------------------------
// Print debug info

#define VTK_FTTM_DEBUG 0
#define VTK_FTTM_DEBUG_CD 0

// Cache by RBGA is nasty, but this is the way to go at the moment for pixmaps.
// This will cache a font for each new text property color, where each color
// component is discretized to 0..255.
// The reason for that is that pixmaps fonts use glDrawPixels() which does not
// honor glColor* settings. GL_*_SCALE and GL_*_BIAS could be used to
// scale and shift the color of each pixels, but this is awfully slow.

#define VTK_FTTM_CACHE_BY_RGBA 1

// Reorder most recently used

#define VTK_FTTM_REORDER 1

// Font cache capacity

#define VTK_FTTM_CACHE_CAPACITY 150

//----------------------------------------------------------------------------
// The embedded fonts
// Create a lookup table between the text mapper attributes 
// and the font buffers.

#include "fonts/vtkEmbeddedFonts.h"

struct EmbeddedFontStruct
{
  size_t length;
  unsigned char* ptr;
};

// Fonts, organized by [Family][Bold][Italic]
 
static EmbeddedFontStruct EmbeddedFonts[3][2][2] = 
{
  {
    {
      { // VTK_ARIAL: Bold [ ] Italic [ ]
        
        face_arial_buffer_length, face_arial_buffer
      },
      { // VTK_ARIAL: Bold [ ] Italic [x]
        face_arial_italic_buffer_length, face_arial_italic_buffer
      }
    },
    {
      { // VTK_ARIAL: Bold [x] Italic [ ]
        face_arial_bold_buffer_length, face_arial_bold_buffer
      },
      { // VTK_ARIAL: Bold [x] Italic [x]
        face_arial_bold_italic_buffer_length, face_arial_bold_italic_buffer
      }
    }
  },
  {
    {
      { // VTK_COURIER: Bold [ ] Italic [ ]
        face_courier_buffer_length, face_courier_buffer
      },
      { // VTK_COURIER: Bold [ ] Italic [x]
        face_courier_italic_buffer_length, face_courier_italic_buffer
      }
    },
    {
      { // VTK_COURIER: Bold [x] Italic [ ]
        face_courier_bold_buffer_length, face_courier_bold_buffer
      },
      { // VTK_COURIER: Bold [x] Italic [x]
        face_courier_bold_italic_buffer_length, face_courier_bold_italic_buffer
      }
    }
  },
  {
    {
      { // VTK_TIMES: Bold [ ] Italic [ ]
        face_times_buffer_length, face_times_buffer
      },
      { // VTK_TIMES: Bold [ ] Italic [x]
        face_times_italic_buffer_length, face_times_italic_buffer
      }
    },
    {
      { // VTK_TIMES: Bold [x] Italic [ ]
        face_times_bold_buffer_length, face_times_bold_buffer
      },
      { // VTK_TIMES: Bold [x] Italic [x]
        face_times_bold_italic_buffer_length, face_times_bold_italic_buffer
      }
    }
  }
};

//----------------------------------------------------------------------------
// Is antialiasing requested by a text prop

int IsAntiAliasingRequestedByThisProperty(vtkTextProperty *tprop)
{
  return
    (tprop->GetGlobalAntiAliasing() == VTK_TEXT_GLOBAL_ANTIALIASING_ALL || 
     (tprop->GetGlobalAntiAliasing() == VTK_TEXT_GLOBAL_ANTIALIASING_SOME 
      && tprop->GetAntiAliasing())) ? 1 : 0;
}

//----------------------------------------------------------------------------
// A cache

// Singleton cleanup

class vtkFontCacheCleanup
{
public:
  vtkFontCacheCleanup();
  ~vtkFontCacheCleanup();
};

// Singleton font cache

class vtkFontCache
{
public:

  // Cache entry

  struct Entry
  {
    int FontFamily;
    int Bold;
    int Italic;
    int AntiAliasing;
    int FontSize;
#if VTK_FTTM_CACHE_BY_RGBA
    unsigned char Red;
    unsigned char Green;
    unsigned char Blue;
    unsigned char Alpha;
#endif
    FTFont *Font;
    char *FaceFileName;
    float LargestAscender;
    float LargestDescender;
  };

  static vtkFontCache* GetInstance();
  static void SetInstance(vtkFontCache *instance);

  vtkFontCache::Entry* GetFont(vtkTextProperty *tprop, 
                               int override_color = 0,
                               unsigned char red = 0,
                               unsigned char green = 0,
                               unsigned char blue = 0);

private:

  vtkFontCache();
  ~vtkFontCache();

  static vtkFontCacheCleanup Cleanup;
  static vtkFontCache* Instance;

  void PrintEntry(int i, char *msg = 0);
  void ReleaseEntry(int i);

  void InitializeCache();
  void ReleaseCache();

  Entry *Entries[VTK_FTTM_CACHE_CAPACITY];
  int NumberOfEntries;
};

// This callback will be called by the FTGLibrary singleton cleanup destructor
// if it happens to be destroyed before our singleton (this order is not 
// deterministic). It will destroy our singleton, if needed.

void vtkFontCacheCleanupCallback ()
{
#if VTK_FTTM_DEBUG_CD
  printf("vtkFontCacheCleanupCallback\n");
#endif
  vtkFontCache::SetInstance(0);
}

// The singleton, and the singleton cleanup

vtkFontCache* vtkFontCache::Instance = 0;
vtkFontCacheCleanup vtkFontCache::Cleanup;

// Create the singleton cleanup
// Register our singleton cleanup callback against the FTLibrary so that
// it might be called before the FTLibrary singleton is destroyed.

vtkFontCacheCleanup::vtkFontCacheCleanup()
{
#if VTK_FTTM_DEBUG_CD
  printf("vtkFontCacheCleanup::vtkFontCacheCleanup\n");
#endif
  FTLibraryCleanup::AddDependency(&vtkFontCacheCleanupCallback);
}

// Delete the singleton cleanup 
// The callback called here might have been called by the FTLibrary singleton
// cleanup first (depending on the destruction order), but in case ours is
// destroyed first, let's call it too.

vtkFontCacheCleanup::~vtkFontCacheCleanup()
{
#if VTK_FTTM_DEBUG_CD
  printf("vtkFontCacheCleanup::~vtkFontCacheCleanup\n");
#endif
  vtkFontCacheCleanupCallback();
}

// (static) Return the single instance

vtkFontCache* vtkFontCache::GetInstance()
{
  if(!vtkFontCache::Instance)
    {
    vtkFontCache::Instance = new vtkFontCache;
    }
  return vtkFontCache::Instance;
}

// (static) Set the singleton instance

void vtkFontCache::SetInstance(vtkFontCache* instance)
{
  if (vtkFontCache::Instance == instance)
    {
    return;
    }

  if (vtkFontCache::Instance)
    {
    delete vtkFontCache::Instance;
    }

  vtkFontCache::Instance = instance;
}

// Create/Delete cache

vtkFontCache::vtkFontCache() 
{
#if VTK_FTTM_DEBUG_CD
  printf("vtkFontCache::vtkFontCache\n");
#endif
  this->NumberOfEntries = 0;
  this->InitializeCache();
}

vtkFontCache::~vtkFontCache() 
{
#if VTK_FTTM_DEBUG_CD
  printf("vtkFontCache::~vtkFontCache\n");
#endif
  this->ReleaseCache();
}

// Print entry

void vtkFontCache::PrintEntry(int i, char *msg) 
{
  if (!this->Entries[i])
    {
    return;
    }

    printf("%s: [%2d] =", msg, i);

    printf(" [S: %2d]", this->Entries[i]->FontSize);

#if VTK_FTTM_CACHE_BY_RGBA
    printf(" [RGBA: %2X/%2X/%2X (%2X)]", 
           this->Entries[i]->Red, 
           this->Entries[i]->Green, 
           this->Entries[i]->Blue, 
           this->Entries[i]->Alpha);
#endif

  if (this->Entries[i]->FaceFileName)
    {
    printf(" [F: %s]", this->Entries[i]->FaceFileName);
    }
  else
    {
    printf(" [F: %d] [I: %d] [B: %d]", 
           this->Entries[i]->FontFamily, 
           this->Entries[i]->Italic, 
           this->Entries[i]->Bold);
    }

  if (this->Entries[i]->Font)
    {
    printf(" [F: %p]", this->Entries[i]->Font);
    printf("\n                                                [f: %p]", *(this->Entries[i]->Font->Face()->Face()));
    }
  
  printf("\n");
  fflush(stdout);
}

// Release entry

void vtkFontCache::ReleaseEntry(int i) 
{
  if (!this->Entries[i])
    {
    return;
    }

#if VTK_FTTM_DEBUG
  this->PrintEntry(this->NumberOfEntries, "Rl");
#endif

  if (this->Entries[i]->Font)
    {
    delete this->Entries[i]->Font;
    this->Entries[i]->Font = 0;
    }
  
  if (this->Entries[i]->FaceFileName)
    {
    delete [] this->Entries[i]->FaceFileName;
    this->Entries[i]->FaceFileName = 0;
    }

  delete this->Entries[i];
  this->Entries[i] = 0;
}

// Initialize cache

void vtkFontCache::InitializeCache() 
{
#if VTK_FTTM_DEBUG_CD
  printf("vtkFontCache::InitializeCache()\n");
#endif  
  this->ReleaseCache();

  int i;
  for (i = 0; i < VTK_FTTM_CACHE_CAPACITY; i++)
    {
    this->Entries[i] = 0;
    }
}

// Release cache

void vtkFontCache::ReleaseCache() 
{
#if VTK_FTTM_DEBUG_CD
  printf("vtkFontCache::ReleaseCache()\n");
#endif  

  int i;
  for (i = 0; i < this->NumberOfEntries; i++)
    {
#if VTK_FTTM_DEBUG
    this->PrintEntry(i, "Rl");
#endif
    this->ReleaseEntry(i);
    }

  this->NumberOfEntries = 0;
}

// Get a font from the cache given the text property. If no font is
// found in the cache, one is created and stored with the given color
// parameters.  If AntiAliasing is Off, the font is a bitmap, thus
// color are not used in the cache (since glBitmap honors glColor*) If
// override_color is true, then red, green, blue are used as text color
// instead of the colors found in the vtkTextProperty.

vtkFontCache::Entry* vtkFontCache::GetFont(vtkTextProperty *tprop, 
                                           int override_color,
                                           unsigned char red,
                                           unsigned char green,
                                           unsigned char blue)
{
  int i;
#if VTK_FTTM_REORDER
  int j;
#endif

  int antialiasing_requested = IsAntiAliasingRequestedByThisProperty(tprop);

#if VTK_FTTM_CACHE_BY_RGBA
  float opacity = tprop->GetOpacity();
  unsigned char alpha = (opacity < 0.0) ? 255 : (unsigned char)(opacity*255.0);
  if (!override_color)
    {
    float* tpropColor = tprop->GetColor();
    red   = (tpropColor[0] < 0.0) ? 0 : (unsigned char)(tpropColor[0] * 255.0);
    green = (tpropColor[1] < 0.0) ? 0 : (unsigned char)(tpropColor[1] * 255.0);
    blue  = (tpropColor[2] < 0.0) ? 0 : (unsigned char)(tpropColor[2] * 255.0);
    }
#endif
  
  // Has the font been cached ?
  
  for (i = 0; i < this->NumberOfEntries; i++)
    {
    if (
      // If a face file name has been specified, it overrides the 
      // font family as well as italic and bold attributes

      ((!tprop->GetFaceFileName() && 
        !this->Entries[i]->FaceFileName &&
        this->Entries[i]->FontFamily == tprop->GetFontFamily() &&
        this->Entries[i]->Italic     == tprop->GetItalic() &&
        this->Entries[i]->Bold       == tprop->GetBold()) ||

       (tprop->GetFaceFileName() && 
        this->Entries[i]->FaceFileName && 
        !strcmp(tprop->GetFaceFileName(), this->Entries[i]->FaceFileName))) &&

      this->Entries[i]->AntiAliasing == antialiasing_requested

#if VTK_FTTM_CACHE_BY_RGBA
      && (!antialiasing_requested ||
          (this->Entries[i]->Red     == red &&
           this->Entries[i]->Green   == green &&
           this->Entries[i]->Blue    == blue &&
           this->Entries[i]->Alpha   == alpha))
#endif
      && this->Entries[i]->FontSize  == tprop->GetFontSize())
      {
#if VTK_FTTM_REORDER
      // Make this the most recently used
      if (i != 0)
        {
        vtkFontCache::Entry *tmp = this->Entries[i];
        for (j = i - 1; j >= 0; j--)
          {
          this->Entries[j+1] = this->Entries[j];
          }
        this->Entries[0] = tmp;
        }
      return this->Entries[0];
#else
      return this->Entries[i];
#endif
      }
    }

  // OK the font is not cached, try to create one

  FTFont *font;
  if (antialiasing_requested)
    {
    font = new FTGLPixmapFont;
    }
  else
    {
    font = new FTGLBitmapFont;
    }

  // A face file name has been provided, try to load it, otherwise
  // just use the embedded fonts (i.e. font family, bold and italic attribs)

  if (tprop->GetFaceFileName())
    {
    if (!font->Open(tprop->GetFaceFileName(), false))
      {
      vtkErrorWithObjectMacro(tprop,<< "Unable to load font " << tprop->GetFaceFileName());
      delete font;
      return 0;
      }
    // Try to load an AFM metrics file for the PFB/PFA Postscript fonts
    int length = strlen(tprop->GetFaceFileName());
    if (length > 4 &&
        (!strcmp(tprop->GetFaceFileName() + length - 4, ".pfb") ||
         !strcmp(tprop->GetFaceFileName() + length - 4, ".pfa")))
      {
      char *metrics = new char[length + 1];
      strncpy(metrics, tprop->GetFaceFileName(), length - 3);
      strcpy(metrics + length - 3, "afm");
      struct stat fs;
      if (stat(metrics, &fs) == 0)
        {
        font->Attach(metrics); 
        }
      delete [] metrics;
      }
    }
  else
    {
    if (!font->Open(
      EmbeddedFonts[tprop->GetFontFamily()][tprop->GetBold()][tprop->GetItalic()].ptr, 
      EmbeddedFonts[tprop->GetFontFamily()][tprop->GetBold()][tprop->GetItalic()].length,
      false))
      {
      vtkErrorWithObjectMacro(tprop,<< "Unable to create font !");
      delete font;
      return 0;
      }
    }

  // Set face size

  font->FaceSize(tprop->GetFontSize());
  
  // We need to make room for a new font

  if (this->NumberOfEntries == VTK_FTTM_CACHE_CAPACITY)
    {
#if VTK_FTTM_DEBUG
    printf("Cache is full, deleting last!\n");
#endif
    this->NumberOfEntries--;
    }

  // Add the new font

  if (this->Entries[this->NumberOfEntries])
    {
    this->ReleaseEntry(this->NumberOfEntries);
    }
  this->Entries[this->NumberOfEntries] = new vtkFontCache::Entry;
  
  // Set the other info

  if (tprop->GetFaceFileName())
    {
    this->Entries[this->NumberOfEntries]->FaceFileName = 
      new char [strlen(tprop->GetFaceFileName()) + 1];
    strcpy(this->Entries[this->NumberOfEntries]->FaceFileName, 
           tprop->GetFaceFileName());
    }
  else
    {
    this->Entries[this->NumberOfEntries]->FontFamily  = tprop->GetFontFamily();
    this->Entries[this->NumberOfEntries]->Bold        = tprop->GetBold();
    this->Entries[this->NumberOfEntries]->Italic      = tprop->GetItalic();
    this->Entries[this->NumberOfEntries]->FaceFileName = 0;
    }

  this->Entries[this->NumberOfEntries]->AntiAliasing  = antialiasing_requested;
  this->Entries[this->NumberOfEntries]->FontSize      = tprop->GetFontSize();

#if VTK_FTTM_CACHE_BY_RGBA
  if (antialiasing_requested)
    {
    this->Entries[this->NumberOfEntries]->Red         = red;
    this->Entries[this->NumberOfEntries]->Green       = green;
    this->Entries[this->NumberOfEntries]->Blue        = blue;
    this->Entries[this->NumberOfEntries]->Alpha       = alpha;
    }
#endif

  this->Entries[this->NumberOfEntries]->Font          = font;

  this->Entries[this->NumberOfEntries]->LargestAscender  =
  this->Entries[this->NumberOfEntries]->LargestDescender = -1;

#if VTK_FTTM_DEBUG
  this->PrintEntry(this->NumberOfEntries, "Cr");
#endif

  vtkFontCache::Entry *tmp = this->Entries[this->NumberOfEntries];

#if VTK_FTTM_DO_NOT_REORDER
  // Now resort the list

  for (i = this->NumberOfEntries - 1; i >= 0; i--)
    {
    this->Entries[i+1] = this->Entries[i];
    }
  this->Entries[0] = tmp;
#endif

  this->NumberOfEntries++;
  return tmp;
}

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkOpenGLFreeTypeTextMapper, "$Revision$");
vtkStandardNewMacro(vtkOpenGLFreeTypeTextMapper);

//----------------------------------------------------------------------------
vtkOpenGLFreeTypeTextMapper::vtkOpenGLFreeTypeTextMapper()
{
  this->LastSize[0] = 0;
  this->LastSize[1] = 0;
}

//----------------------------------------------------------------------------
vtkOpenGLFreeTypeTextMapper::~vtkOpenGLFreeTypeTextMapper()
{
  if (this->LastWindow)
    {
    this->ReleaseGraphicsResources(this->LastWindow);
    }  
}

//----------------------------------------------------------------------------
void vtkOpenGLFreeTypeTextMapper::ReleaseGraphicsResources(vtkWindow *vtkNotUsed(win))
{
#if VTK_FTTM_DEBUG
    printf("vtkOpenGLFreeTypeTextMapper::ReleaseGraphicsResources\n");
#endif
  
  this->LastWindow = NULL;
  
  // Very important
  // the release of graphics resources indicates that significant changes have
  // occurred. Old fonts, cached sizes etc are all no longer valid, so we send
  // ourselves a general modified message.

  // this->Modified();
}

//----------------------------------------------------------------------------
void vtkOpenGLFreeTypeTextMapper::GetSize(vtkViewport* viewport, int *size)
{
  // Check for multiline

  if (this->NumberOfLines > 1)
    {
    this->GetMultiLineSize(viewport, size);
    return;
    }

  // Check for input

  if (this->Input == NULL || this->Input[0] == '\0') 
    {
    size[0] = size[1] = 0;
    return;
    }

  vtkTextProperty *tprop = this->GetTextProperty();
  if (!tprop)
    {
    vtkErrorMacro(<< "Need a text property to get size");
    size[0] = size[1] = 0;
    return;
    }

  // Check to see whether we have to rebuild anything

  if (this->GetMTime() < this->SizeBuildTime &&
      tprop->GetMTime() < this->SizeBuildTime)
    {
#if VTK_FTTM_DEBUG
  printf("vtkOpenGLFreeTypeTextMapper::GetSize: In cache!\n");
#endif

    size[0] = this->LastSize[0];
    size[1] = this->LastSize[1];
    return;
    }

  // Check for font and try to set the size

  vtkFontCache::Entry *entry = vtkFontCache::GetInstance()->GetFont(tprop);
  FTFont *font = entry->Font;
  if (!font) 
    {
    vtkErrorMacro(<< "Render - No font");
    size[0] = size[1] = 0;
    return;
    }
  
  // The font global ascender and descender might just be too high
  // for given a face. Let's get a compromise by computing these values
  // from some usual ascii chars.
  
  if (entry->LargestAscender < 0 || entry->LargestDescender < 0)
    {
    float llx, lly, llz, urx, ury, urz;
    font->BBox("_/7Agfy", llx, lly, llz, urx, ury, urz);
    entry->LargestAscender = ury;
    entry->LargestDescender = lly;
    }
  
  this->LastSize[0] = size[0] = (int)font->Advance(this->Input);
  this->LastSize[1] = size[1] =
    (int)(entry->LargestAscender - entry->LargestDescender);
  this->LastLargestDescender = (int)entry->LargestDescender;

  this->SizeBuildTime.Modified();
}

//----------------------------------------------------------------------------
void vtkOpenGLFreeTypeTextMapper::RenderOverlay(vtkViewport* viewport, 
                                                vtkActor2D* actor)
{
  vtkDebugMacro (<< "RenderOverlay");

  // Check for input

  if (this->Input == NULL || this->Input[0] == '\0') 
    {
    return;
    }

  // Check for multi-lines

  if (this->NumberOfLines > 1)
    {
    this->RenderOverlayMultipleLines(viewport, actor);
    return;
    }

  // Get text property

  vtkTextProperty *tprop = this->GetTextProperty();
  if (!tprop)
    {
    vtkErrorMacro(<< "Need a text property to render mapper");
    return;
    }

  // Get the window information for display

  vtkWindow* window = viewport->GetVTKWindow();
  if (this->LastWindow && this->LastWindow != window)
    {
    this->ReleaseGraphicsResources(this->LastWindow);
    }
  this->LastWindow = window;

  // Get size of text

  int size[2];
  this->GetSize(viewport, size);

  // Get the position of the text actor

  int* actorPos;
  actorPos= 
    actor->GetActualPositionCoordinate()->GetComputedViewportValue(viewport);
  
  // Define bounding rectangle

  int pos[2];
  pos[0] = actorPos[0];
  pos[1] = (int)(actorPos[1] - tprop->GetLineOffset());

  switch (tprop->GetJustification())
    {
    case VTK_TEXT_LEFT: 
      break;
    case VTK_TEXT_CENTERED:
      pos[0] = pos[0] - size[0] / 2;
      break;
    case VTK_TEXT_RIGHT: 
      pos[0] = pos[0] - size[0];
      break;
    }

  switch (tprop->GetVerticalJustification())
    {
    case VTK_TEXT_TOP: 
      pos[1] = pos[1] - size[1] - this->LastLargestDescender;
      break;
    case VTK_TEXT_CENTERED:
      pos[1] = pos[1] - size[1] / 2 - this->LastLargestDescender / 2;
      break;
    case VTK_TEXT_BOTTOM: 
      break;
    }
  
  // Push a 2D matrix on the stack

  int *vsize = viewport->GetSize();
  float *vport = viewport->GetViewport();
  float *tileViewport = viewport->GetVTKWindow()->GetTileViewport();
  float visVP[4];

  visVP[0] = (vport[0] >= tileViewport[0]) ? vport[0] : tileViewport[0];
  visVP[1] = (vport[1] >= tileViewport[1]) ? vport[1] : tileViewport[1];
  visVP[2] = (vport[2] <= tileViewport[2]) ? vport[2] : tileViewport[2];
  visVP[3] = (vport[3] <= tileViewport[3]) ? vport[3] : tileViewport[3];

  if (visVP[0] == visVP[2] || visVP[1] == visVP[3])
    {
    return;
    }

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  if(viewport->GetIsPicking())
    {
    vtkgluPickMatrix(viewport->GetPickX(), viewport->GetPickY(),
                     1, 1, viewport->GetOrigin(), viewport->GetSize());
    }
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_LIGHTING);

  int front = 
    (actor->GetProperty()->GetDisplayLocation() == VTK_FOREGROUND_LOCATION);

  int *winSize = viewport->GetVTKWindow()->GetSize();

  int xoff = static_cast<int>
    (pos[0] - winSize[0] * ((visVP[2] + visVP[0]) / 2.0 - vport[0]));

  int yoff = static_cast<int>
    (pos[1] - winSize[1] * ((visVP[3] + visVP[1]) / 2.0 - vport[1]));
  
  // When picking draw the bounds of the text as a rectangle,
  // as text only picks when the pick point is exactly on the
  // origin of the text 

  if(viewport->GetIsPicking())
    {
    float x1 = 2.0 * (float)actorPos[0] / vsize[0] - 1;
    float y1 = 2.0 * ((float)actorPos[1] - tprop->GetLineOffset())/vsize[1] - 1;
    float width = 2.0 * (float)size[0] / vsize[0];
    float height = 2.0 * (float)size[1] / vsize[1];
    glRectf(x1, y1, x1 + width, y1 + height);

    // Clean up and return after drawing the rectangle

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_LIGHTING);
    
    return;
    }

  // Get the font color from the text actor

  unsigned char red, green, blue, alpha;
  
  // TOFIX: the default text prop color is set to a special (-1, -1, -1) value
  // to maintain backward compatibility for a while. Text mapper classes will
  // use the Actor2D color instead of the text prop color if this value is 
  // found (i.e. if the text prop color has not been set).

  float* tpropColor = tprop->GetColor();
  if (tpropColor[0] < 0.0 && tpropColor[1] < 0.0 && tpropColor[2] < 0.0)
    {
    tpropColor = actor->GetProperty()->GetColor();
    }

  // TOFIX: same goes for opacity

  float opacity = tprop->GetOpacity();
  if (opacity < 0.0)
    {
    opacity = actor->GetProperty()->GetOpacity();
    }

  red   = (unsigned char) (tpropColor[0] * 255.0);
  green = (unsigned char) (tpropColor[1] * 255.0);
  blue  = (unsigned char) (tpropColor[2] * 255.0);
  alpha = (unsigned char) (opacity       * 255.0);

  // Get the font
  
  FTFont *font = 
    vtkFontCache::GetInstance()->GetFont(tprop, 1, red,green,blue)->Font;
  if (!font) 
    {
    vtkErrorMacro(<< "Render - No font");
    return;
    }

  // Set up the shadow color

  if (tprop->GetShadow())
    {
    unsigned char rgb = (red + green + blue) / 3.0 > 128.0 ? 0 : 255;
    unsigned char shadow_red = rgb, shadow_green = rgb, shadow_blue = rgb; 

    // Get the shadow font
  
#if VTK_FTTM_CACHE_BY_RGBA
    FTFont *shadow_font;
    if (IsAntiAliasingRequestedByThisProperty(tprop))
      {
      shadow_font = vtkFontCache::GetInstance()->GetFont(
        tprop, 1, shadow_red, shadow_green, shadow_blue)->Font;
      if (!shadow_font) 
        {
        vtkErrorMacro(<< "Render - No shadow font");
        return;
        }
      } 
    else 
      {
      shadow_font = font;
      }
#endif
    
    // Set the color here since load/render glyphs is done
    // on demand and this color has to be consistent for a given font entry.
    
    glColor4ub(shadow_red, shadow_green, shadow_blue, alpha);

    // Required for clipping to work correctly

    glRasterPos3f(0, 0, (front)?(-1):(.99999));
    glBitmap(0, 0, 0, 0, xoff + 1, yoff - 1, NULL);
    
    // Draw the shadow text
    
#if VTK_FTTM_CACHE_BY_RGBA
    shadow_font->render(this->Input);  

    // Get the font again, Duh, since it may have been freed from the 
    // cache by the shadow font

    if (IsAntiAliasingRequestedByThisProperty(tprop))
      {
      font = 
        vtkFontCache::GetInstance()->GetFont(tprop, 1, red, green, blue)->Font;
      if (!font) 
        {
        vtkErrorMacro(<< "Render - No font");
        return;
        }
      }
#else
    font->render(this->Input);  
#endif
    }
  
  // Set the color here since load/render glyphs is done
  // on demand and this color has to be consistent for a given font entry.

  glColor4ub(red, green, blue, alpha);

  // Required for clipping to work correctly

  glRasterPos3f(0, 0, (front)?(-1):(.99999));
  glBitmap(0, 0, 0, 0, xoff, yoff, NULL);

  // Display a string

  font->render(this->Input);  

  glFlush();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

