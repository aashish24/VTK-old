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

#include "vtkAnimationScene.h"
#include "vtkObjectFactory.h"
#include "vtkCollection.h"
#include "vtkCollectionIterator.h"
#include "vtkTimerLog.h"

vtkCxxRevisionMacro(vtkAnimationScene, "$Revision$");
vtkStandardNewMacro(vtkAnimationScene);

//----------------------------------------------------------------------------
vtkAnimationScene::vtkAnimationScene()
{
  this->PlayMode = VTK_ANIMATION_SCENE_PLAYMODE_SEQUENCE;
  this->FrameRate = 10.0;
  this->Loop = 0;
  this->InPlay = 0;
  this->StopPlay = 0;
  this->CurrentTime = 0.0;

  this->AnimationCues = vtkCollection::New();
  this->AnimationCuesIterator = this->AnimationCues->NewIterator();
  this->AnimationTimer = vtkTimerLog::New();
}

//----------------------------------------------------------------------------
vtkAnimationScene::~vtkAnimationScene()
{
  if (this->InPlay)
    {
    this->Stop();
    }
  this->AnimationCues->Delete();
  this->AnimationCuesIterator->Delete();
  this->AnimationTimer->Delete();
}

//----------------------------------------------------------------------------
void vtkAnimationScene::AddCue(vtkAnimationCue* cue)
{
  if (this->AnimationCues->IsItemPresent(cue))
    {
    vtkErrorMacro("Animation cue already present in the scene");
    return;
    }
  if (this->TimeMode == VTK_ANIMATION_CUE_TIMEMODE_NORMALIZED &&
    cue->GetTimeMode() != VTK_ANIMATION_CUE_TIMEMODE_NORMALIZED)
    {
    vtkErrorMacro("A cue with relative time mode cannot be added to a scene "
      "with normalized time mode.");
    return;
    }
  this->AnimationCues->AddItem(cue);
}

//----------------------------------------------------------------------------
void vtkAnimationScene::RemoveCue(vtkAnimationCue* cue)
{
  this->AnimationCues->RemoveItem(cue);
}

//----------------------------------------------------------------------------
void vtkAnimationScene::SetTimeMode(int mode)
{
  if (mode == VTK_ANIMATION_CUE_TIMEMODE_NORMALIZED)
    {
    // If noralized time mode is being set on the scene,
    // ensure that none of the contained cues need relative times.
    vtkCollectionIterator *it = this->AnimationCuesIterator;
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
      {
      vtkAnimationCue* cue = 
        vtkAnimationCue::SafeDownCast(it->GetCurrentObject());
      if (cue && cue->GetTimeMode() != VTK_ANIMATION_CUE_TIMEMODE_NORMALIZED)
        {
        vtkErrorMacro("Scene contains a cue in relative mode. It must be removed "
          "or chaged to normalized mode before changing the scene time mode");
        return;
        }
      }
    }
  this->Superclass::SetTimeMode(mode);
}

//----------------------------------------------------------------------------
void vtkAnimationScene::Initialize()
{
  this->Superclass::Initialize();
  // run thr all the cues and init them.
  vtkCollectionIterator *it = this->AnimationCuesIterator;
  for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
    {
    vtkAnimationCue* cue = 
      vtkAnimationCue::SafeDownCast(it->GetCurrentObject());
    if (cue)
      {
      cue->Initialize();
      }
    }
}

//----------------------------------------------------------------------------
void vtkAnimationScene::Finalize()
{
  this->Superclass::Finalize();
  vtkCollectionIterator *it = this->AnimationCuesIterator;
  for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
    {
    vtkAnimationCue* cue = 
      vtkAnimationCue::SafeDownCast(it->GetCurrentObject());
    if (cue)
      {
      cue->Finalize();
      }
    }
}

//----------------------------------------------------------------------------
void vtkAnimationScene::Play()
{
  if (this->InPlay)
    {
    return;
    }
  
  if (this->TimeMode == VTK_ANIMATION_CUE_TIMEMODE_NORMALIZED)
    {
    vtkErrorMacro("Cannot play a scene with normalized time mode");
    return;
    }

  this->InPlay = 1;
  this->StopPlay = 0;
  this->FrameRate = (!this->FrameRate)? 1.0 : this->FrameRate;
  // the actual play loop, check for StopPlay flag.
  do
    {
    double deltatime = 0.0;
    double oldcurrenttime = 0.0;
    double currenttime = this->CurrentTime;
    if (currenttime < this->StartTime || currenttime >= this->EndTime)
      {
      currenttime = this->StartTime;
      }
    this->AnimationTimer->StartTimer();
    this->Initialize();
    do
      {
      this->Tick(currenttime, deltatime);
      oldcurrenttime = currenttime;
      if (this->PlayMode == VTK_ANIMATION_SCENE_PLAYMODE_REALTIME)
        {
        this->AnimationTimer->StopTimer();
        currenttime = this->AnimationTimer->GetElapsedTime() + 
          this->StartTime;
        }
      else if (this->PlayMode == VTK_ANIMATION_SCENE_PLAYMODE_SEQUENCE)
        {
        currenttime += 1.0 / this->FrameRate;
        }
      else
        {
        vtkErrorMacro("Invalid Play Mode");
        this->StopPlay = 1;
        break;
        }
      deltatime = currenttime - oldcurrenttime;
      deltatime = (deltatime < 0)? -1*deltatime : deltatime;
      }
    while (!this->StopPlay && this->CueState != vtkAnimationCue::INACTIVE);
    this->Finalize();
    if (this->Loop)
      {
      this->CurrentTime = this->StartTime;
      }
    }
  while (this->Loop && !this->StopPlay);
  this->StopPlay = 0;
  this->InPlay = 0;
}

//----------------------------------------------------------------------------
void vtkAnimationScene::Stop()
{
  if (!this->InPlay)
    {
    return;
    }
  this->StopPlay = 1;
}

//----------------------------------------------------------------------------
void vtkAnimationScene::TickInternal(double currenttime, double deltatime)
{
  this->CurrentTime = currenttime;
  
  vtkCollectionIterator* iter = this->AnimationCuesIterator;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    vtkAnimationCue* cue = vtkAnimationCue::SafeDownCast(
      iter->GetCurrentObject());
    if (cue)
      {
      switch(cue->GetTimeMode())
        {
      case VTK_ANIMATION_CUE_TIMEMODE_RELATIVE:
        cue->Tick(currenttime - this->StartTime, deltatime);
        break;
      case VTK_ANIMATION_CUE_TIMEMODE_NORMALIZED:
        cue->Tick( (currenttime - this->StartTime) / (this->EndTime - this->StartTime),
          deltatime / (this->EndTime - this->StartTime));
        break;
      default:
        vtkErrorMacro("Invalid cue time mode");
        }
      }
    }
  this->Superclass::TickInternal(currenttime, deltatime);
}

//----------------------------------------------------------------------------
void vtkAnimationScene::SetCurrentTime(double currenttime)
{
  if (this->InPlay)
    {
    vtkErrorMacro("SetCurrentTime cannot be called while playing");
    return;
    }
  this->Initialize();
  this->Tick(currenttime,0.0);
  if (this->CueState == vtkAnimationCue::INACTIVE)
    {
    this->Finalize();
    }
}

//----------------------------------------------------------------------------
void vtkAnimationScene::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PlayMode: " << this->PlayMode << endl;
  os << indent << "FrameRate: " << this->FrameRate << endl;
  os << indent << "Loop: " << this->Loop << endl;
  os << indent << "InPlay: " << this->InPlay << endl;
  os << indent << "StopPlay: " << this->StopPlay << endl;
  os << indent << "CurrentTime: " << this->CurrentTime << endl;
}
