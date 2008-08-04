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

/// \file vtkQtBarChart.h
/// \date February 15, 2008

#ifndef _vtkQtBarChart_h
#define _vtkQtBarChart_h

#include "vtkQtChartExport.h"
#include "vtkQtChartSeriesLayer.h"

class vtkQtBarChartInternal;
class vtkQtBarChartOptions;
class vtkQtBarChartSeriesOptions;


/// \class vtkQtBarChart
/// \brief
///   The vtkQtBarChart class is used to display a bar chart.
class VTKQTCHART_EXPORT vtkQtBarChart : public vtkQtChartSeriesLayer
{
  Q_OBJECT

public:
  enum {Type = vtkQtChart_BarChartType};

public:
  vtkQtBarChart();
  virtual ~vtkQtBarChart();

  /// \name Setup Methods
  //@{
  virtual void setChartArea(vtkQtChartArea *area);

  virtual void setModel(vtkQtChartSeriesModel *model);
  //@}

  /// \name Drawing Parameters
  //@{
  /// \brief
  ///   Gets the bar chart drawing options.
  /// \return
  ///   A pointer to the bar chart drawing options.
  vtkQtBarChartOptions *getOptions() const {return this->Options;}

  /// \brief
  ///   Sets the bar chart drawing options.
  ///
  /// This method sets all the options at once, which can prevent
  /// unnecessary view updates.
  ///
  /// \param options The new bar chart drawing options.
  void setOptions(const vtkQtBarChartOptions &options);

  vtkQtBarChartSeriesOptions *getBarSeriesOptions(int series) const;
  //@}

  /// \name Layout Methods
  //@{
  virtual void getLayerDomain(vtkQtChartLayerDomain &domain) const;

  virtual void layoutChart(const QRectF &area);

  virtual bool drawItemFilter(QGraphicsItem *item, QPainter *painter);

  virtual bool getHelpText(const QPointF &point, QString &text);
  //@}

  /// \name Selection Methods
  //@{
  virtual void getSeriesAt(const QPointF &point,
      vtkQtChartSeriesSelection &selection) const;

  virtual void getPointsAt(const QPointF &point,
      vtkQtChartSeriesSelection &selection) const;

  virtual void getSeriesIn(const QRectF &area,
      vtkQtChartSeriesSelection &selection) const;

  virtual void getPointsIn(const QRectF &area,
      vtkQtChartSeriesSelection &selection) const;
  //@}

  /// \name QGraphicsItem Methods
  //@{
  virtual QRectF boundingRect() const;

  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
      QWidget *widget=0);
  //@}

public slots:
  void reset();

protected:
  virtual vtkQtChartSeriesOptions *createOptions(QObject *parent);
  virtual void setupOptions(vtkQtChartSeriesOptions *options);

private slots:
  void prepareSeriesInsert(int first, int last);
  void insertSeries(int first, int last);
  void startSeriesRemoval(int first, int last);
  void finishSeriesRemoval(int first, int last);
  void handleAxesCornerChange();
  void handleOutlineChange();
  void handleSeriesVisibilityChange(bool visible);
  void handleSeriesPenChange(const QPen &pen);
  void handleSeriesBrushChange(const QBrush &brush);
  void updateHighlights();

private:
  void layoutHighlights();
  bool addSeriesDomain(int series);
  void calculateDomain(int seriesGroup);

private:
  vtkQtBarChartInternal *Internal; ///< Stores the bar series.
  vtkQtBarChartOptions *Options;   ///< Stores the drawing options.
  bool InModelChange;              ///< Used for selection changes.
};

#endif
