//  Copyright (C) 2008, 2009, 2014, 2020, 2026 Ben Asselstine
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
#include "lw-dialog-base.h"
#ifndef DIPLOMACY_REPORT_DIALOG_H
#define DIPLOMACY_REPORT_DIALOG_H
class DiplomacyReportDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "diplomacy-report.ui";
      }

    DiplomacyReportDialog (BaseObjectType* o,
                           const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_diplomacy_table = load <Gtk::Grid> ("diplomacy_table");
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        fill_table ();
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Grid *m_diplomacy_table;

    void fill_table ()
      {
        int order[MAX_PLAYERS];

        Gtk::Grid *grid = m_diplomacy_table;

        /* find the diplomatic order of the players */
        for (guint32 i = 0; i < MAX_PLAYERS; i++)
          {
            order[i] = -1;
            for (auto p : *Playerlist::instance ())
              {
                if (Playerlist::getNeutral () == p)
                  continue;
                if (p->isDead () == true)
                  continue;
                if (i != p->getDiplomaticRank () - 1)
                  continue;
                order[i] = (int) p->getId ();
              }
          }

        /* show the players in order of their diplomatic ranking. */
        for (guint32 i = 0; i < MAX_PLAYERS; i++)
          {
            if (order[i] == -1)
              continue;
            Player *p = Playerlist::instance ()->get (order[i]);

            auto image =
              ImageCache::instance ()->getShieldPic (2, p, false)->to_pixbuf ();
            auto im = Gtk::make_managed<Gtk::Image> ();
            im->set_pixel_size (LW_BUTTON_SIZE);
            im->set (image);
            grid->attach (*im, 1, i + 1, 1, 1);
            auto im2 = Gtk::make_managed<Gtk::Image> ();
            im2->set_pixel_size (LW_BUTTON_SIZE);
            im2->set (image);
            grid->attach (*im2, i + 2, 0, 1, 1);
            Gtk::Label *label =
              Gtk::make_managed<Gtk::Label>(p->getDiplomaticTitle ());
            grid->attach (*label, 0, i + 1, 1, 1);

            for (guint32 j = 0; j < MAX_PLAYERS; j++)
              {
                if (order[j] == -1)
                  continue;
                auto state = p->getDiplomaticState
                  (Playerlist::instance ()->get (order[j]));

                auto pix2 = ImageCache::instance ()->getDiplomacyPic
                  (0, state)->to_pixbuf ();
                auto im3 = Gtk::make_managed<Gtk::Image>();
                im3->set_pixel_size (LW_BUTTON_SIZE * 0.6666);
                im3->set (pix2);
                grid->attach (*im3, i + 2, j + 1, 1, 1);
              }
          }
      }
};
#endif
