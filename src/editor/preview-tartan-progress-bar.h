//  Copyright (C) 2017, 2026 Ben Asselstine
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
#ifndef PREVIEW_TARTAN_PROGRESS_BAR_H
#define PREVIEW_TARTAN_PROGRESS_BAR_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "image-cache.h"
#include "shield-set.h"
#include "pixmask.h"

//! A progress bar that shows an image of a tartan
/**
 * The idea here is that we're going to show a player-specific image of
 * a progressbar.
 * The whole image is generated in the ImageCache, and here in this class
 * we choose how much of it to show, and how wide it should be overall.
 *
 */
class PreviewTartanProgressBar: public Gtk::DrawingArea
{
 public:

    // use this percent of the width available to us.  1 = 100%
    const double TARTAN_PERCENT_WIDTH = 1.00;

    //always show this much of the tartan.
    const double MIN_PERCENT = 0.10;

    PreviewTartanProgressBar (Shieldset *shieldset, Shield::Color shield)
      {
        m_shieldset = shieldset;
        m_shield = shield;
        m_percent = 0;
      }

    ~PreviewTartanProgressBar ()
      {
      }

    void pulse ()
      {
        m_percent += 0.05;
        if (m_percent > 1)
          m_percent = 0.0;
        queue_draw ();
      }

 protected:

    void snapshot_vfunc (const Glib::RefPtr<Gtk::Snapshot>& snapshot) override
      {
        if (is_visible () && get_width () > 0)
          {

            PixMask *p = Shield::get_progress_bar_completed
              (m_shieldset, m_shield, get_width () * TARTAN_PERCENT_WIDTH);

            if (m_percent < MIN_PERCENT) 
              m_percent = MIN_PERCENT;

            double offset_x = (get_width () - p->get_width ()) / 2;
            guint32 limit = (double)p->get_width () * m_percent;

            double x = offset_x, y = 0, w = limit, h = p->get_height ();
            graphene_rect_t dest_rect;
            graphene_rect_init (&dest_rect, x, y, w, h);

            auto sub =
              Gdk::Pixbuf::create_subpixbuf (p->to_pixbuf (), 0, 0, limit,
                                             p->get_height ());
            auto texture = Gdk::Texture::create_for_pixbuf (sub);
            snapshot->append_texture (texture, &dest_rect);
            
            delete p;

            p = Shield::get_progress_bar_uncompleted
              (m_shieldset, m_shield, get_width () * TARTAN_PERCENT_WIDTH);

            x = limit + offset_x;
            y = 0;
            w = p->get_width () - limit;
            h = p->get_height ();
            graphene_rect_t dest2_rect;
            graphene_rect_init (&dest2_rect, x, y, w, h);

            if (p->get_width () - limit > 0)
              {
                sub =
                  Gdk::Pixbuf::create_subpixbuf (p->to_pixbuf (), limit, 0,
                                                 p->get_width () - limit,
                                                 p->get_height ());
                texture = Gdk::Texture::create_for_pixbuf (sub);
                snapshot->append_texture (texture, &dest2_rect);
              }
            delete p;
          }

        return;
      }

 private:

    Shieldset *m_shieldset;
    Shield::Color m_shield;
    double m_percent = 0.0;
};

#endif
