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
#include <vector>

#ifndef SELECT_STONE_POPOVER_H
#define SELECT_STONE_POPOVER_H
#include "tar-file-image.h"
class SelectStonePopover : public Gtk::Popover
{
public:
    SelectStonePopover ()
      {
        set_has_arrow (false);

        set_child (m_scrolled_window);

        m_scrolled_window.set_policy (Gtk::PolicyType::NEVER,
                                      Gtk::PolicyType::AUTOMATIC);
        m_scrolled_window.set_min_content_height (LW_BUTTON_SIZE * 5);

        m_scrolled_window.set_child (m_flowbox);

        m_flowbox.set_selection_mode (Gtk::SelectionMode::NONE);
        m_flowbox.set_max_children_per_line(5);
        m_flowbox.set_min_children_per_line(5);
        m_flowbox.set_valign (Gtk::Align::START);
      }

    void setup (Stone *stone, Road *road)
      {
        Gtk::ToggleButton *first = NULL;
        Gtk::ToggleButton *selected = NULL;

        std::vector<Stone::Type> types;
        if (road)
          types = Stone::getSuitableTypes (Road::Type(road->getType()));
        else
          types = Stone::getTypes ();

        for (auto t : types)
          {
            auto button = Gtk::make_managed<Gtk::ToggleButton>();
            auto image = Gtk::make_managed<Gtk::Image>();

            auto pix = make_pic (t, road);
            image->set (pix->to_pixbuf ());
            delete pix;
            image->set_pixel_size (LW_BUTTON_SIZE);
            button->set_child (*image);
            button->add_css_class ("editor-control-button");
            if (first)
              button->set_group (*first);
            else
              first = button;

            add_connection
              (button->signal_toggled ().connect
               ([this, t, button] ()
                {
                  if (button->get_active ())
                    {
                      m_signal_stone_selected.emit (t);
                      popdown ();
                    }
                }));

            if (t == stone->getType ())
              selected = button;
              
            m_flowbox.insert (*button, -1);
          }

        block_signals ();
        if (selected)
          selected->set_active (true);
        unblock_signals ();
      }

    sigc::signal<void(Stone::Type)> signal_stone_selected ()
      {
        return m_signal_stone_selected;
      }

private:

    Gtk::ScrolledWindow m_scrolled_window;
    Gtk::FlowBox m_flowbox;

    sigc::signal<void(Stone::Type)> m_signal_stone_selected;
    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void block_signals ()
      {
        for (auto c : m_connections)
          c.block ();
      }

    void unblock_signals ()
      {
        for (auto c : m_connections)
          c.unblock ();
      }

    PixMask* make_pic (Stone::Type t, Road *road)
      {
        Tileset *ts = GameMap::getTileset ();
        int siz = GameMap::getTileset()->getTileSize ();
        auto p = PixMask::create (siz);

        //first of all, go get a grass tile
        PixMask *grass = get_grass_image ();
        if (grass)
          {
            grass->blit (p->get_pixmap (), Vector<int>(0, 0));
            delete grass;
          }

        //go get the road if we're doing that
        if (road)
          {
            PixMask *r = ts->getRoad ()->getImage (road->getType ());
            if (r)
              r->blit (p->get_pixmap (), Vector<int>(0, 0));
          }

        //finally, do the stone
        PixMask *stone = ts->getStone ()->getImage(t);
        if (stone)
          stone->blit (p->get_pixmap (), Vector<int>(0, 0));

        return p;
      }

    PixMask *get_grass_image ()
      {
        Tileset *ts = GameMap::getTileset ();
        int idx = ts->getIndex (Tile::GRASS);
        if (idx == -1)
          return NULL;
        TileStyle *style = ts->getRandomTileStyle (idx, TileStyle::LONE);
        if (!style)
          return NULL;
        return style->getImage ()->copy ();
      }
};
#endif
