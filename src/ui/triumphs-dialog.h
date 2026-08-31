//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2015, 2020, 2026 Ben Asselstine
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
#ifndef TRIUMPHS_DIALOG_H
#define TRIUMPHS_DIALOG_H
#include "triumphs.h"
class TriumphsDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "triumphs.ui";
      }

    TriumphsDialog (BaseObjectType* o,
                    const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_box = load <Gtk::Box> ("box");
      }

    ~TriumphsDialog ()
      {
        m_dark_style_handler.disconnect ();
      }

    void setup (Player *player)
      {
        m_player = player;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_notebook = Gtk::make_managed<Gtk::Notebook> ();
        m_box->append (*m_notebook);
    
        setup_dark_mode_change ();
        fill_in_info ();
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Box *m_box;
    Gtk::Notebook *m_notebook;
    Player *m_player;
    bool m_dark;
    sigc::connection m_dark_style_handler;

    guint32 tally (Player *p, Triumphs::TriumphType type)
      {
        guint32 count = 0;
        if (p == m_player)
          {
            // add up what the other players did to us
            for (auto player : *Playerlist::instance ())
              {
                if (player == Playerlist::getNeutral ())
                  continue;
                count += player->getTriumphs ()->getTriumphTally (p, type);
              }
          }
        else
          {
            // add up what we did to that player
            count = m_player->getTriumphs ()->getTriumphTally (p, type);
          }
        return count;
      }

    void fill_in_page (Player *p)
      {
        //here we tally up the stats, make a vbox and append it as a new page
        //tally it up differently when p == d_player

        guint32 count;
        Glib::ustring s;
        count = tally (p, Triumphs::TALLY_HERO);
        if (p == m_player)
          s = String::ucompose
            (ngettext ("%1 hero earned fates worthy of legend!",
                       "%1 heroes earned fates worthy of legend!",
                       count), count);
        else
          s = String::ucompose
            (ngettext
             ("%1 so-called hero slaughtered without mercy!",
              "%1 so-called heroes slaughtered without mercy!",
              count), count);

        auto hero_label = Gtk::make_managed<Gtk::Label> (s);

        const ArmyProto *hero = NULL;
        const Armysetlist* al = Armysetlist::instance ();
        //let's go find the hero army
        Armyset *as = al->get (p->getArmyset ());
        for (auto army : *as)
          {
            const ArmyProto *a = al->getArmy (p->getArmyset (), army->getId ());
            if (a->isHero ())
              {
                hero = a;
                break;
              }
          }
        auto hero_image = Gtk::make_managed<Gtk::Image> ();
        hero_image->set_pixel_size (LW_BUTTON_SIZE);
        hero_image->set
          (ImageCache::instance ()->getCircledArmyPic
           (p->getArmyset (), hero->getId (), p->get_shield (), NULL, false,
            Shield::NEUTRAL, true, Lw::get_dark ())->to_pixbuf ());

        auto hero_hbox =
          Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        hero_hbox->set_spacing (10);
        hero_hbox->append (*hero_image);
        hero_hbox->append (*hero_label);

        count = tally (p, Triumphs::TALLY_SHIP);
        if (p == m_player)
          s = String::ucompose
            (ngettext ("%1 navy not currently in service!",
                       "%1 navies not currently in service!",
                       count), count);
        else
          s = String::ucompose
            (ngettext ("%1 navy rests with the fishes!",
                       "%1 navies rest with the fishes!",
                       count), count);

        auto ship_label = Gtk::make_managed<Gtk::Label> (s);
        auto ship_image = Gtk::make_managed<Gtk::Image> ();
        ship_image->set_pixel_size (LW_BUTTON_SIZE);
        ship_image->set
          (ImageCache::instance ()->getCircledShipPic
          (p->getArmyset (), p->get_shield (), false,
           Shield::NEUTRAL, Lw::get_dark ())->to_pixbuf ());

        auto ship_hbox =
          Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        ship_hbox->set_spacing (10);
        ship_hbox->append (*ship_image);
        ship_hbox->append (*ship_label);

        count = tally (p, Triumphs::TALLY_NORMAL);
        if (p == m_player)
          s = String::ucompose
            (ngettext ("%1 army died to ensure final victory!",
                       "%1 armies died to ensure final victory!",
                       count), count);
        else
          s = String::ucompose
            (ngettext ("%1 army smote like sheep!",
                       "%1 armies smote like sheep!",
                       count), count);

        auto normal_label = Gtk::make_managed<Gtk::Label>(s);
        auto normal_image = Gtk::make_managed<Gtk::Image>();
        normal_image->set_pixel_size (LW_BUTTON_SIZE);
        normal_image->set
          (ImageCache::instance ()->getCircledArmyPic
           (p->getArmyset (), 0, p->get_shield (), NULL, false, Shield::NEUTRAL,
            true, Lw::get_dark ())->to_pixbuf ());

        auto normal_hbox =
          Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        normal_hbox->set_spacing (10);
        normal_hbox->append (*normal_image);
        normal_hbox->append (*normal_label);

        count = tally (p, Triumphs::TALLY_SPECIAL);
        if (p == m_player)
          s = String::ucompose
            (ngettext ("%1 unnatural creature returned from whence it came!",
                       "%1 unnatural creatures returned from whence they came!",
                       count), count);
        else
          s = String::ucompose
            (ngettext ("%1 unnatural creature dispatched!",
                       "%1 unnatural creatures dispatched!",
                       count), count);
        auto special_label = Gtk::make_managed<Gtk::Label>(s);
        //let's go find a special army
        const ArmyProto *special = NULL;
        for (auto army : *as)
          {
            const ArmyProto *a = al->getArmy (p->getArmyset (), army->getId ());
            if (a->getAwardable ())
              {
                special = a;
                break;
              }
          }
        auto special_image = Gtk::make_managed<Gtk::Image>();
        special_image->set_pixel_size (LW_BUTTON_SIZE);
        special_image->set
          (ImageCache::instance ()->getCircledArmyPic
           (p->getArmyset (), special->getId (), p->get_shield (), NULL, false,
            Shield::NEUTRAL, true, Lw::get_dark ())->to_pixbuf ());
        auto special_hbox =
          Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
        special_hbox->set_spacing (10);
        special_hbox->append (*special_image);
        special_hbox->append (*special_label);

        count = tally (p, Triumphs::TALLY_FLAG);
        if (p == m_player)
          s = String::ucompose
            (ngettext ("%1 standard betrayed by its guardian!",
                       "%1 standards betrayed by its guardian!",
                       count), count);
        else
          s = String::ucompose
            (ngettext ("%1 standard wrested from a vanquished foe!",
                       "%1 standards wrested from a vanquished foe!",
                       count), count);
        auto flag_label = Gtk::make_managed<Gtk::Label>(s);
        auto flag_image = Gtk::make_managed<Gtk::Image>();
        flag_image->set_pixel_size (LW_BUTTON_SIZE);
        flag_image->set
          (ImageCache::instance ()->getCircledStandardPic
          (p->getArmyset (), p->get_shield (), false,
           Shield::NEUTRAL, Lw::get_dark ())->to_pixbuf ());
        auto flag_hbox =
          Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        flag_hbox->set_spacing (10);
        flag_hbox->append (*flag_image);
        flag_hbox->append (*flag_label);

        Gtk::Box *contents =
          Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
        contents->append (*normal_hbox);
        contents->append (*special_hbox);
        contents->append (*hero_hbox);
        contents->append (*ship_hbox);
        contents->append (*flag_hbox);
        contents->set_margin (6);

        auto shield_image = Gtk::make_managed<Gtk::Image>();
        shield_image->set_pixel_size (LW_BUTTON_SIZE);
        shield_image->set
          (ImageCache::instance ()->getShieldPic (2, p, false)->to_pixbuf ());
        m_notebook->append_page (*contents, *shield_image);
      }

    void fill_in_info ()
      {
        while (m_notebook->get_n_pages () > 0)
          m_notebook->remove_page (0);

        for (auto p : *Playerlist::instance ())
          {
            if (p == Playerlist::getNeutral ())
              continue;
            fill_in_page (p);
          }
      }

    void setup_dark_mode_change ()
      {
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
                 fill_in_info ();
               });
          }
      }
};
#endif
