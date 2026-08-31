//  Copyright (C) 2008, 2009, 2011, 2014, 2020, 2026 Ben Asselstine
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
#ifndef DIPLOMACY_DIALOG_H
#define DIPLOMACY_DIALOG_H
#include "diplomacy-report-dialog.h"
class DiplomacyDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "diplomacy.ui";
      }

    DiplomacyDialog (BaseObjectType* o,
                     const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_proposals_table = load <Gtk::Grid> ("proposals_table");
        m_offers_table = load <Gtk::Grid> ("offers_table");
        m_player_label = load <Gtk::Label> ("player_label");
        m_player_shield_image = load <Gtk::Image> ("player_shield_image");
        m_report_button = load <Gtk::Button> ("report_button");
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        fill_proposals ();
        fill_offers ();

        auto p = Playerlist::getActiveplayer ();
        m_player_shield_image->set
          (ImageCache::instance ()->getShieldPic (2, p, false)->to_pixbuf ());
        m_player_shield_image->set_pixel_size (LW_BUTTON_SIZE);
        m_player_label->set_text (p->getName ());

        m_report_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build<DiplomacyReportDialog> (this);
             d->setup ();
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

      }
private:
    Gtk::Button *m_close_button;
    Gtk::Grid *m_proposals_table;
    Gtk::Grid *m_offers_table;
    Gtk::Label *m_player_label;
    Gtk::Image *m_player_shield_image;
    Gtk::Button *m_report_button;

    void fill_proposals ()
      {
        guint32 i = 0;
        for (auto p : *Playerlist::instance ())
          {
            if (Playerlist::getNeutral () == p)
              continue;

            if (p == Playerlist::getActiveplayer ())
              continue;

            auto pixbuf =
              ImageCache::instance ()->getShieldPic (2, p, false)->to_pixbuf ();
            Gtk::Image *im = Gtk::make_managed<Gtk::Image> ();
            im->set_pixel_size (LW_BUTTON_SIZE);
            im->set (pixbuf);
            m_proposals_table->attach (*im, i, 0, 1, 1);

            i++;
          }

        Player *active = Playerlist::getActiveplayer ();
        i = 0;
        for (auto p : *Playerlist::instance ())
          {
            if (Playerlist::getNeutral () == p)
              continue;
            if (p == active)
              continue;
            if (p->isDead ())
              {
                i++;
                continue;
              }
            guint32 j = 0;
            auto state = active->getDiplomaticState (p);
            auto pixbuf2 =
              ImageCache::instance ()->getDiplomacyPic (1, state)->to_pixbuf ();
            Gtk::Image *im = Gtk::make_managed<Gtk::Image>();
            im->set_pixel_size (LW_BUTTON_SIZE);
            im->set (pixbuf2);
            im->add_css_class ("border-image");
            im->set_vexpand (false);
            im->set_hexpand (false);
            im->set_valign (Gtk::Align::CENTER);
            im->set_halign (Gtk::Align::CENTER);

            m_proposals_table->attach (*im, i, j + 1, 1, 1);

            auto proposal = p->getDiplomaticProposal (active);
            j = 1;
            Glib::RefPtr<Gdk::Pixbuf> pixbuf;
            switch (proposal)
              {
              case Player::PROPOSE_PEACE:
                pixbuf = ImageCache::instance ()->getDiplomacyPic
                  (1, Player::AT_PEACE)->to_pixbuf ();
                break;

              case Player::PROPOSE_WAR_IN_FIELD:
                pixbuf = ImageCache::instance ()->getDiplomacyPic
                  (1, Player::AT_WAR_IN_FIELD)->to_pixbuf ();
                break;

              case Player::PROPOSE_WAR:
                pixbuf = ImageCache::instance ()->getDiplomacyPic
                  (1, Player::AT_WAR)->to_pixbuf ();
                break;

              case Player::NO_PROPOSAL:
                pixbuf =
                  Gdk::Pixbuf::create (Gdk::Colorspace::RGB,
                                       true, 8,
                                       LW_BUTTON_SIZE,
                                       LW_BUTTON_SIZE);
                pixbuf->fill (0x00000000);
                break;

              default:
                break;
              }

            auto im2 = Gtk::make_managed<Gtk::Image>();
            im2->set_pixel_size (LW_BUTTON_SIZE);
            im2->set (pixbuf);
            im2->add_css_class ("border-image");
            im2->set_vexpand (false);
            im2->set_hexpand (false);
            im2->set_valign (Gtk::Align::CENTER);
            im2->set_halign (Gtk::Align::CENTER);
            m_proposals_table->attach (*im2, i, j + 2, 1, 1);

            i++;
          }
      }

    void fill_offers ()
      {
        // fill in the togglebuttons
        guint32 i = 0;
        auto active = Playerlist::getActiveplayer ();
        for (auto p : *Playerlist::instance ())
          {
            if (Playerlist::getNeutral () == p)
              continue;

            if (p == active)
              continue;

            //show the peace radio buttons
            guint32 j = 0;
            auto radio1 = make_radio (Player::AT_PEACE, _("Propose peace"));
            if (p->isDead ())
              radio1->set_sensitive (false);
            else
              radio1->set_active (active->getDiplomaticProposal (p) == 
                                  Player::PROPOSE_PEACE);
            radio1->signal_toggled ().connect
              ([this, radio1, active, p] ()
               {
                 if (radio1->get_active() == true)
                   active->proposeDiplomacy (Player::PROPOSE_PEACE, p);
               });
            m_offers_table->attach (*radio1, i, j, 1, 1);

            j = 1;
            auto radio2 = make_radio (Player::AT_WAR_IN_FIELD,
                                      _("Propose war on armies not in cities"));
            radio2->set_group (*radio1);
            if (p->isDead ())
              radio2->set_sensitive (false);
            else
              radio2->set_active (active->getDiplomaticProposal (p) == 
                                  Player::PROPOSE_WAR_IN_FIELD);
            radio2->signal_toggled ().connect
              ([this, radio2, active, p] ()
               {
                 if (radio2->get_active() == true)
                   active->proposeDiplomacy (Player::PROPOSE_WAR_IN_FIELD, p);
               });
            m_offers_table->attach (*radio2, i, j, 1, 1);

            j = 2;
            auto radio3 = make_radio (Player::AT_WAR, _("Propose war"));
            radio3->set_group (*radio1);
            if (p->isDead ())
              radio3->set_sensitive (false);
            else
              radio3->set_active (active->getDiplomaticProposal (p) == 
                                  Player::PROPOSE_WAR);
            radio3->signal_toggled ().connect
              ([this, radio3, active, p] ()
               {
                 if (radio3->get_active() == true)
                   active->proposeDiplomacy (Player::PROPOSE_WAR, p);
               });
            m_offers_table->attach (*radio3, i, j, 1, 1);
            i++;
          }
      }

    Gtk::ToggleButton* make_radio (Player::DiplomaticState state, Glib::ustring tooltip)
      {
        auto radio = Gtk::make_managed<Gtk::ToggleButton> ();
        auto im = Gtk::make_managed<Gtk::Image> ();
        im->set_pixel_size (LW_BUTTON_SIZE);
        auto pix = ImageCache::instance ()->getDiplomacyPic (1, state);
        im->set (pix->to_pixbuf ());
        radio->set_tooltip_text (tooltip);
        radio->set_child (*im);
        return radio;
      }
};
#endif
