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
#include "vtkPushPipeline.h"

#include "vtkAbstractMapper.h"
#include "vtkArrayMap.txx"
#include "vtkCommand.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPushImageReader.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkSource.h"
#include "vtkVector.txx"
#include "vtkVolumeMapper.h"

class vtkPushPipelineProcessInfo {
public:
  // an input is ready if it has new data
  int AreAllInputsReady(vtkPushPipeline *pp);
  int IsInputReady(int i, vtkPushPipeline *pp);
  // an output is ready if its last results were consumed
  int AreAllOutputsReady(vtkPushPipeline *pp);
  int IsOutputReady(int o, vtkPushPipeline *pp);
  void ConsumeAllInputs(vtkPushPipeline *pp);
  void ConsumeInput(int i, vtkPushPipeline *pp);    
  void ProduceOutputs(vtkPushPipeline *);
  vtkPushPipelineProcessInfo();
  int ExecutionToOutputRatio;
  int NumberProcessed;
  void ProcessSomeData(vtkPushPipeline *);
  int Marked;
  int InputToExecutionRatio[VTK_PP_MAX_INPUTS];
  vtkProcessObject *ProcessObject;
};
  
class vtkPushPipelineDataInfo {
public:
  int IsConsumerLeft(vtkProcessObject *);
  int IsWindowConsumerLeft(vtkRenderWindow *);
  void ConsumeData(vtkProcessObject *);
  void ConsumeWindow(vtkRenderWindow *);
  void FillConsumersLeft();
  vtkPushPipelineDataInfo();
  ~vtkPushPipelineDataInfo();
  vtkVector<vtkProcessObject *> *ConsumersLeft;
  vtkVector<vtkRenderWindow *> *WindowConsumersLeft;
  int Marked;
  vtkDataObject *DataObject;
  vtkVector<vtkRenderWindow *> *WindowConsumers;
};

class vtkPushPipelineConsumeCommand : public vtkCommand
{
public:
  static vtkPushPipelineConsumeCommand *New() { return new vtkPushPipelineConsumeCommand;}
  virtual void Execute(vtkObject *caller, unsigned long, void *)
    {
      vtkProcessObject *po = vtkProcessObject::SafeDownCast(caller);
      if (po && this->PushPipeline)
        {
        vtkPushPipelineProcessInfo *ppi =
          this->PushPipeline->GetPushProcessInfo(po);
        ppi->NumberProcessed++;              
        ppi->ConsumeAllInputs(this->PushPipeline);
        ppi->ProduceOutputs(this->PushPipeline);
        }
    }
  void SetPushPipeline(vtkPushPipeline *p) {
    this->PushPipeline = p; }
  
  vtkPushPipeline *PushPipeline;
};

class vtkPushPipelineEndRunCommand : public vtkCommand
{
public:
  static vtkPushPipelineEndRunCommand *New() { return new vtkPushPipelineEndRunCommand;}
  virtual void Execute(vtkObject *, unsigned long, void *)
    {
      if (this->PushPipeline)
        {
        this->PushPipeline->SetRunState(2);
        }
    }
  void SetPushPipeline(vtkPushPipeline *p) {
    this->PushPipeline = p; }
  
  vtkPushPipeline *PushPipeline;
};

vtkCxxRevisionMacro(vtkPushPipeline, "$Revision$");
vtkStandardNewMacro(vtkPushPipeline);

vtkPushPipeline::vtkPushPipeline()
{
  this->RunState = 0;
  this->ProcessMap = vtkArrayMap<vtkProcessObject *, vtkPushPipelineProcessInfo *>::New();
  this->DataMap = vtkArrayMap<vtkDataObject *, vtkPushPipelineDataInfo *>::New();
  this->Windows = vtkVector<vtkRenderWindow *>::New();
}

vtkPushPipeline::~vtkPushPipeline()
{
  vtkArrayMapIterator<vtkProcessObject *, vtkPushPipelineProcessInfo *> *pmi = 
    this->ProcessMap->NewIterator();
  vtkPushPipelineProcessInfo *pref = NULL;
  while(!pmi->IsDoneWithTraversal())
    {
    pmi->GetData(pref);
    delete pref;
    pmi->GoToNextItem();
    }
  pmi->Delete();
  this->ProcessMap->Delete();

  vtkArrayMapIterator<vtkDataObject *, vtkPushPipelineDataInfo *> *dmi = 
    this->DataMap->NewIterator();
  vtkPushPipelineDataInfo *dref = NULL;
  while(!dmi->IsDoneWithTraversal())
    {
    dmi->GetData(dref);
    delete dref;
    dmi->GoToNextItem();
    }
  dmi->Delete();
  this->DataMap->Delete();

  this->Windows->Delete();
}

void vtkPushPipeline::AddPusher(vtkProcessObject* pusher)
{
  // add this pusher
  if (!this->ProcessMap->IsItemPresent(pusher))
    {
    vtkPushPipelineProcessInfo *ppi = new vtkPushPipelineProcessInfo;
    this->ProcessMap->SetItem(pusher,ppi);
    ppi->ProcessObject = pusher;
    pusher->InvokeEvent(vtkCommand::PushDataStartEvent,this);

    // if it is a mapper then attach an observer to the end render event
    vtkAbstractMapper *mpr = 
      vtkAbstractMapper::SafeDownCast(pusher);
    if (mpr)
      {
      vtkPushPipelineConsumeCommand *cc = vtkPushPipelineConsumeCommand::New();
      cc->SetPushPipeline(this);
      mpr->AddObserver(vtkCommand::EndEvent,cc);
      cc->Delete();
      }
    }
}

void vtkPushPipeline::AddData(vtkDataObject *dao)
{
  // add this data
  if (!this->DataMap->IsItemPresent(dao))
    {
    vtkPushPipelineDataInfo *pdi = new vtkPushPipelineDataInfo;
    this->DataMap->SetItem(dao,pdi);
    pdi->DataObject = dao;
    }
}

void vtkPushPipeline::ClearTraceMarkers()
{
  vtkArrayMapIterator<vtkDataObject *, vtkPushPipelineDataInfo *> *dmi = 
    this->DataMap->NewIterator();
  vtkPushPipelineDataInfo *dref = NULL;
  while(!dmi->IsDoneWithTraversal())
    {
    dmi->GetData(dref);
    dref->Marked = 0;
    dmi->GoToNextItem();
    }
  dmi->Delete();

  vtkArrayMapIterator<vtkProcessObject *, vtkPushPipelineProcessInfo *> *pmi = 
    this->ProcessMap->NewIterator();
  vtkPushPipelineProcessInfo *pref = NULL;
  while(!pmi->IsDoneWithTraversal())
    {
    pmi->GetData(pref);
    pref->Marked = 0;
    pmi->GoToNextItem();
    }
  pmi->Delete();
}

void vtkPushPipeline::Trace(vtkDataObject *dao)
{
  // add the data object to the map
  this->AddData(dao);
  vtkPushPipelineDataInfo *dref = NULL;
  this->DataMap->GetItem(dao,dref);
  if (dref->Marked)
    {
    return;
    }
  dref->Marked = 1;
  
  // now propagate to source and consumers
  if (dao->GetSource())
    {
    this->Trace(dao->GetSource());
    }
  
  // now trace the consumers
  int numCon = dao->GetNumberOfConsumers();
  int i;
  for (i = 0; i < numCon; ++i)
    {
    vtkProcessObject *con = 
      vtkProcessObject::SafeDownCast(dao->GetConsumer(i));
    if (con)
      {
      this->Trace(con);
      }
    else
      {
      // if it wasn't a process object it might lead us to a vtkWindow
      // that we need to setup
      vtkImageActor *ia = vtkImageActor::SafeDownCast(dao->GetConsumer(i));
      if (ia)
        {
        int numCon2 = ia->GetNumberOfConsumers();
        int i2;
        for (i2 = 0; i2 < numCon2; ++i2)
          {
          vtkRenderer *v = vtkRenderer::SafeDownCast(ia->GetConsumer(i2));
          if (v)
            {
            this->AddWindow(v->GetRenderWindow());
            }
          }        
        }
      }  
    }
}

void vtkPushPipeline::Trace(vtkProcessObject *po)
{
  // add the po to the map
  this->AddPusher(po);
  vtkPushPipelineProcessInfo *dref = NULL;
  this->ProcessMap->GetItem(po,dref);
  if (dref->Marked)
    {
    return;
    }
  dref->Marked = 1;

  // trace all inputs
  int numIn = po->GetNumberOfInputs();
  vtkDataObject **ins = po->GetInputs();
  int i;
  for (i = 0; i < numIn; i++)
    {
    if (ins[i])
      {
      this->Trace(ins[i]);
      }
    }
  
  // trace all outputs
  vtkSource *src = vtkSource::SafeDownCast(po);
  if (src)
    {
    int numOut = src->GetNumberOfOutputs();
    vtkDataObject **outs = src->GetOutputs();
    int i;
    src->UpdateInformation();
    for (i = 0; i < numOut; i++)
      {
      if (outs[i])
        {
        outs[i]->SetUpdateExtent(outs[i]->GetWholeExtent());
        this->Trace(outs[i]);
        }
      }
    }
  
  // it might be a PushImageReader, if so set PushPipeline ivar
  vtkPushImageReader *pir = vtkPushImageReader::SafeDownCast(po);
  if (pir)
    {
    pir->SetPushPipeline(this);
    }
  
  // it might also be a mapper
  // that we need to setup
  vtkAbstractMapper *am = vtkAbstractMapper::SafeDownCast(po);
  if (am)
    {
    // a mappers consumers are props
    int numCon = am->GetNumberOfConsumers();
    int i;
    for (i = 0; i < numCon; ++i)
      {
      vtkProp *prop = vtkProp::SafeDownCast(am->GetConsumer(i));
      if (prop)
        {
        int numCon2 = prop->GetNumberOfConsumers();
        int i2;
        for (i2 = 0; i2 < numCon2; ++i2)
          {
          vtkRenderer *v = vtkRenderer::SafeDownCast(prop->GetConsumer(i2));
          if (v)
            {
            this->AddWindow(v->GetRenderWindow());
            }
          }        
        }
      }
    }
}

vtkPushPipelineProcessInfo *
vtkPushPipeline::GetPushProcessInfo(vtkProcessObject *pusher)
{
  vtkPushPipelineProcessInfo *res = NULL;
  this->ProcessMap->GetItem(pusher,res);
  return res;
}

vtkPushPipelineDataInfo *
vtkPushPipeline::GetPushDataInfo(vtkDataObject *dao)
{
  vtkPushPipelineDataInfo *res = NULL;
  this->DataMap->GetItem(dao,res);
  return res;
}

void vtkPushPipeline::Push(vtkSource *pusher)
{
  // trace the network
  this->ClearTraceMarkers();
  this->Trace(pusher);
  this->SetupWindows();
  
  if (!ProcessMap->IsItemPresent(pusher))
    {
    vtkErrorMacro("pusher is not found");
    return;
    }

  // run the network until the pusher has executed, 
  // and everything else has executed, but do not run the pusher
  // twice
  int state = 0; // state 0 = not executed 1 = executed 2= idle
  while (state < 2)
    {
    // foreach source/filter/worker
    vtkArrayMapIterator<vtkProcessObject *, vtkPushPipelineProcessInfo *> *pmi = this->ProcessMap->NewIterator();
    int executedOne = 0;
    vtkPushPipelineProcessInfo *pref = NULL;
    for (;!pmi->IsDoneWithTraversal(); pmi->GoToNextItem())
      {
      pmi->GetData(pref);
      if (pref->AreAllInputsReady(this) && 
          pref->AreAllOutputsReady(this) && 
          !(state == 1 &&  pusher == pref->ProcessObject))
        {
        pref->ProcessSomeData(this);
        if (pusher == pref->ProcessObject)
          {
          state = 1;
          }
        executedOne = 1;
        }
      }
    this->RenderWindows();
    if (state && !executedOne)
      {
      state = 2;
      }
    }
}

void vtkPushPipeline::Run(vtkSource *pusher)
{
  // trace the network
  this->ClearTraceMarkers();
  this->Trace(pusher);
  this->SetupWindows();
  
  if (!ProcessMap->IsItemPresent(pusher))
    {
    vtkErrorMacro("pusher is not found");
    return;
    }

  // attach an end of data observer on pusher
  vtkPushPipelineEndRunCommand *cc = vtkPushPipelineEndRunCommand::New();
  cc->SetPushPipeline(this);
  unsigned long tag = 
    pusher->AddObserver(vtkCommand::EndOfDataEvent,cc);
  // run the network until the pusher is out of data
  // and everything else has executed, but do not run the pusher
  // after it is out of data
  this->RunState = 1; 
  // Running 1 = running, 
  // 2 = pusher ran out of data but network still running
  // 3 = pusher out of data and rest of network idle
  while (this->RunState < 3)
    {
    // foreach source/filter/worker
    vtkArrayMapIterator<vtkProcessObject *, vtkPushPipelineProcessInfo *> *pmi = this->ProcessMap->NewIterator();
    int executedOne = 0;
    vtkPushPipelineProcessInfo *pref = NULL;
    for (;!pmi->IsDoneWithTraversal(); pmi->GoToNextItem())
      {
      pmi->GetData(pref);
      if (pref->AreAllInputsReady(this) && 
          pref->AreAllOutputsReady(this) && 
          !(this->RunState == 2 &&  pusher == pref->ProcessObject))
        {
        pref->ProcessSomeData(this);
        executedOne = 1;
        }
      }
    this->RenderWindows();
    if (this->RunState == 2 && !executedOne)
      {
      this->RunState = 3;
      }
    pmi->Delete();
    }
  // remove the observer now that we are done running
  pusher->RemoveObserver(tag);
  cc->Delete();
}

vtkPushPipelineDataInfo::vtkPushPipelineDataInfo()
{
  this->Marked = 0;
  this->DataObject = 0;
  this->ConsumersLeft = vtkVector<vtkProcessObject *>::New();
  this->WindowConsumers = vtkVector<vtkRenderWindow *>::New();
  this->WindowConsumersLeft = vtkVector<vtkRenderWindow *>::New();
}

vtkPushPipelineDataInfo::~vtkPushPipelineDataInfo()
{
  this->ConsumersLeft->Delete();
  this->WindowConsumers->Delete();
  this->WindowConsumersLeft->Delete();
}

vtkPushPipelineProcessInfo::vtkPushPipelineProcessInfo()
{
  this->Marked = 0;
  this->NumberProcessed = 0;
  this->ExecutionToOutputRatio = 1;
  this->ProcessObject = 0;
  int i;
  for (i = 0; i < VTK_PP_MAX_INPUTS; ++i)
    {
    this->InputToExecutionRatio[i] = 1;
    }
}

int vtkPushPipelineProcessInfo::IsInputReady(int i, vtkPushPipeline *pp)
{
  vtkPushPipelineDataInfo *pdi = pp->GetPushDataInfo(this->ProcessObject->GetInputs()[i]);
  if (!pdi)
    {
    vtkGenericWarningMacro("Attempt to check input status for aninput that is unknown to the vtkPushPiepline");
    return 0;
    }
  
  return pdi->IsConsumerLeft(this->ProcessObject);
}


int vtkPushPipelineProcessInfo::AreAllInputsReady(vtkPushPipeline *pp)
{
  int numIn = this->ProcessObject->GetNumberOfInputs();
  int i;
  for (i = 0; i < numIn; ++i)
    {
    if (!this->IsInputReady(i,pp))
      {
      return 0;
      }
    }
  return 1;
}

int vtkPushPipelineProcessInfo::AreAllOutputsReady(vtkPushPipeline *pp)
{
  vtkSource *src = vtkSource::SafeDownCast(this->ProcessObject);
  if (src)
    {
    int numOut = src->GetNumberOfOutputs();
    int i;
    for (i = 0; i < numOut; ++i)
      {
      if (!this->IsOutputReady(i,pp))
        {
        return 0;
        }
      }
    }
  return 1;
}

void vtkPushPipelineProcessInfo::ConsumeAllInputs(vtkPushPipeline *pp)
{
  int numIn = this->ProcessObject->GetNumberOfInputs();
  int i;
  for (i = 0; i < numIn; ++i)
    {
    if (this->InputToExecutionRatio[i] && 
        (this->NumberProcessed % this->InputToExecutionRatio[i] == 0))
      {
      this->ConsumeInput(i,pp);
      }
    }
}

void vtkPushPipelineProcessInfo::ProduceOutputs(vtkPushPipeline *pp)
{
  if (this->NumberProcessed % this->ExecutionToOutputRatio == 0)
    {
    vtkSource *src = vtkSource::SafeDownCast(this->ProcessObject);
    if (src)
      {
      int numOut = src->GetNumberOfOutputs();
      int i;
      for (i = 0; i < numOut; ++i)
        {
        // mark output as produced, in this case not consumed
        vtkPushPipelineDataInfo *pdi = pp->GetPushDataInfo(src->GetOutputs()[i]);
        if (pdi)
          {
          pdi->FillConsumersLeft();
          }
        }
      }
    }
}

void vtkPushPipelineDataInfo::FillConsumersLeft()
{
  this->ConsumersLeft->RemoveAllItems();
  int numCon = this->DataObject->GetNumberOfConsumers();
  int i;
  for (i = 0; i < numCon; ++i)
    {
    vtkProcessObject *con = 
      vtkProcessObject::SafeDownCast(this->DataObject->GetConsumer(i));
    if (con)
      {
      this->ConsumersLeft->AppendItem(con);
      }
    }
  // now add any window consumers that we know of
  this->WindowConsumersLeft->RemoveAllItems();
  this->WindowConsumersLeft->CopyItems(this->WindowConsumers);
}

void vtkPushPipelineDataInfo::ConsumeData(vtkProcessObject *po)
{
  if (this->IsConsumerLeft(po))
    {
    vtkIdType id = 0;
    this->ConsumersLeft->FindItem(po,id);
    this->ConsumersLeft->RemoveItem(id);
    }
}

void vtkPushPipelineDataInfo::ConsumeWindow(vtkRenderWindow *rw)
{
  if (this->IsWindowConsumerLeft(rw))
    {
    vtkIdType id = 0;
    this->WindowConsumersLeft->FindItem(rw,id);
    this->WindowConsumersLeft->RemoveItem(id);
    }
}

int vtkPushPipelineDataInfo::IsConsumerLeft(vtkProcessObject *po)
{
  if (this->ConsumersLeft->IsItemPresent(po))
    {
    return 1;
    }
  return 0;
}

int vtkPushPipelineDataInfo::IsWindowConsumerLeft(vtkRenderWindow *rw)
{
  if (this->WindowConsumersLeft->IsItemPresent(rw))
    {
    return 1;
    }
  return 0;
}

int vtkPushPipelineProcessInfo::IsOutputReady(int i, vtkPushPipeline *pp)
{
  vtkSource *src = vtkSource::SafeDownCast(this->ProcessObject);
  if (src)
    {  
    vtkPushPipelineDataInfo *pdi = pp->GetPushDataInfo(src->GetOutputs()[i]);
    if (!pdi)
      {
      vtkGenericWarningMacro("Attempt to check output status for an output that is unknown to the vtkPushPiepline");
      return 0;
      }
    if (pdi->ConsumersLeft->GetNumberOfItems())
      {
      return 0;
      }
    if (pdi->WindowConsumersLeft->GetNumberOfItems())
      {
      return 0;
      }
    return 1;
    }
  return 0;  
}

void vtkPushPipelineProcessInfo::ConsumeInput(int i, vtkPushPipeline *pp)
{
  vtkPushPipelineDataInfo *pdi = pp->GetPushDataInfo(this->ProcessObject->GetInputs()[i]);
  if (!pdi)
    {
    vtkGenericWarningMacro("Attempt to check input status for aninput that is unknown to the vtkPushPipeline");
    return;
    }
  
  pdi->ConsumeData(this->ProcessObject);
}

void vtkPushPipelineProcessInfo::ProcessSomeData(vtkPushPipeline *pp)
{
  // if it is a mapper force a render?
  vtkAbstractMapper *mpr = 
    vtkAbstractMapper::SafeDownCast(this->ProcessObject);
  if (mpr)
    {
    return;
    }
  
  vtkSource *src = vtkSource::SafeDownCast(this->ProcessObject);
  if (src)
    {
    // pass the push to the pusher
    src->InvokeEvent(vtkCommand::NextDataEvent,pp);
    src->UpdateInformation();
    
    if (src->GetOutputs()[0])
      {
      src->GetOutputs()[0]->SetUpdateExtentToWholeExtent();
      src->UpdateData(src->GetOutputs()[0]);
      }
    }
  this->NumberProcessed++;              
  this->ConsumeAllInputs(pp);
  this->ProduceOutputs(pp);
}


void vtkPushPipeline::SetInputToExecutionRatio(vtkProcessObject *po, int inNum, int ratio)
{
  vtkPushPipelineProcessInfo *pi = this->GetPushProcessInfo(po);
  if (!pi)
    {
    vtkWarningMacro("failed to find process object in pushpipeline");
    return;
    }
  
  pi->InputToExecutionRatio[inNum] = ratio;
}

void vtkPushPipeline::SetExecutionToOutputRatio(vtkProcessObject *po, 
                                                int ratio)
{
  vtkPushPipelineProcessInfo *pi = this->GetPushProcessInfo(po);
  if (!pi)
    {
    vtkWarningMacro("failed to find process object in pushpipeline");
    return;
    }
  
  pi->ExecutionToOutputRatio = ratio;
}

void vtkPushPipeline::AddWindow(vtkRenderWindow *win)
{
  this->Windows->AppendItem(win);
}

void vtkPushPipeline::RenderWindows()
{
  // look at all associated render windows and render any that
  // have all their data ready
  int wi;
  vtkRenderWindow *rwp;
  for (wi = 0; wi < this->Windows->GetNumberOfItems(); wi++)
    {
    
    this->Windows->GetItem(wi,rwp);
    if (this->IsRenderWindowReady(rwp))
      {
      rwp->Render();
      this->ConsumeRenderWindowInputs(rwp);
      }
    }
}

int vtkPushPipeline::IsRenderWindowReady(vtkRenderWindow *win)
{
  // are all the mappers of this window, that we know about, ready
  vtkRendererCollection *rc = win->GetRenderers();
  vtkRenderer *ren;
  for (rc->InitTraversal(); (ren = rc->GetNextItem()) != NULL;)
    {
    if (!this->IsRendererReady(ren))
      {
      return 0;
      }
    }
  return 1;
}

int vtkPushPipeline::IsRendererReady(vtkRenderer *ren)
{
  int ready = 1;
  
  // check for other props
  vtkPropCollection *pc = ren->GetProps();
  vtkProp *prop;
  for (pc->InitTraversal(); (prop = pc->GetNextProp()) != NULL;)
    {

    // look for image actors
    vtkImageActor *ia = vtkImageActor::SafeDownCast(prop);
    if (ia)
      {
      vtkImageData *id = ia->GetInput();
      vtkPushPipelineDataInfo *pdi = this->GetPushDataInfo(id);
      if (pdi)
        {
        if (!pdi->IsWindowConsumerLeft(ren->GetRenderWindow()))
          {
          ready = 0;
          }
        }
      }

    // look for volumes
    vtkVolume *v = vtkVolume::SafeDownCast(prop);
    if (v)
      {
      vtkVolumeMapper *vm = v->GetMapper();
      if (vm)
        {
        vtkPushPipelineProcessInfo *ppi =
          this->GetPushProcessInfo(vm);
        if (ppi && !ppi->AreAllInputsReady(this))
          {
          ready = 0;
          }
        }
      }
    }
  return ready;
}


void vtkPushPipeline::ConsumeRenderWindowInputs(vtkRenderWindow *win)
{
  // are all the mappers of this window, that we know about, ready
  vtkRendererCollection *rc = win->GetRenderers();
  vtkRenderer *ren;
  for (rc->InitTraversal(); (ren = rc->GetNextItem()) != NULL;)
    {
    this->ConsumeRendererInputs(ren);
    }
}

void vtkPushPipeline::ConsumeRendererInputs(vtkRenderer *ren)
{
  // check for other props
  vtkPropCollection *pc = ren->GetProps();
  vtkProp *prop;
  for (pc->InitTraversal(); (prop = pc->GetNextProp()) != NULL;)
    {
    vtkImageActor *ia = vtkImageActor::SafeDownCast(prop);
    if (ia)
      {
      vtkImageData *id = ia->GetInput();
      vtkPushPipelineDataInfo *pdi = this->GetPushDataInfo(id);
      if (pdi)
        {
        pdi->ConsumeWindow(ren->GetRenderWindow());
        }
      }
    }
}

void vtkPushPipeline::SetupWindows()
{
  // look at all associated render windows and render any that
  // have all their data ready
  int wi;
  vtkRenderWindow *rwp;
  for (wi = 0; wi < this->Windows->GetNumberOfItems(); wi++)
    {
    this->Windows->GetItem(wi,rwp);
    this->SetupRenderWindow(rwp);
    }
}

void vtkPushPipeline::SetupRenderWindow(vtkRenderWindow *win)
{
  // are all the mappers of this window, that we know about, ready
  vtkRendererCollection *rc = win->GetRenderers();
  vtkRenderer *ren;
  for (rc->InitTraversal(); (ren = rc->GetNextItem()) != NULL;)
    {
    this->SetupRenderer(ren);
    }
}

void vtkPushPipeline::SetupRenderer(vtkRenderer *ren)
{
  // check for other props
  vtkPropCollection *pc = ren->GetProps();
  vtkProp *prop;
  for (pc->InitTraversal(); (prop = pc->GetNextProp()) != NULL;)
    {

    // look for image actors
    vtkImageActor *ia = vtkImageActor::SafeDownCast(prop);
    if (ia)
      {
      vtkImageData *id = ia->GetInput();
      vtkPushPipelineDataInfo *pdi = this->GetPushDataInfo(id);
      if (pdi)
        {
        if (!pdi->WindowConsumers->IsItemPresent(ren->GetRenderWindow()))
          {
          pdi->WindowConsumers->AppendItem(ren->GetRenderWindow());
          }
        }
      }
    }
}

void vtkPushPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "RunState: " << this->RunState << "\n";
}
