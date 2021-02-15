//  Copyright (C) 2007, 2008, 2012, 2014, 2020 Ben Asselstine
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
#include "bar-chart.h"
#include "ucompose.hpp"
#include <cairomm/context.h>
#include "font-size.h"

BarChart::BarChart(const std::list<unsigned int> &bars,
                   const std::list<Gdk::RGBA> &colors, unsigned int max_value)
 : d_bars (bars), d_colors (colors), d_max_value (max_value)
{
}

bool BarChart::on_draw (const Cairo::RefPtr<Cairo::Context> &cr)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // labels
    Glib::RefPtr<Pango::Layout> layout = Glib::wrap (pango_cairo_create_layout (cr->cobj ()));

    Glib::ustring text_font =
      String::ucompose ("Sans %1", FontSize::getInstance()->get_height ());
    Pango::FontDescription font_desc (text_font);
    layout->set_font_description (font_desc);
    layout->set_text("0");
    int w, h;
    layout->get_pixel_size (w, h);

    unsigned int lw = 10;

    // clip to the area indicated by the expose event so that we only redraw
    // the portion of the window that needs to be redrawn
    cr->rectangle(0, 0, width, height);
    cr->clip();
    cr->set_source_rgb (0.11328125, 0.11328125, 0.11328125);
    cr->set_line_width(1000.0);
    cr->move_to(0,0);
    cr->line_to(width, height);
    cr->stroke();
    cr->set_line_width((double)lw);

    unsigned int max = 0;
    if (d_max_value == 0)
      {
	for (std::list<unsigned int>::iterator bit = d_bars.begin();
             bit != d_bars.end(); ++bit)
	  {
	    if (*bit > max)
	      max = *bit;
	  }
	if (max < 10)
	  max = 10;
	else if (max < 100)
	  max = 100;
	else if (max < 250)
	  max = 250;
	else if (max < 500)
	  max = 500;
	else if (max < 1000)
	  max = 1000;
	else if (max < 1500)
	  max = 1500;
	else if (max < 2500)
	  max = 2500;
	else if (max < 3500)
	  max = 3500;
	else if (max < 5000)
	  max = 5000;
	else if (max < 7500)
	  max = 7500;
	else if (max < 10000)
	  max = 10000;
	else if (max < 25000)
	  max = 25000;
	else if (max < 50000)
	  max = 50000;
	else if (max < 100000)
	  max = 100000;
      }
    else
      max = d_max_value;

    unsigned int voffs = 15;
    unsigned int hoffs = 15;
    unsigned int d = ((height-voffs-lw-h)/d_colors.size())-lw;
    cr->move_to(0, 0);
    std::list<Gdk::RGBA>::iterator cit = d_colors.begin();
    unsigned int i = 0;
    for (std::list<unsigned int>::iterator bit = d_bars.begin();
         bit != d_bars.end(), cit != d_colors.end(); ++bit, ++cit, i+=(lw+d))
      {
	cr->move_to(hoffs, i + lw + voffs);
	double red = (*cit).get_red();
	double green = (*cit).get_green();
	double blue = (*cit).get_blue();
	cr->set_source_rgb(red, green, blue);
	cr->line_to(((float) *bit / (float)max) * (width - hoffs) + hoffs, 
		    i + lw + voffs);
	cr->stroke();
      }
    cr->set_source_rgb (1.0, 1.0, 1.0);
    lw = 2;
    cr->set_line_width((double)lw);

    // draw the line across the bottom
    cr->move_to(hoffs, i + lw + voffs);
    cr->line_to(((float)max / (float)max) * (width - (hoffs * 2)), 
		i + lw + voffs);
    cr->stroke();

    //now the three ticks
    cr->move_to(hoffs + 1, i + lw + voffs + 1);
    cr->line_to(hoffs + 1, i + lw + voffs + (voffs / 2) + 1);
    cr->stroke();
    cr->move_to(((float)0.25 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), i + lw + voffs + 1);
    cr->line_to(((float)0.25 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), 
		i + lw + voffs + (voffs / 4) + 1);
    cr->stroke();
    cr->move_to(((float)0.5 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), i + lw + voffs + 1);
    cr->line_to(((float)0.5 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), 
		i + lw + voffs + (voffs / 2) + 1);
    cr->stroke();
    cr->move_to(((float)0.75 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), i + lw + voffs + 1);
    cr->line_to(((float)0.75 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), 
		i + lw + voffs + (voffs / 4) + 1);
    cr->stroke();
    cr->move_to(((float)1.0 * ((float)width - ((float)hoffs * 2.0))) - 1, i + lw + voffs + 1);
    cr->line_to(((float)1.0 * ((float)width - ((float)hoffs * 2.0))) - 1, 
		i + lw + voffs + (voffs / 2) + 1);
    cr->stroke();

    // labels
    cr->move_to(hoffs + 1 - (w / 2), i + lw + voffs + (voffs / 2) + 1);
    cr->set_source_rgb (1.0, 1.0, 1.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    layout->set_text(String::ucompose("%1", max / 2));
    layout->get_pixel_size (w, h);
    cr->move_to(((float)0.5 * ((float)width - (hoffs * 2.0))) + (hoffs / 2) - 
		( w/2), i + lw + voffs + (voffs / 2) + 1);
    cr->set_source_rgb (1.0, 1.0, 1.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    layout->set_text(String::ucompose("%1", max));
    layout->get_pixel_size (w, h);
    cr->move_to(((float)1.0 * ((float)width - ((float)hoffs * 2.0))) - 1 - 
		(w / 2), i + lw + voffs + (voffs / 2) + 1);
    cr->set_source_rgb (1.0, 1.0, 1.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());
  }

  return true;
}
