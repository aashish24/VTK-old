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
// .NAME vtkProperty - represent surface properties of a geometric object
// .SECTION Description
// vtkProperty is an object that represents lighting and other surface
// properties of a geometric object. The primary properties that can be 
// set are colors (overall, ambient, diffuse, specular, and edge color);
// specular power; opacity of the object; the representation of the
// object (points, wireframe, or surface); and the shading method to be 
// used (flat, Gouraud, and Phong). Also, some special graphics features
// like backface properties can be set and manipulated with this object.
// .SECTION See Also
// vtkActor vtkPropertyDevice

#ifndef __vtkProperty_h
#define __vtkProperty_h

#include "vtkRender.h"
#include "vtkObject.h"
#include "vtkStructuredPoints.h"

class vtkRenderer;
class vtkActor;

class VTK_EXPORT vtkProperty : public vtkObject
{
public:
  vtkProperty();
  static vtkProperty *New();
  const char *GetClassName() {return "vtkProperty";};
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkProperty &operator=(const vtkProperty& p);

  // Description:
  // This method causes the property to set up whatever is required for
  // its instance variables. This is actually handled by a subclass of
  // vtkProperty, which is created automatically. This
  // method includes the invoking actor as an argument which can
  // be used by property devices that require the actor.
  virtual void Render(vtkActor *,vtkRenderer *) {};

  // Description:
  // This method renders the property as a backface property. TwoSidedLighting
  // must be turned off to see any backface properties. Note that only
  // colors and opacity are used for backface properties. Other properties
  // such as Representation, Culling are specified by the Property.
  virtual void BackfaceRender(vtkActor *,vtkRenderer *) {};

  // Description:
  // Set the shading interpolation method for an object.
  vtkSetClampMacro(Interpolation,int,VTK_FLAT,VTK_PHONG);
  vtkGetMacro(Interpolation,int);
  void SetInterpolationToFlat() {this->SetInterpolation(VTK_FLAT);};
  void SetInterpolationToGouraud() {this->SetInterpolation(VTK_GOURAUD);};
  void SetInterpolationToPhong() {this->SetInterpolation(VTK_PHONG);};
  char *GetInterpolationAsString();

  // Description:
  // Control the surface geometry representation for the object.
  vtkSetClampMacro(Representation,int,VTK_POINTS,VTK_SURFACE);
  vtkGetMacro(Representation,int);
  void SetRepresentationToPoints() {this->SetRepresentation(VTK_POINTS);};
  void SetRepresentationToWireframe() {this->SetRepresentation(VTK_WIREFRAME);};
  void SetRepresentationToSurface() {this->SetRepresentation(VTK_SURFACE);};
  char *GetRepresentationAsString();

  // Description:
  // Set the color of the object. Has the side effect of setting the
  // ambient diffuse and specular colors as well. This is basically
  // a quick overall color setting method.
  void SetColor(float r,float g,float b);
  void SetColor(float a[3]) { this->SetColor(a[0], a[1], a[2]); };
  float *GetColor();
  void GetColor(float rgb[3]);

  // Description:
  // Set/Get the ambient lighting coefficient.
  vtkSetClampMacro(Ambient,float,0.0,1.0);
  vtkGetMacro(Ambient,float);

  // Description:
  // Set/Get the diffuse lighting coefficient.
  vtkSetClampMacro(Diffuse,float,0.0,1.0);
  vtkGetMacro(Diffuse,float);

  // Description:
  // Set/Get the specular lighting coefficient.
  vtkSetClampMacro(Specular,float,0.0,1.0);
  vtkGetMacro(Specular,float);

  // Description:
  // Set/Get the specular power.
  vtkSetClampMacro(SpecularPower,float,0.0,100.0);
  vtkGetMacro(SpecularPower,float);

  // Description:
  // Set/Get the object's opacity. 1.0 is totally opaque and 0.0 is completely
  // transparent.
  vtkSetClampMacro(Opacity,float,0.0,1.0);
  vtkGetMacro(Opacity,float);

  // Description:
  // Turn on/off the visibility of edges. On some renderers it is
  // possible to render the edges of geometric primitives separately
  // from the interior.
  vtkGetMacro(EdgeVisibility,int);
  vtkSetMacro(EdgeVisibility,int);
  vtkBooleanMacro(EdgeVisibility,int);

  // Description:
  // Set/Get the ambient surface color. Not all renderers support separate
  // ambient and diffuse colors. From a physical standpoint it really
  // doesn't make too much sense to have both. For the rendering
  // libraries that don't support both, the diffuse color is used.
  vtkSetVector3Macro(AmbientColor,float);
  vtkGetVectorMacro(AmbientColor,float,3);

  // Description:
  // Set/Get the diffuse surface color.
  vtkSetVector3Macro(DiffuseColor,float);
  vtkGetVectorMacro(DiffuseColor,float,3);

  // Description:
  // Set/Get the specular surface color.
  vtkSetVector3Macro(SpecularColor,float);
  vtkGetVectorMacro(SpecularColor,float,3);

  // Description:
  // Set/Get the color of primitive edges (if edge visibility enabled).
  vtkSetVector3Macro(EdgeColor,float);
  vtkGetVectorMacro(EdgeColor,float,3);

  // Description:
  // Turn on/off fast culling of polygons based on orientation of normal 
  // with respect to camera. If backface culling is on, polygons facing
  // away from camera are not drawn.
  vtkGetMacro(BackfaceCulling,int);
  vtkSetMacro(BackfaceCulling,int);
  vtkBooleanMacro(BackfaceCulling,int);

  // Description:
  // Turn on/off fast culling of polygons based on orientation of normal 
  // with respect to camera. If frontface culling is on, polygons facing
  // towards camera are not drawn.
  vtkGetMacro(FrontfaceCulling,int);
  vtkSetMacro(FrontfaceCulling,int);
  vtkBooleanMacro(FrontfaceCulling,int);

protected:
  float Color[3];
  float AmbientColor[3];
  float DiffuseColor[3];
  float SpecularColor[3];
  float EdgeColor[3];
  float Ambient;
  float Diffuse;
  float Specular;
  float SpecularPower;
  float Opacity;
  int   Interpolation; 
  int   Representation;
  int   EdgeVisibility;
  int   BackfaceCulling;
  int   FrontfaceCulling;
};

// Description:
// Return the method of shading as a descriptive character string.
inline char *vtkProperty::GetInterpolationAsString(void)
{
  if ( this->Interpolation == VTK_FLAT )
    {
    return "Flat";
    }
  else if ( this->Interpolation == VTK_GOURAUD ) 
    {
    return "Gouraud";
    }
  else 
    {
    return "Phong";
    }
}


// Description:
// Return the method of shading as a descriptive character string.
inline char *vtkProperty::GetRepresentationAsString(void)
{
  if ( this->Representation == VTK_POINTS )
    {
    return "Points";
    }
  else if ( this->Representation == VTK_WIREFRAME ) 
    {
    return "Wireframe";
    }
  else 
    {
    return "Surface";
    }
}



#endif
