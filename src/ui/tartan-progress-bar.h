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
#ifndef TARTAN_PROGRESS_BAR_H
#define TARTAN_PROGRESS_BAR_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "player.h"
#include "image-cache.h"
#include "game-map.h"
#include "player.h"
#include "stack-list.h"
#include "shield-set.h"
#include "pixmask.h"
#include "player-list.h"
#include "action.h"
#include "lw.h"

//! A progress bar that shows an image of a tartan
/**
 * The idea here is that we're going to show a player-specific image of
 * a progressbar.
 * The whole image is generated in the ImageCache, and here in this class
 * we choose how much of it to show, and how wide it should be overall.
 *
 */
class TartanProgressBar: public Gtk::DrawingArea
{
 public:

    // use this percent of the width available to us.  1 = 100%
    const double TARTAN_PERCENT_WIDTH = 1.00;

    //always show this much of the tartan.
    const double MIN_PERCENT = 0.10;

    TartanProgressBar ()
      {
        m_percent = 0;
      }

    ~TartanProgressBar ()
      {
      }

    void init_turn (Player *p)
      {
        m_player = p;
        m_percent = 0.0;
        m_max_stacks = m_player->getStacklist ()->size ();
      }

    void pulse ()
      {
        double new_percent = calculate_percentage ();
        if (new_percent == m_percent)
          return;
        m_percent = new_percent;
        queue_draw ();
      }

 protected:

    void snapshot_vfunc (const Glib::RefPtr<Gtk::Snapshot>& snapshot) override
      {
        if (m_player == Playerlist::getNeutral ())
          return;

        if (is_visible () && m_player && get_width () > 0)
          {
            PixMask *p = 
              ImageCache::instance ()->getTartanPic
              (GameMap::getShieldset (), m_player->get_shield (),
               get_width () * TARTAN_PERCENT_WIDTH);

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

            p = 
              ImageCache::instance ()->getEmptyTartanPic
              (GameMap::getShieldset (), m_player->get_shield (),
               get_width () * TARTAN_PERCENT_WIDTH);

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
          }

        return;
      }

 private:

    Player *m_player = NULL;
    double m_percent = 0.0;
    guint32 m_max_stacks;

    double calculate_percentage () const
      {
        /*
         * it's a difficult problem to know what our progress bar should be
         * at.
         *
         * here we count the number of times we moved a unique stack and
         * compare against our total number of stacks, which could decrease
         * and increase as we go.
         */
        auto actions = m_player->getStackMoveActionEntries ();
        actions.sort
          ([](Action_Move* a, Action_Move* b)
           {
             return a->getStackId () < b->getStackId ();
           });

        actions.unique
          ([](Action_Move* a, Action_Move* b)
           {
             return a->getStackId () == b->getStackId ();
           });

        guint32 max = m_max_stacks;
        if (max == 0)
          max = 1;
        guint32 count = actions.size ();
        if (count > max)
          count = max;

        return (double) count / (double)max;
      }
};

#endif
