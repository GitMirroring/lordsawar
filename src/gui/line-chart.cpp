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

#include "line-chart.h"
#include "ucompose.hpp"
#include <cairomm/context.h>
#include "font-size.h"

LineChart::LineChart(const std::list<std::list<unsigned int> > &lines, 
		     const std::list<Gdk::RGBA> &colors, 
		     unsigned int max_height_value,
		     Glib::ustring x_axis_description,
		     Glib::ustring y_axis_description)
 : d_lines (lines), d_colors (colors), d_max_height_value (max_height_value),
    d_x_indicator (-1), d_x_axis_description (x_axis_description),
    d_y_axis_description (y_axis_description)
{
}

bool LineChart::on_draw (const Cairo::RefPtr<Cairo::Context> &cr)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    int origin_x = 0;
    int origin_y = height;
    unsigned int hoffs = 30;
    unsigned int voffs = 30;

    // clip to the area indicated by the expose event so that we only redraw
    // the portion of the window that needs to be redrawn
    cr->rectangle(0, 0, width, height);
    cr->clip();
    cr->set_source_rgb (0.11328125, 0.11328125, 0.11328125);
    cr->set_line_width(1000.0);
    cr->move_to(0,0);
    cr->line_to(width,height);
    cr->stroke();

    cr->set_line_width(1.0);
    //loop through the outer list, and operate on the inner lists
    unsigned int max_turn = 0;
    for (std::list<std::list<unsigned int> >::iterator line = d_lines.begin();
         line!= d_lines.end(); ++line)
      {
	if ((*line).size() > max_turn)
	  max_turn = (*line).size();
      }

    if (d_max_height_value == 0)
      {
        for (std::list<std::list<unsigned int> >::iterator line = d_lines.begin();
             line!= d_lines.end(); ++line)
	  {
	    for (std::list<unsigned int>::iterator it = (*line).begin();
                 it != (*line).end(); ++it)
	      {
		if (*it > d_max_height_value)
		  d_max_height_value = *it;
	      }
	  }
      }

    // ensure the border is big enough for the label.
    Glib::RefPtr<Pango::Layout> layout = Glib::wrap (pango_cairo_create_layout (cr->cobj ()));
    Glib::ustring text_font =
      String::ucompose ("Sans %1", FontSize::getInstance()->get_height ());
    Pango::FontDescription font_desc (text_font);
    layout->set_font_description (font_desc);

    layout->set_text(String::ucompose("%1", d_max_height_value));
    int w, h;
    layout->get_pixel_size (w, h);
    if (w * (hoffs / 4) > hoffs)
      hoffs = w + (hoffs / 4);

    std::list<Gdk::RGBA>::iterator cit = d_colors.begin();
    for (std::list<std::list<unsigned int> >::iterator line = d_lines.begin();
         line!= d_lines.end(), cit != d_colors.end(); ++line, ++cit)
      {
	//okay, here's my line and it's color,
	double red = (*cit).get_red();
	double green = (*cit).get_green();
	double blue = (*cit).get_blue();
	cr->set_source_rgb(red, green, blue);
	unsigned int turn = 1;
	cr->move_to(origin_x + hoffs, origin_y - voffs);
	for (std::list<unsigned int>::iterator it = (*line).begin();
             it != (*line).end(); ++it, turn++)
	  {
	    cr->line_to((((float)turn / (float)max_turn) * 
			 (width - (hoffs * 2))) + hoffs,
			height - (voffs * 2) - 
			(((float)*it / (float)d_max_height_value) * 
			 (height - (voffs * 2.0))) + voffs);
	  }
	cr->stroke();
      }

    //draw horizontal axis
    cr->set_source_rgb (1.0, 1.0, 1.0);
    cr->move_to(origin_x + hoffs, origin_y - voffs);
    cr->line_to((width - (hoffs * 2)) + hoffs, origin_y - voffs);
    cr->stroke();

    //draw ticks on the horizontal axis
    cr->set_source_rgb(0.3, 0.3, 0.3);
    cr->move_to(origin_x + hoffs, origin_y - voffs + 1);
    cr->line_to(origin_x + hoffs, origin_y - voffs + (voffs / 4) + 1);
    cr->stroke();

    cr->move_to((width - (hoffs * 2)) + hoffs - 1, origin_y - voffs + 1);
    cr->line_to((width - (hoffs * 2)) + hoffs - 1, 
		origin_y - voffs + (voffs / 4) + 1);
    cr->stroke();

    //draw vertical axis
    cr->set_source_rgb(0.3, 0.3, 0.3);
    cr->move_to(origin_x + hoffs, origin_y - voffs);
    cr->line_to(origin_x + hoffs, voffs);
    cr->stroke();

    //draw ticks on the vertical axis
    cr->move_to(origin_x + hoffs - 1, origin_y - voffs);
    cr->line_to(origin_x + hoffs - (hoffs / 4) - 1, origin_y - voffs);
    cr->stroke();
    cr->move_to(origin_x + hoffs - 1, voffs + 1);
    cr->line_to(origin_x + hoffs - (hoffs / 4) - 1, voffs + 1);
    cr->stroke();

    //draw the indicator line
    if (d_x_indicator > -1 && (unsigned int) d_x_indicator <= max_turn)
      {
	//draw a line at turn x
	cr->set_source_rgb(0.0, 0.0, 0.0);
	cr->move_to((((float)d_x_indicator/ (float)max_turn) * (width - (hoffs * 2))) + hoffs, voffs);
	cr->line_to((((float)d_x_indicator/ (float)max_turn) * (width - (hoffs * 2))) + hoffs, height - voffs);
	cr->stroke();
      }

    // draw the labels on the horizontal axis
    layout->set_font_description (font_desc);
    layout->set_text("0");
    layout->get_pixel_size (w, h);
    cr->move_to(hoffs - (w / 2), origin_y - voffs + (voffs / 4));
    cr->set_source_rgb (0.0, 0.0, 0.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    layout->set_text(String::ucompose("%1", max_turn));
    layout->get_pixel_size (w, h);
    cr->move_to((width - (hoffs * 2)) + hoffs - (w / 2), 
		origin_y - voffs + (voffs / 4) + 1);
    cr->set_source_rgb (1.0, 1.0, 1.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    layout->set_text(d_y_axis_description);
    layout->get_pixel_size (w, h);
    cr->move_to((width / 2 - (hoffs * 1)) + hoffs - (w / 2), 
		origin_y - voffs + (voffs / 2) + 1);
    cr->set_source_rgb (0.0, 0.0, 0.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    // draw the labels on the vertical axis
    layout->set_font_description (font_desc);
    layout->set_text("0");
    layout->get_pixel_size (w, h);
    cr->move_to(origin_x + hoffs - (hoffs / 4) - 1 - w, origin_y - voffs - (h/2));
    cr->set_source_rgb (1.0, 1.0, 1.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    layout->set_text(String::ucompose("%1", d_max_height_value));
    layout->get_pixel_size (w, h);
    cr->move_to(origin_x + hoffs - (hoffs / 4) - 1 - w, voffs + 1 - (h/2));
    cr->set_source_rgb (1.0, 1.0, 1.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    PangoContext *context;
    PangoFontMap *fontmap;
    fontmap = (PangoFontMap *) pango_cairo_font_map_get_default ();
    context = pango_font_map_create_context (fontmap);
    pango_context_set_base_gravity(context, PANGO_GRAVITY_EAST);
    pango_context_set_gravity_hint(context, PANGO_GRAVITY_HINT_STRONG);
    layout->context_changed();

    layout->set_text(d_x_axis_description);
    layout->get_pixel_size (w, h);
    cr->move_to(0, height - (voffs * 2) - 
		(0.50 * (height - (voffs * 2.0))) + voffs + (w / 2));
    cr->rotate(-90 / (180.0 / G_PI));
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

  }

  return true;
}

void LineChart::set_x_indicator(int x)
{
  d_x_indicator = x;
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window)
    queue_draw();
}
