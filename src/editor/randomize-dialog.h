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
#include "lw-dialog-base.h"
#ifndef RANDOMIZE_DIALOG_H
#define RANDOMIZE_DIALOG_H
class RandomizeDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "randomize.ui";
      }

    RandomizeDialog (BaseObjectType* o,
                     const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_randomize_button = load <Gtk::Button> ("randomize_button");
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_cities_button = load <Gtk::CheckButton> ("cities_button");
        m_unnamed_cities_button =
          load <Gtk::CheckButton> ("unnamed_cities_button");
        m_ruins_button = load <Gtk::CheckButton> ("ruins_button");
        m_unnamed_ruins_button =
          load <Gtk::CheckButton> ("unnamed_ruins_button");
        m_temples_button = load <Gtk::CheckButton> ("temples_button");
        m_unnamed_temples_button =
          load <Gtk::CheckButton> ("unnamed_temples_button");
        m_signs_button = load <Gtk::CheckButton> ("signs_button");
        m_unnamed_signs_button =
          load <Gtk::CheckButton> ("unnamed_signs_button");
        m_status_label = load <Gtk::Label> ("status_label");
      }

    void randomize_objects ()
      {
        if (m_cities_button->get_active ())
          {
            for (auto c : *Citylist::instance ())
              randomize_city (c);
          }
        else if (m_unnamed_cities_button->get_active ())
          {
            for (auto c : *Citylist::instance ())
              {
                if (c->isUnnamed () == true)
                  randomize_city (c);
              }
          }

        if (m_ruins_button->get_active ())
          {
            for (auto r : *Ruinlist::instance ())
              randomize_ruin (r);
          }
        else if (m_unnamed_ruins_button->get_active ())
          {
            for (auto r : *Ruinlist::instance ())
              {
                if (r->isUnnamed ())
                  randomize_ruin (r);
              }
          }

        if (m_temples_button->get_active ())
          {
            for (auto t : *Templelist::instance ())
              randomize_temple (t);
          }
        else if (m_unnamed_temples_button->get_active ())
          {
            for (auto t : *Templelist::instance ())
              {
                if (t->isUnnamed ())
                  randomize_temple (t);
              }
          }

        if (m_signs_button->get_active ())
          {
            for (auto s : *Signpostlist::instance ())
              randomize_signpost (s);
          }
        else if (m_unnamed_signs_button->get_active ())
          {
            for (auto s : *Signpostlist::instance ())
              {
                if (s->getName() == DEFAULT_SIGNPOST)
                  randomize_signpost (s);
              }
          }
      }

    void setup (CreateScenarioRandomize *randomize)
      {
        m_randomize = randomize;

        set_response (m_randomize_button, Gtk::ResponseType::ACCEPT);
        set_response (m_cancel_button, Gtk::ResponseType::CANCEL);

        m_cities_button->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_unnamed_cities_button->set_sensitive
               (!m_cities_button->get_active ());
             if (m_cities_button->get_active ())
               m_unnamed_cities_button->set_active (false);
             update ();
           });

        m_ruins_button->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_unnamed_ruins_button->set_sensitive
               (!m_ruins_button->get_active ());
             if (m_ruins_button->get_active ())
               m_unnamed_ruins_button->set_active (false);
             update ();
           });

        m_temples_button->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_unnamed_temples_button->set_sensitive
               (!m_temples_button->get_active ());
             if (m_temples_button->get_active ())
               m_unnamed_temples_button->set_active (false);
             update ();
           });

        m_signs_button->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_unnamed_signs_button->set_sensitive
               (!m_signs_button->get_active ());
             if (m_signs_button->get_active ())
               m_unnamed_signs_button->set_active (false);
             update ();
           });

        m_unnamed_cities_button->property_active ().signal_changed ().connect
          ([this] ()
           {
             update ();
           });

        m_unnamed_ruins_button->property_active ().signal_changed ().connect
          ([this] ()
           {
             update ();
           });

        m_unnamed_temples_button->property_active ().signal_changed ().connect
          ([this] ()
           {
             update ();
           });

        m_unnamed_signs_button->property_active ().signal_changed ().connect
          ([this] ()
           {
             update ();
           });

        update ();
      }
private:
    Gtk::Button *m_randomize_button;
    Gtk::Button *m_cancel_button;
    Gtk::CheckButton *m_cities_button;
    Gtk::CheckButton *m_unnamed_cities_button;
    Gtk::CheckButton *m_ruins_button;
    Gtk::CheckButton *m_unnamed_ruins_button;
    Gtk::CheckButton *m_temples_button;
    Gtk::CheckButton *m_unnamed_temples_button;
    Gtk::CheckButton *m_signs_button;
    Gtk::CheckButton *m_unnamed_signs_button;
    Gtk::Label *m_status_label;

    CreateScenarioRandomize *m_randomize;

    void randomize_city (City *c)
      {
        Glib::ustring name = m_randomize->popRandomCityName ();
        if (name != "")
          c->setName (name);
        c->setRandomArmytypes (true, 1);
      }

    void randomize_ruin (Ruin *r)
      {
        Glib::ustring name = m_randomize->popRandomRuinName ();
        if (name != "")
          r->setName (name);
      }

    void randomize_temple (Temple *t)
      {
        Glib::ustring name = m_randomize->popRandomTempleName ();
        if (name != "")
          t->setName (name);
      }

    void randomize_signpost (Signpost *s)
      {
        Glib::ustring name = "";
        if (m_randomize->getNumSignposts () > 0 &&
            (Rnd::rand () % m_randomize->getNumSignposts ()) == 0)
          name = m_randomize->popRandomSignpost ();
        else
          name = m_randomize->getDynamicSignpost (s);
        if (name != "")
          s->setName (name);
      }

    void update ()
      {
        guint32 count = get_object_count ();
        m_status_label->set_text
          (String::ucompose (ngettext ("There is %1 object selected",
                                       "There are %1 objects selected",
                                       count),
                             count));
        m_randomize_button->set_sensitive (count > 0);
      }

    guint32 get_object_count ()
      {
        guint32 count = 0;
        if (m_cities_button->get_active ())
          count += Citylist::instance ()->size ();
        else if (m_unnamed_cities_button->get_active ())
          {
            for (auto c : *Citylist::instance ())
              {
                if (c->isUnnamed () == true)
                  count++;
              }
          }

        if (m_ruins_button->get_active ())
          count += Ruinlist::instance ()->size ();
        else if (m_unnamed_ruins_button->get_active ())
          {
            for (auto r : *Ruinlist::instance ())
              {
                if (r->isUnnamed ())
                  count++;
              }
          }

        if (m_temples_button->get_active ())
          count += Templelist::instance ()->size ();
        else if (m_unnamed_temples_button->get_active ())
          {
            for (auto t : *Templelist::instance ())
              {
                if (t->isUnnamed ())
                  count++;
              }
          }

        if (m_signs_button->get_active ())
          count += Signpostlist::instance ()->size ();
        else if (m_unnamed_signs_button->get_active ())
          {
            for (auto s : *Signpostlist::instance ())
              {
                if (s->getName() == DEFAULT_SIGNPOST)
                  count++;
              }
          }

        return count;
      }

};
#endif
