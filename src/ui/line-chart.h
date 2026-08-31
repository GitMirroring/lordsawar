//  Copyright (C) 2007, 2008, 2012, 2014, 2020, 2026 Ben Asselstine
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

#pragma once
#ifndef LINE_CHART_H
#define LINE_CHART_H
#include <gtkmm.h>
#include <cairomm/context.h>
#include <list>
#include <algorithm>

#include "lw.h"

#include <pangomm/layout.h>
#include <list>
#include <cmath>

class LineChart : public Gtk::DrawingArea
{
public:
    LineChart(const std::list<std::list<unsigned int>>& lines,
              const std::list<Gdk::RGBA>& colors,
              unsigned int max_height_value,
              const Glib::ustring& y_axis_description,
              const Glib::ustring& x_axis_description)
        : m_lines(lines),
          m_colors(colors),
          m_max_height_value(max_height_value),
          m_x_axis_description(x_axis_description),
          m_y_axis_description(y_axis_description)
    {
        set_draw_func (sigc::mem_fun(*this, &LineChart::draw));

        m_max_width_value = 0;
        for (auto ll : lines)
          if (ll.size () > m_max_width_value)
            m_max_width_value = ll.size ();
    }

    void set_x_indicator (int x)
      {
        m_x_indicator = x;
        queue_draw ();
      }
private:
    void set_line_color (const Cairo::RefPtr<Cairo::Context>& cr)
      {
        if (Lw::get_dark ())
          cr->set_source_rgb(1.0, 1.0, 1.0);
        else
          cr->set_source_rgb(0.0, 0.0, 0.0);
      }

    void draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height)
    {
        double left_margin = 45.0;
        double right_margin = 10.0;
        double top_margin = 10.0;
        double bottom_margin = 30.0;
        double plot_width = width - left_margin - right_margin;
        double plot_height = height - top_margin - bottom_margin;


        // Axes
        set_line_color (cr);
        cr->set_line_width (2.0);

        cr->move_to (left_margin, top_margin);
        cr->line_to (left_margin, height - bottom_margin);

        cr->line_to (width - right_margin,
                     height - bottom_margin);

        cr->stroke ();

        // Draw line series
        auto it = m_colors.begin ();
        for (const auto& line : m_lines)
          {
            if (line.size () < 2)
              {
                it++;
                continue;
              }

            size_t count = line.size ();

            const double x_step = plot_width / static_cast<double>(count - 1);

            double red = (*it).get_red ();
            double green = (*it).get_green ();
            double blue = (*it).get_blue ();
            cr->set_source_rgb (red, green, blue);
            cr->set_line_width (2.0);

            bool first = true;
            std::size_t index = 0;

            for (unsigned int value : line)
              {
                const double x = left_margin + index * x_step;

                double normalized = 0.0;

                if (m_max_height_value != 0)
                  {
                    normalized =
                      static_cast<double>(value) /
                      static_cast<double>(m_max_height_value);
                  }

                if (normalized > 1.0)
                  normalized = 1.0;

                const double y =
                  top_margin +
                  plot_height * (1.0 - normalized);

                if (first)
                  {
                    cr->move_to (x, y);
                    first = false;
                  }
                else
                  {
                    cr->line_to (x, y);
                  }

                ++index;
              }

            cr->stroke ();
            it++;
          }

        // indicator line
          {
            double normalized = 0.0;
            const std::size_t count = m_lines.front ().size ();
            const double x_step = plot_width / static_cast<double>(count - 1);
            const double x = left_margin + m_x_indicator * x_step;
            const double y = top_margin + plot_height * (1.0 - normalized);
            cr->save ();
            cr->move_to (x, y);
            set_line_color (cr);
            std::vector<double> dashes = { 2.0, 2.0 };
            cr->set_dash (dashes, 0.0);
            cr->line_to (x, top_margin);
            cr->stroke ();
            cr->unset_dash ();
            cr->restore ();
          }

        auto pango_context = get_pango_context ();

        // X axis label
        {
            auto layout = Pango::Layout::create (pango_context);
            layout->set_text (m_x_axis_description);

            int text_w = 0;
            int text_h = 0;
            layout->get_pixel_size (text_w, text_h);

            cr->move_to (left_margin +
                         ((plot_width - text_w) / 2.0),
                         height - text_h - 10.0);

            set_line_color (cr);

            layout->show_in_cairo_context (cr);
        }

        // Y axis label
        {
            auto layout = Pango::Layout::create (pango_context);
            layout->set_text (m_y_axis_description);

            int text_w = 0;
            int text_h = 0;
            layout->get_pixel_size (text_w, text_h);

            cr->save ();

            cr->move_to (20.0,
                         (top_margin +
                          (plot_height - text_h) / 2.0) + (text_w / 2));

            cr->rotate (-M_PI / 2.0);

            set_line_color (cr);

            layout->show_in_cairo_context (cr);

            cr->restore ();
        }

      // y axis value label
        {
            auto layout = Pango::Layout::create (pango_context);
            layout->set_text (String::ucompose ("%1", m_max_height_value));
            int text_w = 0;
            int text_h = 0;
            layout->get_pixel_size (text_w, text_h);

            cr->move_to (left_margin - text_w - 5,
                         top_margin - (text_h / 2));

            cr->save ();
            set_line_color (cr);
            layout->show_in_cairo_context (cr);
            cr->restore ();
        }

      // x axis value label
        {
            auto layout = Pango::Layout::create (pango_context);
            layout->set_text (String::ucompose ("%1", m_max_width_value));
            int text_w = 0;
            int text_h = 0;
            layout->get_pixel_size (text_w, text_h);

            cr->move_to (width - text_w - 5,
                         height - text_h - 10.0);

            cr->save ();
            set_line_color (cr);
            layout->show_in_cairo_context (cr);
            cr->restore ();
        }
    }

private:
    std::list<std::list<unsigned int>> m_lines;
    std::list<Gdk::RGBA> m_colors; // unused
    int m_x_indicator;

    unsigned int m_max_height_value;
    unsigned int m_max_width_value;

    Glib::ustring m_x_axis_description;
    Glib::ustring m_y_axis_description;
};
#endif
