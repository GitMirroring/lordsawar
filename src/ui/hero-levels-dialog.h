//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2020, 2026 Ben Asselstine
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
#ifndef HERO_LEVELS_DIALOG_H
#define HERO_LEVELS_DIALOG_H
#include "lw-dialog-base.h"
#include "lw-column.h"
class HeroLevelsRow: public Glib::Object
{
public:
    Hero *m_hero;

    static Glib::RefPtr<HeroLevelsRow> create (Hero *h)
      {
        return
          Glib::make_refptr_for_instance<HeroLevelsRow> (new HeroLevelsRow (h));
      }

protected:
    HeroLevelsRow (Hero *h)
      : m_hero (h)
      {
      }
};

class HeroLevelsDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "hero-levels.ui";
      }

    HeroLevelsDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        m_store = Gio::ListStore<HeroLevelsRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        for (auto h : Playerlist::getActiveplayer ()->getHeroes ())
          m_store->append (HeroLevelsRow::create (h));

        setup_hero_image_column ();
        setup_hero_name_column ();
        setup_hero_level_column ();
        setup_hero_exp ();
        setup_hero_needs ();
        setup_hero_str ();
        setup_hero_moves ();
      }

private:
    Gtk::Button *m_close_button = NULL;
    Gtk::ColumnView *m_treeview = NULL;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<HeroLevelsRow>> m_store;

    void setup_hero_image_column ()
      {
        LwColumn::setup_picture_column<HeroLevelsRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "hero_image", "",
           [] (const auto& row)
           {
             Player *p = row->m_hero->getOwner ();
             auto im = 
               ImageCache::instance ()->getCircledArmyPic
               (p->getArmyset (), row->m_hero->getTypeId (), p->get_shield (),
                NULL, false, p->getId (), true, Lw::get_dark ())->to_texture ();
             return im;
           });
      }

    void setup_hero_name_column ()
      {
        LwColumn::setup_text_column<HeroLevelsRow>
          (m_treeview, "hero_name", true, Gtk::Justification::LEFT,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_hero->getName ();
           });
      }

    void setup_hero_level_column ()
      {
        LwColumn::setup_text_column<HeroLevelsRow>
          (m_treeview, "hero_level", false, Gtk::Justification::CENTER,
           _("Level"),
           [] (const auto& row)
           {
             return String::ucompose ("%1", row->m_hero->getLevel ());
           });
      }

    void setup_hero_exp ()
      {
        LwColumn::setup_text_column<HeroLevelsRow>
          (m_treeview, "hero_xp", false, Gtk::Justification::CENTER,
           _("Exp"),
           [] (const auto& row)
           {
             char buf[32];
             snprintf (buf, sizeof (buf), "%.2lf", row->m_hero->getXP ());

             return String::ucompose ("%1", std::string (buf));
           });
      }

    void setup_hero_needs ()
      {
        LwColumn::setup_text_column<HeroLevelsRow>
          (m_treeview, "hero_needs", false, Gtk::Justification::CENTER,
           _("Exp For\nNext Level"),
           [] (const auto& row)
           {
             return String::ucompose ("%1",
                                      row->m_hero->getXpNeededForNextLevel ());
           });
      }

    void setup_hero_str ()
      {
        LwColumn::setup_text_column<HeroLevelsRow>
          (m_treeview, "hero_str", false, Gtk::Justification::CENTER,
           _("Str"),
           [] (const auto& row)
           {
             return String::ucompose ("%1",
                                      row->m_hero->getStat (Army::STRENGTH,
                                                            true));

           });
      }

    void setup_hero_moves ()
      {
        LwColumn::setup_text_column<HeroLevelsRow>
          (m_treeview, "hero_moves", false, Gtk::Justification::CENTER,
           _("Moves"),
           [] (const auto& row)
           {
             return String::ucompose ("%1",
                                      row->m_hero->getStat (Army::MOVES, true));
           });
      }

};
#endif
