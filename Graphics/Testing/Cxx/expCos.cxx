//
// Brute force computation of Bessel functions. Might be better to create a
// filter (or source) object. Might also consider vtkSampleFunction.

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPlaneSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkPoints.h"
#include "vtkScalars.h"
#include "vtkWarpScalar.h"
#include "vtkDataSetMapper.h"
#include "vtkPolyData.h"
#include "vtkActor.h"

#include "vtkRegressionTestImage.h"

int main( int argc, char *argv[] )
{
  int i, numPts;
  float x[3];
  double r, deriv;

  vtkRenderer *ren = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
    renWin->AddRenderer(ren);
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);
  
  // create plane to warp
  vtkPlaneSource *plane = vtkPlaneSource::New();
    plane->SetResolution (300,300);

  vtkTransform *transform = vtkTransform::New();
     transform->Scale(10.0,10.0,1.0);

  vtkTransformPolyDataFilter *transF = vtkTransformPolyDataFilter::New();
     transF->SetInput(plane->GetOutput());
     transF->SetTransform(transform);
     transF->Update();

  // compute Bessel function and derivatives. This portion could be 
  // encapsulated into source or filter object.
  //
  vtkPolyData *input = transF->GetOutput();
  numPts = input->GetNumberOfPoints();
  vtkPoints *newPts = vtkPoints::New();
    newPts->SetNumberOfPoints(numPts);
  vtkScalars *derivs = vtkScalars::New();
    derivs->SetNumberOfScalars(numPts);
  vtkPolyData *bessel = vtkPolyData::New();
    bessel->CopyStructure(input);
    bessel->SetPoints(newPts);
    bessel->GetPointData()->SetScalars(derivs);

  for (i=0; i<numPts; i++)
    {
    input->GetPoint(i,x);
    r = sqrt((double)x[0]*x[0] + x[1]*x[1]);
    x[2] = exp(-r) * cos (10.0*r);
    newPts->SetPoint(i,x);
    deriv = -exp(-r) * (cos(10.0*r) + 10.0*sin(10.0*r));
    derivs->SetScalar(i,deriv);
    }
  newPts->Delete(); //reference counting - it's ok
  derivs->Delete();
  
  // warp plane
  vtkWarpScalar *warp = vtkWarpScalar::New();
    warp->SetInput(bessel);
    warp->XYPlaneOn();
    warp->SetScaleFactor(0.5);

  // mapper and actor
  vtkDataSetMapper *mapper = vtkDataSetMapper::New();
    mapper->SetInput(warp->GetOutput());
    mapper->SetScalarRange(bessel->GetScalarRange());

  vtkActor *carpet = vtkActor::New();
    carpet->SetMapper(mapper);

  // assign our actor to the renderer
  ren->AddActor(carpet);
  ren->SetBackground(1,1,1);
  renWin->SetSize(300,300);

  // draw the resulting scene
  ren->GetActiveCamera()->Zoom(1.4);
  ren->GetActiveCamera()->Elevation(-55);
  ren->GetActiveCamera()->Azimuth(25);
  ren->ResetCameraClippingRange();
  renWin->Render();

  int retVal = vtkRegressionTestImage( renWin );

  // Clean up
  ren->Delete();
  renWin->Delete();
  iren->Delete();
  plane->Delete();
  transform->Delete();
  transF->Delete();
  bessel->Delete();
  warp->Delete();
  mapper->Delete();
  carpet->Delete();

  return !retVal;
}
