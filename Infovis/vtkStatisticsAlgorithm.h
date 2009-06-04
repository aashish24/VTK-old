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
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
  -------------------------------------------------------------------------*/
// .NAME vtkStatisticsAlgorithm - Base class for statistics algorithms
//
// .SECTION Description
// All statistics algorithms can conceptually be operated with several options:
// * Learn: given an input data set, calculate a minimal statistical model (e.g., 
//   sums, raw moments, joint probabilities).
// * Derive: given an input minimal statistical model, derive the full model 
//   (e.g., descriptive statistics, quantiles, correlations, conditional
//    probabilities).
// * Assess: given an input data set, input statistics, and some form of 
//   threshold, assess a subset of the data set. 
// Therefore, a vtkStatisticsAlgorithm has the following vtkTable ports
// * 3 input ports:
//   * Data (mandatory)
//   * Parameters to the learn phase (optional)
//   * Input model (optional) 
// * 3 output port (called Output):
//   * Data (annotated with assessments when the Assess option is ON).
//   * Output model (identical to the the input model when Learn option is OFF).
//   * Meta information about the model and/or the overall fit of the data to the
//     model; is filled only when the Assess option is ON.
//
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this class.

#ifndef __vtkStatisticsAlgorithm_h
#define __vtkStatisticsAlgorithm_h

#include "vtkTableAlgorithm.h"

class vtkStdString;
class vtkStringArray;
class vtkVariantArray;
class vtkStatisticsAlgorithmPrivate;

class VTK_INFOVIS_EXPORT vtkStatisticsAlgorithm : public vtkTableAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkStatisticsAlgorithm, vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
// BTX
  // Description:
  // enumeration values to specify input port types
  enum InputPorts
    {
    INPUT_DATA = 0,         //!< Port 0 is for learn data
    LEARN_PARAMETERS = 1,   //!< Port 1 is for learn parameters (initial guesses, etc.)
    INPUT_MODEL = 2         //!< Port 2 is for a priori models
    };

  // Description:
  // enumeration values to specify output port types
  enum OutputIndices
    {
    OUTPUT_DATA = 0,        //!< Output 0 mirrors the input data, plus optional assessment columns
    OUTPUT_MODEL = 1,       //!< Output 1 contains any generated model
    ASSESSMENT = 2          //!< Output 2 
    };
// ETX

  // Description:
  // A convenience method for setting learn input parameters (if one is expected or allowed).
  // It is equivalent to calling SetInputConnection( 1, params );
  virtual void SetLearnParameterConnection( vtkAlgorithmOutput* params )
    { this->SetInputConnection( vtkStatisticsAlgorithm::LEARN_PARAMETERS, params ); }

  // Description:
  // A convenience method for setting learn input parameters (if one is expected or allowed).
  // It is equivalent to calling SetInput( 1, params );
  virtual void SetLearnParameters( vtkDataObject* params )
    { this->SetInput( vtkStatisticsAlgorithm::LEARN_PARAMETERS, params ); }

  // Description:
  // A convenience method for setting the input model (if one is expected or allowed).
  // It is equivalent to calling SetInputConnection( 2, model );
  virtual void SetInputModelConnection( vtkAlgorithmOutput* model )
    { this->SetInputConnection( vtkStatisticsAlgorithm::INPUT_MODEL, model ); }

  // Description:
  virtual void SetInputModel( vtkDataObject* model )
    { this->SetInput( vtkStatisticsAlgorithm::INPUT_MODEL, model ); }

  // Description:
  // Set the Learn option.
  vtkSetMacro( Learn, bool );

  // Description:
  // Get the Learn option.
  vtkGetMacro( Learn, bool );

  // Description:
  // Set the Derive option.
  vtkSetMacro( Derive, bool );

  // Description:
  // Get the Derive option.
  vtkGetMacro( Derive, bool );

  // Description:
  // Set the Assess option.
  vtkSetMacro( Assess, bool );

  // Description:
  // Get the Assess option.
  vtkGetMacro( Assess, bool );

  // Description:
  // Let the user know whether the full statistical model (when available) was
  // indeed derived from the underlying minimal model.
  // NB: It may be, or not be, a problem that a full model was not derived. For
  // instance, when doing parallel calculations, one only wants to derive the full
  // model after all partial calculations have completed. On the other hand, one
  // can also directly provide a full model, that was previously calculated or
  // guessed, and not derive a new one; in this case, IsFullModelDerived() will
  // always return false, but this does not mean that the full model is invalid 
  // (nor does it mean that it is valid).
  virtual int IsFullModelDerived() {return this->FullWasDerived;}

//BTX
  // Description:
  // Set the name of a parameter of the Assess option
  void SetAssessParameter( vtkIdType id, vtkStdString name );

  // Description:
  // Get the name of a parameter of the Assess option
  vtkStdString GetAssessParameter( vtkIdType id );

  // Description:
  // A base class for a functor that assesses data.
  class AssessFunctor {
  public:
    virtual void operator() ( vtkVariantArray*,
                              vtkIdType ) = 0;
    virtual ~AssessFunctor() { }
  };

  // Description:
  // A pure virtual method to select the appropriate assessment functor.
  virtual void SelectAssessFunctor( vtkTable* outData, 
                                    vtkDataObject* inMeta,
                                    vtkStringArray* rowNames,
                                    AssessFunctor*& dfunc ) = 0;
//ETX

  // Description:
  // Add or remove a column from the current analysis request.
  // Once all the column status values are set, call RequestSelectedColumns()
  // before selecting another set of columns for a different analysis request.
  // The way that columns selections are used varies from algorithm to algorithm.
  //
  // Note: the set of selected columns is maintained in vtkStatisticsAlgorithmPrivate::Buffer
  // until RequestSelectedColumns() is called, at which point the set is appended
  // to vtkStatisticsAlgorithmPrivate::Requests.
  // If there are any columns in vtkStatisticsAlgorithmPrivate::Buffer at the time
  // RequestData() is called, RequestSelectedColumns() will be called and the
  // selection added to the list of requests.
  virtual void SetColumnStatus( const char* namCol, int status );

  // Description:
  // Set the the status of each and every column in the current request to OFF (0).
  virtual void ResetAllColumnStates();

  // Description:
  // Use the current column status values to produce a new request for statistics
  // to be produced when RequestData() is called. See SetColumnStatus() for more information.
  virtual int RequestSelectedColumns();

  // Description:
  // Empty the list of current requests.
  virtual void ResetRequests();

  // Description:
  // Return the number of requests.
  // This does not include any request that is in the column-status buffer
  // but for which RequestSelectedColumns() has not yet been called (even though
  // it is possible this request will be honored when the filter is run -- see SetColumnStatus()
  // for more information).
  virtual vtkIdType GetNumberOfRequests();

  // Description:
  // Return the number of columns for a given request.
  virtual vtkIdType GetNumberOfColumnsForRequest( vtkIdType request );

  // Description:
  // Provide the name of the \a c-th column for the \a r-th request.
  //
  // For the version of this routine that returns an integer,
  // if the request or column does not exist because \a r or \a c is out of bounds,
  // this routine returns 0 and the value of \a columnName is unspecified.
  // Otherwise, it returns 1 and the value of \a columnName is set.
  //
  // For the version of this routine that returns const char*,
  // if the request or column does not exist because \a r or \a c is out of bounds,
  // the routine returns NULL. Otherwise it returns the column name.
  // This version is not thread-safe.
  virtual const char* GetColumnForRequest( vtkIdType r, vtkIdType c );
  //BTX
  virtual int GetColumnForRequest( vtkIdType r, vtkIdType c, vtkStdString& columnName );
  //ETX

protected:
  vtkStatisticsAlgorithm();
  ~vtkStatisticsAlgorithm();

  virtual int FillInputPortInformation( int port, vtkInformation* info );
  virtual int FillOutputPortInformation( int port, vtkInformation* info );

  virtual int RequestData(
    vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector* );

  // Description:
  // Execute the required calculations in the specified execution modes
  virtual void ExecuteLearn( vtkTable*,
                             vtkTable*,
                             vtkDataObject* ) = 0;
  virtual void ExecuteDerive( vtkDataObject* ) = 0;
  virtual void ExecuteAssess( vtkTable*,
                              vtkDataObject*,
                              vtkTable*,
                              vtkDataObject* ) = 0; 

  bool Learn;
  bool Derive;
  bool Assess;
  bool FullWasDerived;
  vtkStringArray* AssessParameters;
  vtkStringArray* AssessNames;
  vtkStatisticsAlgorithmPrivate* Internals;

private:
  vtkStatisticsAlgorithm(const vtkStatisticsAlgorithm&); // Not implemented
  void operator=(const vtkStatisticsAlgorithm&);   // Not implemented
};

#endif
