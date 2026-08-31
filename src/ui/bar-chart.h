//  Copyright (C) 2007, 2008, 2009, 2012, 2014 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#pragma once
#ifndef BAR_CHART_H
#define BAR_CHART_H

#include <gtkmm.h>
#include <list>

class BarChart: public Gtk::Image
{
public:
    BarChart(const std::list<unsigned int> &bars,
             const std::list<Gdk::RGBA> &colors, unsigned int max_value);
    virtual ~BarChart() {};

protected:
    //Override default signal handler:
    virtual bool on_draw (const Cairo::RefPtr<Cairo::Context> &cr);
private:
    std::list<unsigned int> d_bars;
    std::list<Gdk::RGBA> d_colors;
    unsigned int d_max_value;
};

#endif
