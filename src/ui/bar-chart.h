//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2020, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#ifndef BAR_CHART_H
#define BAR_CHART_H
#include <gtkmm.h>
#include <cairomm/context.h>
#include <list>
#include <algorithm>

#include "lw.h"

class BarChart : public Gtk::DrawingArea
{
public:
  BarChart (const std::list<unsigned int>& bars,
            const std::list<Gdk::RGBA>& colors, unsigned int max_value)
      : d_bars (bars), d_colors (colors), d_max_value (max_value)
  {
    set_draw_func (sigc::mem_fun (*this, &BarChart::on_draw));
    m_dark = Lw::get_dark ();
    auto iface = Gio::Settings::create ("org.gnome.desktop.interface",
                                        "/org/gnome/desktop/interface/");
    if (iface)
      {
        m_dark_style_handler =
          iface->signal_changed ().connect
          ([this, iface] (const Glib::ustring &key)
           {
             (void)key;
             auto scheme = iface->get_string ("color-scheme");
             if (scheme == "prefer-dark")
               m_dark = true;
             else
               m_dark = false;
             queue_draw ();
           });
      }
  }

~BarChart ()
  {
    m_dark_style_handler.disconnect ();
  }

private:
  bool m_dark;
  sigc::connection m_dark_style_handler;

  void on_draw (const Cairo::RefPtr<Cairo::Context>& cr, int width, int height)
    {
      // labels
      auto pango_context = get_pango_context ();
          
      int w, h;
        {
          auto layout = Pango::Layout::create (pango_context);
          layout->set_text ("0");
          layout->get_pixel_size (w, h);
        }

      unsigned int lw = 10;

      unsigned int max = 0;
      if (d_max_value == 0)
        {
          for (auto bit = d_bars.begin (); bit != d_bars.end (); ++bit)
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
      unsigned int d = ((height - voffs - lw - h) / d_colors.size ()) - lw;
      cr->move_to (0, 0);
      std::list<Gdk::RGBA>::iterator cit = d_colors.begin ();
      unsigned int i = 0;
      for (auto bit = d_bars.begin (); bit != d_bars.end () && 
           cit != d_colors.end (); ++bit, ++cit, i += (lw + d))
        {
          cr->move_to (hoffs, i + lw + voffs);
          double red = (*cit).get_red ();
          double green = (*cit).get_green ();
          double blue = (*cit).get_blue ();

          double halfw = lw / 2.0;

          double x1 = hoffs;
          double y1 = i + lw + voffs;
          double x2 = ((float) *bit / (float)max) * (width - hoffs) + hoffs;
          double y2 = i + lw + voffs;

          double left   = std::min (x1, x2) - halfw + halfw;
          double right  = std::max (x1, x2) + halfw;
          double top    = std::min (y1, y2) - halfw;
          double bottom = std::max (y1, y2) + halfw;

          cr->rectangle (left, top, right - left, bottom - top);

          cr->set_source_rgb (red, green, blue);
          cr->fill_preserve ();

          set_line_color (cr);
          cr->set_line_width (2.0);
          cr->stroke ();

        }

      set_line_color (cr);
      lw = 2;
      cr->set_line_width ((double)lw);

      // draw the line across the bottom
      cr->move_to (hoffs, i + lw + voffs);
      cr->line_to (((float)max / (float)max) * (width - (hoffs * 2)), 
                   i + lw + voffs);
      cr->stroke ();

      // draw the vertical line plus a little more for the tick
      cr->move_to (hoffs, i + lw + voffs + (voffs / 2) + 1);
      cr->line_to (hoffs, 
                   lw + voffs );
      cr->stroke ();

      //now the ticks
      cr->move_to (((float)0.25 * ((float)width - (hoffs * 2.0))) + (hoffs / 2),
                   i + lw + voffs + 1);
      cr->line_to (((float)0.25 * ((float)width - (hoffs * 2.0))) + (hoffs / 2),
                   i + lw + voffs + (voffs / 4) + 1);
      cr->stroke ();
      cr->move_to (((float)0.5 * ((float)width - (hoffs * 2.0))) + (hoffs / 2),
                   i + lw + voffs + 1);
      cr->line_to (((float)0.5 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), 
                   i + lw + voffs + (voffs / 2) + 1);
      cr->stroke ();
      cr->move_to (((float)0.75 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), i + lw + voffs + 1);
      cr->line_to (((float)0.75 * ((float)width - (hoffs * 2.0))) + (hoffs / 2), 
                   i + lw + voffs + (voffs / 4) + 1);
      cr->stroke ();
      cr->move_to (((float)1.0 * ((float)width - ((float)hoffs * 2.0))) - 1, i + lw + voffs + 1);
      cr->line_to (((float)1.0 * ((float)width - ((float)hoffs * 2.0))) - 1, 
                   i + lw + voffs + (voffs / 2) + 1);
      cr->stroke ();

      // labels
            
        {
          auto layout = Pango::Layout::create (pango_context);
          layout->set_text ("0");
          cr->move_to (hoffs + 1 - (w / 2), i + lw + voffs + (voffs / 2) + 1);
          set_line_color (cr);
          layout->show_in_cairo_context (cr);
        }

        {
          auto layout = Pango::Layout::create (pango_context);
          layout->set_text (String::ucompose ("%1", max / 2));
          layout->get_pixel_size (w, h);
          cr->move_to (((float)0.5 * ((float)width - (hoffs * 2.0))) + (hoffs / 2) - 
                       (w / 2), i + lw + voffs + (voffs / 2) + 1);
          set_line_color (cr);
          layout->show_in_cairo_context (cr);
        }

        {
          auto layout = Pango::Layout::create (pango_context);
          layout->set_text (String::ucompose ("%1", max));
          layout->get_pixel_size (w, h);
          cr->move_to (((float)1.0 * ((float)width - ((float)hoffs * 2.0))) - 1 - 
                       (w / 2), i + lw + voffs + (voffs / 2) + 1);
          set_line_color (cr);
          layout->show_in_cairo_context (cr);
        }
    }

    void set_line_color (const Cairo::RefPtr<Cairo::Context>& cr)
      {
        if (Lw::get_dark ())
          cr->set_source_rgb(1.0, 1.0, 1.0);
        else
          cr->set_source_rgb(0.0, 0.0, 0.0);
      }

private:
    std::list<unsigned int> d_bars;
    std::list<Gdk::RGBA> d_colors;
    unsigned int d_max_value;
};
#endif
