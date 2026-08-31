//  Copyright (C) 2026 Ben Asselstine
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

#include <gtkmm.h>
#ifndef TURN_INDICATOR_H
#define TURN_INDICATOR_H
#include "player-list.h"
#include "image-cache.h"
#include "game-map.h"
#include "shield-set.h"

class TurnShield : public Gtk::Widget
{
public:
    TurnShield (Glib::RefPtr<Gdk::Pixbuf> shield, bool active, bool dark)
      {
        m_active = active;
        try
          {
            texture = Gdk::Texture::create_for_pixbuf (shield);
            set_size_request (texture->get_width () * 2.20,
                              texture->get_height () * 2.20);
          }
        catch (const Glib::Error &ex)
          {
            texture.reset ();
          }
        m_dark = dark;
      }

protected:
    void snapshot_vfunc (const Glib::RefPtr<Gtk::Snapshot>& snapshot) override
      {
        const int width = get_width ();
        const int height = get_height ();

        Gdk::RGBA white;
        white.set_rgba (1.0, 1.0, 1.0, 1.0);
        Gdk::RGBA black;
        black.set_rgba (0.0, 0.0, 0.0, 1.0);

        Gdk::RGBA bgcolor;
        if (m_dark)
          {
            if (m_active)
              bgcolor = white;
            else
              bgcolor = black;
          }
        else
          {
            if (m_active)
              bgcolor = black;
            else
              bgcolor = white;
          }

        graphene_rect_t rect;
        graphene_rect_init (&rect, 0, 0, width, height);

        snapshot->append_color (bgcolor, &rect);

        if (texture)
          {
            int img_w = texture->get_width ();
            int img_h = texture->get_height ();

            float x = (width - img_w) / 2.0f;
            float y = (height - img_h) / 2.0f;

            graphene_rect_t img_rect;
            graphene_rect_init (&img_rect, x, y, img_w, img_h);

            snapshot->append_texture (texture, &img_rect);
          }
      }

private:
    Glib::RefPtr<Gdk::Texture> texture;
    bool m_active;
    bool m_dark;
};

class TurnIndicator : public Gtk::Box
{
public:
    TurnIndicator ()
      {
        set_margin_top (1);
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
                 draw_shields ();
               });
          }

        set_vexpand (false);
        set_valign (Gtk::Align::CENTER);
        m_turn = Gtk::make_managed<Gtk::Label> ();
        m_turn->set_margin_end (6);
        update_round (1);
        append (*m_turn);
        draw_shields ();
      }

    void update_round (int round)
      {
        m_turn->set_text (Glib::ustring::compose (_("Turn %1"), round));
        draw_shields ();
      }

    ~TurnIndicator ()
      {
        m_dark_style_handler.disconnect ();
      }

private:
    Gtk::Label *m_turn;
    bool m_dark;
    std::vector<Gtk::Widget*> images;
    sigc::connection m_dark_style_handler;

    void draw_shields ()
      {
        for (auto i : images)
          remove (*i);
        images.clear ();

        auto sid = GameMap::getShieldset ()->getId ();
        for (auto p : *Playerlist::instance ())
          {
            if (p != Playerlist::getNeutral () && !p->isDead ())
              {
                auto pid = p->getId ();
                auto pic =
                  ImageCache::instance ()->getShieldPic (sid, 1, pid,
                                                         false)->to_pixbuf ();

                bool active = p == Playerlist::getActiveplayer ();
                auto t = Gtk::make_managed<TurnShield> (pic, active, m_dark);
                images.push_back (t);
                append (*t);
              }
          }
      }
};
#endif
