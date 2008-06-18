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

/// \file vtkQtChartSeriesDomainGroup.cxx
/// \date March 6, 2008

#include "vtkQtChartSeriesDomainGroup.h"


vtkQtChartSeriesDomainGroup::vtkQtChartSeriesDomainGroup(bool sortSeries)
  : Groups()
{
  this->SortSeries = sortSeries;
}

int vtkQtChartSeriesDomainGroup::getNumberOfGroups() const
{
  return this->Groups.size();
}

int vtkQtChartSeriesDomainGroup::getNumberOfSeries(int group) const
{
  if(group >= 0 && group < this->Groups.size())
    {
    return this->Groups[group].size();
    }

  return 0;
}

QList<int> vtkQtChartSeriesDomainGroup::getGroup(int group) const
{
  if(group >= 0 && group < this->Groups.size())
    {
    return this->Groups[group];
    }

  return QList<int>();
}

int vtkQtChartSeriesDomainGroup::findGroup(int series) const
{
  QList<QList<int> >::ConstIterator iter = this->Groups.begin();
  for(int i = 0; iter != this->Groups.end(); ++iter, ++i)
    {
    if(iter->contains(series))
      {
      return i;
      }
    }

  return -1;
}

void vtkQtChartSeriesDomainGroup::prepareInsert(int seriesFirst,
    int seriesLast)
{
  // Increase the series index for all series with indexes after the
  // insertion range.
  int diff = seriesLast - seriesFirst + 1;
  QList<QList<int> >::Iterator iter = this->Groups.begin();
  for( ; iter != this->Groups.end(); ++iter)
    {
    QList<int>::Iterator jter = iter->begin();
    for( ; jter != iter->end(); ++jter)
      {
      if(*jter >= seriesFirst)
        {
        *jter += diff;
        }
      }
    }
}

void vtkQtChartSeriesDomainGroup::insertSeries(int series, int group)
{
  if(group < 0)
    {
    group = 0;
    }

  if(group >= this->Groups.size())
    {
    group = this->Groups.size();
    this->insertGroup(group);
    }

  if(this->SortSeries)
    {
    bool doAdd = true;
    QList<int>::Iterator iter = this->Groups[group].begin();
    for( ; iter != this->Groups[group].end(); ++iter)
      {
      if(series < *iter)
        {
        this->Groups[group].insert(iter, series);
        doAdd = false;
        break;
        }
      }

    if(doAdd)
      {
      this->Groups[group].append(series);
      }
    }
  else
    {
    this->Groups[group].append(series);
    }
}

int vtkQtChartSeriesDomainGroup::removeSeries(int series)
{
  QList<QList<int> >::Iterator iter = this->Groups.begin();
  for(int i = 0; iter != this->Groups.end(); ++iter, ++i)
    {
    if(iter->contains(series))
      {
      iter->removeAll(series);
      return i;
      }
    }

  return -1;
}

void vtkQtChartSeriesDomainGroup::finishRemoval(int seriesFirst,
    int seriesLast)
{
  // Decrease the series index for all series with indexes after the
  // removed range. Remove any empty groups.
  bool doUpdate = seriesFirst != -1 && seriesLast != -1;
  int diff = seriesLast - seriesFirst + 1;
  int i = 0;
  QList<int>::Iterator jter;
  QList<QList<int> >::Iterator iter = this->Groups.begin();
  while(iter != this->Groups.end())
    {
    if(iter->size() == 0)
      {
      iter = this->Groups.erase(iter);
      this->removeGroup(i);
      }
    else if(doUpdate)
      {
      for(jter = iter->begin(); jter != iter->end(); ++jter)
        {
        if(*jter > seriesLast)
          {
          *jter -= diff;
          }
        }

      ++iter;
      ++i;
      }
    else
      {
      ++iter;
      ++i;
      }
    }
}

void vtkQtChartSeriesDomainGroup::clear()
{
  this->Groups.clear();
}

void vtkQtChartSeriesDomainGroup::insertGroup(int group)
{
  this->Groups.insert(group, QList<int>());
}

void vtkQtChartSeriesDomainGroup::removeGroup(int)
{
}


