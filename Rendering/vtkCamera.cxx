/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994

=========================================================================*/
#include <math.h>
#include "Camera.hh"
#include "vlMath.hh"

// Description:
// Construct camera instance with its focal point at the origin, 
// and position=(0,0,1). The view up is along the y-axis, 
// view angle is 30 degrees, and the clipping range is (.1,1000).
vlCamera::vlCamera()
{
  this->FocalPoint[0] = 0.0;
  this->FocalPoint[1] = 0.0;
  this->FocalPoint[2] = 0.0;

  this->Position[0] = 0.0;
  this->Position[1] = 0.0;
  this->Position[2] = 1.0;

  this->ViewUp[0] = 0.0;
  this->ViewUp[1] = 1.0;
  this->ViewUp[2] = 0.0;

  this->ViewAngle = 30.0;

  this->ClippingRange[0] = 0.01;
  this->ClippingRange[1] = 1000.01;

  this->Switch = 1;
  this->LeftEye = 1;
  this->EyeAngle = 2.0;

  this->Thickness = 1000.0;
  this->Distance = 1.0;

  this->ViewPlaneNormal[0] = 0.0;
  this->ViewPlaneNormal[1] = 0.0;
  this->ViewPlaneNormal[2] = 1.0;

  this->Orientation[0] = 0.0;
  this->Orientation[1] = 0.0;
  this->Orientation[2] = 0.0;
}

// Description:
// Set the position of the camera in world coordinates.
void vlCamera::SetPosition(float X, float Y, float Z)
{
  this->Position[0] = X;
  this->Position[1] = Y;
  this->Position[2] = Z;

  vlDebugMacro(<< " Position set to ( " <<  this->Position[0] << ", "
  << this->Position[1] << ", " << this->Position[2] << ")");

  // recalculate distance
  this->CalcDistance();
  
  // recalculate view plane normal
  this->CalcViewPlaneNormal();
  
  this->Modified();
}

void vlCamera::SetPosition(float a[3])
{
  this->SetPosition(a[0],a[1],a[2]);
}

// Description:
// Set the focal point of the camera in world coordinates.
void vlCamera::SetFocalPoint(float X, float Y, float Z)
{
  this->FocalPoint[0] = X; 
  this->FocalPoint[1] = Y; 
  this->FocalPoint[2] = Z;

  vlDebugMacro(<< " FocalPoint set to ( " <<  this->FocalPoint[0] << ", "
  << this->FocalPoint[1] << ", " << this->FocalPoint[2] << ")");

  // recalculate distance
  this->CalcDistance();
  
  // recalculate view plane normal
  this->CalcViewPlaneNormal();
  
  this->Modified();
}

void vlCamera::SetFocalPoint(float a[3])
{
  this->SetFocalPoint(a[0],a[1],a[2]);
}

// Description:
// Define the view-up direction for the camera.
void vlCamera::SetViewUp(float X, float Y, float Z)
{
  float dx, dy, dz, norm;

  this->ViewUp[0] = X;
  this->ViewUp[1] = Y;
  this->ViewUp[2] = Z;

  // normalize ViewUp
  dx = this->ViewUp[0];
  dy = this->ViewUp[1];
  dz = this->ViewUp[2];
  norm = sqrt( dx * dx + dy * dy + dz * dz );
  
  if(norm != 0) 
    {
    this->ViewUp[0] /= norm;
    this->ViewUp[1] /= norm;
    this->ViewUp[2] /= norm;
    }
  else 
    {
    this->ViewUp[0] = 0;
    this->ViewUp[1] = 1;
    this->ViewUp[2] = 0;
    }
  
  vlDebugMacro(<< " ViewUp set to ( " <<  this->ViewUp[0] << ", "
  << this->ViewUp[1] << ", " << this->ViewUp[2] << ")");
  
  this->Modified();
}

void vlCamera::SetViewUp(float a[3])
{
  this->SetViewUp(a[0],a[1],a[2]);
}

// Description:
// Specify the location of the front and back clipping planes along the
// direction of projection. These are positive distances along the 
// direction of projection.
void vlCamera::SetClippingRange(float X, float Y)
{
  this->ClippingRange[0] = X; 
  this->ClippingRange[1] = Y; 

  // check the order
  if(this->ClippingRange[0] > this->ClippingRange[1]) 
    {
    float temp;
    vlDebugMacro(<< " Front and back clipping range reversed");
    temp = this->ClippingRange[0];
    this->ClippingRange[0] = this->ClippingRange[1];
    this->ClippingRange[1] = temp;
    }
  
  // front should be greater than 0.001
  if (this->ClippingRange[0] < 0.001) 
    {
    this->ClippingRange[1] += 0.001 - this->ClippingRange[0];
    this->ClippingRange[0] = 0.001;
    vlDebugMacro(<< " Front clipping range is set to minimum.");
    }
  
  this->Thickness = this->ClippingRange[1] - this->ClippingRange[0];
  
  // thickness should be greater than THICKNESS_MIN
  if (this->Thickness < 0.002) 
    {
    this->Thickness = 0.002;
    vlDebugMacro(<< " ClippingRange thickness is set to minimum.");
    
    // set back plane
    this->ClippingRange[1] = this->ClippingRange[0] + this->Thickness;
    }
  
  vlDebugMacro(<< " ClippingRange set to ( " <<  this->ClippingRange[0] << ", "
  << this->ClippingRange[1] << ")");

  this->Modified();
}  

void vlCamera::SetClippingRange(float a[2])
{
  this->SetClippingRange(a[0],a[1]);
}

// Description:
// Set the distance between clipping planes. A side effect of this method is
// adjust the back clipping plane to be equal to the front clipping plane 
// plus the thickness.
void vlCamera::SetThickness(float X)
{
  if (this->Thickness == X) return;

  this->Thickness = X; 

  // thickness should be greater than THICKNESS_MIN
  if (this->Thickness < 0.002) 
    {
    this->Thickness = 0.002;
    vlDebugMacro(<< " ClippingRange thickness is set to minimum.");
    }
  
  // set back plane
  this->ClippingRange[1] = this->ClippingRange[0] + this->Thickness;

  vlDebugMacro(<< " ClippingRange set to ( " <<  this->ClippingRange[0] << ", "
  << this->ClippingRange[1] << ")");

  this->Modified();
}  

// Description:
// Set the distance of the focal point from the camera. The focal point is 
// modified accordingly. This should be positive.
void vlCamera::SetDistance(float X)
{
  if (this->Distance == X) return;

  this->Distance = X; 

  // Distance should be greater than .002
  if (this->Distance < 0.002) 
    {
    this->Distance = 0.002;
    vlDebugMacro(<< " Distance is set to minimum.");
    }
  
  // recalculate FocalPoint
  this->FocalPoint[0] = this->ViewPlaneNormal[0] * 
    this->Distance + this->Position[0];
  this->FocalPoint[1] = this->ViewPlaneNormal[1] * 
    this->Distance + this->Position[1];
  this->FocalPoint[2] = this->ViewPlaneNormal[2] * 
    this->Distance + this->Position[2];

  vlDebugMacro(<< " Distance set to ( " <<  this->Distance << ")");

  this->Modified();
}  

// Description:
// This returns the twist of the camera.  The twist corrisponds to Roll and
// represents the angle of rotation about the z axis to achieve the 
// current view-up vector.
float vlCamera::GetTwist()
{
  float *vup, *vn;
  float twist = 0;
  float v1[3], v2[3], y_axis[3];
  double theta, dot, mag;
  double cosang;
  vlMath math;

  vup = this->ViewUp;
  vn = this->GetViewPlaneNormal();

  // compute: vn X ( vup X vn)
  // and:     vn X ( y-axis X vn)
  // then find the angle between the two projected vectors
  //
  y_axis[0] = y_axis[2] = 0.0; y_axis[1] = 1.0;

  // bump the view normal if it is parallel to the y-axis
  //
  if ((vn[0] == 0.0) && (vn[2] == 0.0))
    vn[2] = 0.01*vn[1];

  // first project the view_up onto the view_plane
  //
  math.Cross(vup, vn, v1);
  math.Cross(vn, v1, v1);

  // then project the y-axis onto the view plane
  //
  math.Cross(y_axis, vn, v2);
  math.Cross(vn, v2, v2);

  // then find the angle between the two projected vectors
  //
  dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
  mag = sqrt((double)(v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2]));
  mag *= sqrt((double)(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]));

  // make sure we dont divide by 0 
  if (mag != 0.0) 
    {
    cosang = dot / mag;
    if (cosang < -1.0) cosang = -1.0;
    if (cosang > 1.0) cosang = 1.0;
    theta = acos(cosang);
    }
  else
    theta = 0.0;

  // now see if the angle is positive or negative
  //
  math.Cross(v1, v2, v1);
  dot = v1[0]*vn[0] + v1[1]*vn[1] + v1[2]*vn[2];
  
  twist = (theta);
  if (dot < 0.0)
    twist = -twist;
  
  return twist;
}

// Description:
// Compute the view plane normal from the position and focal point.
void vlCamera::CalcViewPlaneNormal()
{
  float dx,dy,dz;
  float distance;
  float *vpn = this->ViewPlaneNormal;

  // view plane normal is calculated from position and focal point
  //
  dx = this->Position[0] - this->FocalPoint[0];
  dy = this->Position[1] - this->FocalPoint[1];
  dz = this->Position[2] - this->FocalPoint[2];
  
  distance = sqrt(dx*dx+dy*dy+dz*dz);

  if (distance > 0.0) 
    {
    vpn[0] = -dx / distance;
    vpn[1] = -dy / distance;
    vpn[2] = -dz / distance;
    }
  
  vlDebugMacro(<< "Calculating ViewPlaneNormal of (" << vpn[0] << " " << vpn[1] << " " << vpn[2] << ")");
}


// Description:
// Set the roll angle of the camera about the view plane normal.
void vlCamera::SetRoll(float roll)
{
  float current;
  float temp[4];

  // roll is a rotation of camera view up about view plane normal
  vlDebugMacro(<< " Setting Roll to " << roll << "");

  // get the current roll
  current = this->GetRoll();

  roll -= current;

  this->Transform.Push();
  this->Transform.Identity();
  this->Transform.PreMultiply();

  // rotate about view plane normal
  this->Transform.RotateWXYZ(roll,this->ViewPlaneNormal[0],
			     this->ViewPlaneNormal[1],
			     this->ViewPlaneNormal[2]);
  
  // now transform view up
  temp[0] = this->ViewUp[0];
  temp[1] = this->ViewUp[1];
  temp[2] = this->ViewUp[2];
  temp[3] = 1.0;
  this->Transform.PointMultiply(temp,temp);
  
  // now store the result
  this->SetViewUp(temp);

  this->Transform.Pop();
}

// Description:
// Returns the roll of the camera, this is very similar to GetTwist.
float vlCamera::GetRoll()
{
  float	*orient;
  
  // set roll using orientation
  orient = this->GetOrientation();

  vlDebugMacro(<< " Returning Roll of " << orient[2] << "");

  return orient[2];
}

// Description:
// Compute the camera distance which is the distance between the 
// focal point and position.
void vlCamera::CalcDistance ()
{
  float   *distance;
  float   dx, dy, dz;
  
  // pickup pointer to distance
  distance = &this->Distance;
  
  dx = this->FocalPoint[0] - this->Position[0];
  dy = this->FocalPoint[1] - this->Position[1];
  dz = this->FocalPoint[2] - this->Position[2];
  
  *distance = sqrt( dx * dx + dy * dy + dz * dz );
  
  // Distance should be greater than .002
  if (this->Distance < 0.002) 
    {
    this->Distance = 0.002;
    vlDebugMacro(<< " Distance is set to minimum.");

    // recalculate position
    this->Position[0] = 
      - this->ViewPlaneNormal[0] * *distance + this->FocalPoint[0];
    this->Position[1] = 
      - this->ViewPlaneNormal[1] * *distance + this->FocalPoint[1];
    this->Position[2] = 
      - this->ViewPlaneNormal[2] * *distance + this->FocalPoint[2];
    
    vlDebugMacro(<< " Position set to ( " <<  this->Position[0] << ", "
    << this->Position[1] << ", " << this->Position[2] << ")");
    
    vlDebugMacro(<< " Distance set to ( " <<  this->Distance << ")");
    this->Modified();
    }
  
  vlDebugMacro(<< " Distance set to ( " <<  this->Distance << ")");
  
  this->Modified();
} 

// Description:
// Returns the orientation of the camera. This is a vector of X,Y and Z 
// rotations that when performed in the order RotateZ, RotateX and finally
// RotateY will yield the same 3x3 rotation matrix for the camera.
float *vlCamera::GetOrientation ()
{
  // calculate a new orientation
  this->CalcPerspectiveTransform();

  vlDebugMacro(<< " Returning Orientation of ( " <<  this->Orientation[0] 
  << ", " << this->Orientation[1] << ", " << this->Orientation[2] << ")");
  
  return this->Orientation;
}

// Description:
// Compute the perspective transform matrix. This is used in converting 
// between display, view and world coordinates.
void vlCamera::CalcPerspectiveTransform ()
{
  vlMatrix4x4  matrix;
  float   view_ratio;
  float   distance, distance_old;
  float   twist;
  float *temp;

  this->PerspectiveTransform.PostMultiply();  
  this->PerspectiveTransform.Identity();

  // translate to center of projection 
  this->PerspectiveTransform.Translate(-this->Position[0],
				       -this->Position[1],
				       -this->Position[2]);
  
  // first rotate y 
  // rotate around y axis so that result has no x component 
  distance = sqrt((this->Position[0]-this->FocalPoint[0])
		  *(this->Position[0]-this->FocalPoint[0]) +
		  (this->Position[2]-this->FocalPoint[2])*
		  (this->Position[2]-this->FocalPoint[2]));
  /* even with this check there seems to be a bug that causes picking to */
  /* be a little off when looking down the y axis */
  if (distance > 0.0)
    {
    matrix[0][0] = (this->Position[2]-this->FocalPoint[2])/distance;
    matrix[2][0] = -1.0*(this->FocalPoint[0] - 
			 this->Position[0])/distance;
    }
  else
    {
    if (this->Position[1] < this->FocalPoint[1])
      {
      matrix[0][0] = -1.0;
      }
    else
      {
      matrix[0][0] = 1.0;
      }
    matrix[2][0] = 0.0;
    }
  matrix[1][0] = matrix[3][0] = 0.0;
  matrix[1][1] = 1.0;
  matrix[0][1] = matrix[2][1] = matrix[3][1] = 0.0;
  matrix[0][2] = -1.0*matrix[2][0];
  matrix[2][2] = matrix[0][0];
  matrix[3][2] = 0.0;
  matrix[1][2] = 0.0;
  matrix[3][3] = 1.0;
  matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

  this->PerspectiveTransform.Concatenate(matrix);
  
  // now rotate x 
  // rotate about x axis so that the result has no y component 
  distance_old = distance;
  distance = sqrt((this->Position[0]-this->FocalPoint[0])*
		  (this->Position[0]-this->FocalPoint[0]) +
		  (this->Position[1]-this->FocalPoint[1])*
		  (this->Position[1]-this->FocalPoint[1]) +
		  (this->Position[2]-this->FocalPoint[2])*
		  (this->Position[2]-this->FocalPoint[2]));
  matrix[0][0] = 1.0;
  matrix[1][0] = matrix[2][0] = matrix[3][0] = 0.0;
  matrix[1][1] = distance_old/distance;
  matrix[2][1] = (this->Position[1] - this->FocalPoint[1])/distance;
  matrix[0][1] = matrix[3][1] = 0.0;
  matrix[1][2] = -1.0*matrix[2][1];
  matrix[2][2] = matrix[1][1];
  matrix[3][2] = 0.0;
  matrix[0][2] = 0.0;
  matrix[3][3] = 1.0;
  matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

  this->PerspectiveTransform.Concatenate(matrix);

  // now rotate z (twist) 
  // convert view up normal to gl twist 
  twist = this->GetTwist();

  matrix[0][0] = cos(-twist);
  matrix[1][0] = sin(-twist);
  matrix[2][0] = matrix[3][0] = 0.0;
  matrix[0][1] = -1.0*matrix[1][0];
  matrix[1][1] = matrix[0][0];
  matrix[2][1] = matrix[3][1] = 0.0;
  matrix[1][2] = 0.0;
  matrix[2][2] = 1.0;
  matrix[3][2] = 0.0;
  matrix[0][2] = 0.0;
  matrix[3][3] = 1.0;
  matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

  this->PerspectiveTransform.Concatenate(matrix);

  // now set the orientation
  temp = this->PerspectiveTransform.GetOrientation();
  this->Orientation[0] = temp[0];
  this->Orientation[1] = temp[1];
  this->Orientation[2] = temp[2];

  view_ratio   = tan ((fabs (this->ViewAngle) / 2.0) * 
		      (3.1415926 / 180.0));
  matrix[0][0] = 1.0 / view_ratio;
  matrix[1][0] = matrix[2][0] = matrix[3][0] = 0.0;
  matrix[1][1] = 1.0 / view_ratio;
  matrix[0][1] = matrix[2][1] = matrix[3][1] = 0.0;
  matrix[2][2] = -1.0*(this->ClippingRange[1]+this->ClippingRange[0])
    /(this->ClippingRange[1]-this->ClippingRange[0]);
  matrix[3][2] = -1.0;
  matrix[0][2] = matrix[1][2] = 0.0;
  matrix[2][3] = -2.0*this->ClippingRange[1]*this->ClippingRange[0]/ 
    (this->ClippingRange[1]-this->ClippingRange[0]);
  matrix[0][3] = matrix[1][3] = 0.0;
  matrix[3][3] = 0.0;

  this->PerspectiveTransform.Concatenate(matrix);
}


// Description:
// Return the perspective transform matrix. See CalcPerspectiveTransform.
vlMatrix4x4 &vlCamera::GetPerspectiveTransform()
{
  // update transform 
  this->CalcPerspectiveTransform();
  
  // return the transform 
  return this->PerspectiveTransform.GetMatrix();
}

#define SQ_MAG(x) ( (x)[0]*(x)[0] + (x)[1]*(x)[1] + (x)[2]*(x)[2] )

// Description:
// Recompute the view up vector so that it is perpendicular to the
// view plane normal.
void vlCamera::OrthogonalizeViewUp()
{
  float *normal,*up,temp[3],new_up[3];
  float temp_mag_sq,new_mag_sq,ratio;
  vlMath math;

  normal=this->ViewPlaneNormal;
  up=this->ViewUp;
  math.Cross(normal,up,temp);

  temp_mag_sq = (SQ_MAG(up));
  math.Cross(temp,normal,new_up);
  new_mag_sq = (SQ_MAG(new_up));
  ratio = sqrt(new_mag_sq/temp_mag_sq);
  this->SetViewUp(new_up[0]*ratio,new_up[1]*ratio,new_up[2]*ratio);
}

// Description:
// Move the position of the camera along the view plane normal. Moving
// towards the focal point (e.g., zoom>1) is a zoom-in, moving away 
// from the focal point (e.g., zoom<1) is a zoom-out.
void vlCamera::Zoom(float amount)
{
  float	distance;
  
  if (amount <= 0.0) return;
  
  // zoom moves position along view plane normal by a specified ratio
  distance = -this->Distance / amount;
  
  this->SetPosition(this->FocalPoint[0] +distance * this->ViewPlaneNormal[0],
		    this->FocalPoint[1] +distance * this->ViewPlaneNormal[1],
		    this->FocalPoint[2] +distance * this->ViewPlaneNormal[2]);
}


// Description:
// Rotate the camera about the view up vector centered at the focal point.
void vlCamera::Azimuth (float angle)
{
  // azimuth is a rotation of camera position about view up vector
  this->Transform.Push();
  this->Transform.Identity();
  this->Transform.PreMultiply();
  
  // translate to focal point
  this->Transform.Translate(this->FocalPoint[0],
			    this->FocalPoint[1],
			    this->FocalPoint[2]);
   
  // rotate about view up
  this->Transform.RotateWXYZ(-angle,this->ViewUp[0],this->ViewUp[1],
			     this->ViewUp[2]);
  
  
  // translate to focal point
  this->Transform.Translate(-this->FocalPoint[0],
			    -this->FocalPoint[1],
			    -this->FocalPoint[2]);
   
  // now transform position
  this->Transform.SetPoint(this->Position[0],this->Position[1],
			    this->Position[2],1.0);
  
  // now store the result
  this->SetPosition(this->Transform.GetPoint());
  
  this->Transform.Pop();
}

// Description:
// Rotate the camera about the corss product of the view plane normal and 
// the view_up vector centered on the focal_point.
void vlCamera::Elevation (float angle)
{
  double	axis[3];
  
  // elevation is a rotation of camera position about cross between
  // view up and view plane normal
  axis[0] = (this->ViewUp[1] * this->ViewPlaneNormal[2] -
	     this->ViewUp[2] * this->ViewPlaneNormal[1]);
  axis[1] = (this->ViewUp[2] * this->ViewPlaneNormal[0] -
	     this->ViewUp[0] * this->ViewPlaneNormal[2]);
  axis[2] = (this->ViewUp[0] * this->ViewPlaneNormal[1] -
	     this->ViewUp[1] * this->ViewPlaneNormal[0]);

  this->Transform.Push();
  this->Transform.Identity();
  this->Transform.PreMultiply();
  
  // translate to focal point
  this->Transform.Translate(this->FocalPoint[0],
			    this->FocalPoint[1],
			    this->FocalPoint[2]);
   
  // rotate about view up
  this->Transform.RotateWXYZ(-angle,axis[0],axis[1],axis[2]);
  
  
  // translate to focal point
  this->Transform.Translate(-this->FocalPoint[0],
			    -this->FocalPoint[1],
			    -this->FocalPoint[2]);
   
  // now transform position
  this->Transform.SetPoint(this->Position[0],this->Position[1],
			    this->Position[2],1.0);
  
  // now store the result
  this->SetPosition(this->Transform.GetPoint());
  
  this->Transform.Pop();
}

// Description:
// Rotate the focal point about the view_up vector centered at the cameras 
// position. 
void vlCamera::Yaw (float angle)
{
  // yaw is a rotation of camera focal_point about view up vector
  this->Transform.Push();
  this->Transform.Identity();
  this->Transform.PreMultiply();
  
  // translate to position
  this->Transform.Translate(this->Position[0],
			    this->Position[1],
			    this->Position[2]);
   
  // rotate about view up
  this->Transform.RotateWXYZ(angle,this->ViewUp[0],this->ViewUp[1],
			     this->ViewUp[2]);
  
  
  // translate to position
  this->Transform.Translate(-this->Position[0],
			    -this->Position[1],
			    -this->Position[2]);
   
  // now transform focal point
  this->Transform.SetPoint(this->FocalPoint[0],this->FocalPoint[1],
			    this->FocalPoint[2],1.0);
  
  // now store the result
  this->SetFocalPoint(this->Transform.GetPoint());
  
  this->Transform.Pop();
}

// Description:
// Rotate the focal point about the cross product of the view up vector 
// and the view plane normal, centered at the cameras position.
void vlCamera::Pitch (float angle)
{
  float	axis[3];

  
  // pitch is a rotation of camera focal point about cross between
  // view up and view plane normal
  axis[0] = (this->ViewUp[1] * this->ViewPlaneNormal[2] -
	     this->ViewUp[2] * this->ViewPlaneNormal[1]);
  axis[1] = (this->ViewUp[2] * this->ViewPlaneNormal[0] -
	     this->ViewUp[0] * this->ViewPlaneNormal[2]);
  axis[2] = (this->ViewUp[0] * this->ViewPlaneNormal[1] -
	     this->ViewUp[1] * this->ViewPlaneNormal[0]);

  this->Transform.Push();
  this->Transform.Identity();
  this->Transform.PreMultiply();
  
  // translate to position
  this->Transform.Translate(this->Position[0],
			    this->Position[1],
			    this->Position[2]);
   
  // rotate about view up
  this->Transform.RotateWXYZ(angle,axis[0],axis[1],axis[2]);
  
  // translate to position
  this->Transform.Translate(-this->Position[0],
			    -this->Position[1],
			    -this->Position[2]);
   
  // now transform focal point
  this->Transform.SetPoint(this->FocalPoint[0],this->FocalPoint[1],
			    this->FocalPoint[2],1.0);
  
  // now store the result
  this->SetFocalPoint(this->Transform.GetPoint());
  
  this->Transform.Pop();
}

// Description:
// Rotate the camera around the view plane normal.
void vlCamera::Roll (float angle)
{
  
  // roll is a rotation of camera view up about view plane normal
  this->Transform.Push();
  this->Transform.Identity();
  this->Transform.PreMultiply();

  // rotate about view plane normal
  this->Transform.RotateWXYZ(angle,this->ViewPlaneNormal[0],
			     this->ViewPlaneNormal[1],
			     this->ViewPlaneNormal[2]);
  
  // now transform view up
  this->Transform.SetPoint(this->ViewUp[0],this->ViewUp[1],
			    this->ViewUp[2],1.0);
  
  // now store the result
  this->SetViewUp(this->Transform.GetPoint());

  this->Transform.Pop();
}

// Description:
// Set the direction that the camera points.
// Adjusts position to be consistent with the view plane normal.
void vlCamera::SetViewPlaneNormal(float X,float Y,float Z)
{
  float norm;

  // make sure the distance is up to date
  this->CalcDistance();

  // normalize ViewUp
  norm = sqrt( X * X + Y * Y + Z * Z );
  if (!norm)
    {
    vlErrorMacro(<< "SetViewPlaneNormal of (0,0,0)");
    return;
    }
  
  // recalculate position
  this->Position[0] = 
    - X * this->Distance/norm + this->FocalPoint[0];
  this->Position[1] = 
    - Y * this->Distance/norm + this->FocalPoint[1];
  this->Position[2] = 
    - Z * this->Distance/norm + this->FocalPoint[2];
    
  vlDebugMacro(<< " Position set to ( " <<  this->Position[0] << ", "
  << this->Position[1] << ", " << this->Position[2] << ")");
  
  // recalculate view plane normal
  this->CalcViewPlaneNormal();
  
  this->Modified();
}

void vlCamera::SetViewPlaneNormal(float a[3])
{
  this->SetViewPlaneNormal(a[0],a[1],a[2]);
}

void vlCamera::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlCamera::GetClassName()))
    {
    vlObject::PrintSelf(os,indent);
    
    // update orientation
    this->GetOrientation();

    os << indent << "Clipping Range: (" << this->ClippingRange[0] << ", " 
      << this->ClippingRange[2] << ")\n";
    os << indent << "Distance: " << this->Distance << "\n";
    os << indent << "Eye Angle: " << this->EyeAngle << "\n";
    os << indent << "Focal Point: (" << this->FocalPoint[0] << ", " 
      << this->FocalPoint[1] << ", " << this->FocalPoint[2] << ")\n";
    os << indent << "Left Eye: " << this->LeftEye << "\n";
    os << indent << "Orientation: (" << this->Orientation[0] << ", " 
      << this->Orientation[1] << ", " << this->Orientation[2] << ")\n";
    os << indent << "Position: (" << this->Position[0] << ", " 
      << this->Position[1] << ", " << this->Position[2] << ")\n";
    os << indent << "Switch: " << (this->Switch ? "On\n" : "Off\n");
    os << indent << "Thickness: " << this->Thickness << "\n";
    os << indent << "Twist: " << this->GetTwist() << "\n";
    os << indent << "View Angle: " << this->ViewAngle << "\n";
    os << indent << "View Plane Normal: (" << this->ViewPlaneNormal[0] << ", " 
      << this->ViewPlaneNormal[1] << ", " << this->ViewPlaneNormal[2] << ")\n";
    os << indent << "View Up: (" << this->ViewUp[0] << ", " 
      << this->ViewUp[1] << ", " << this->ViewUp[2] << ")\n";
    }
}
