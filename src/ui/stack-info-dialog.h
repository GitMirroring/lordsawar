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
#ifndef STACK_INFO_DIALOG_H
#define STACK_INFO_DIALOG_H

#include "army.h"
#include "stack-tile.h"
#include "image-cache.h"
#include "army-info-tip.h"
#include "stack.h"

class StackInfoRow: public Glib::Object
{
public:
    Army *m_army;

    static Glib::RefPtr<StackInfoRow> create (Army *a)
      {
        return
          Glib::make_refptr_for_instance<StackInfoRow>
          (new StackInfoRow (a));
      }

protected:
    StackInfoRow (Army *a)
      :m_army (a)
      {
      }
};

class StackInfoDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "stack-info.ui";
      }

    StackInfoDialog (BaseObjectType* o,
                     const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_group_button = load <Gtk::Button> ("group_button");
        m_ungroup_button = load <Gtk::Button> ("ungroup_button");
        m_stack_table = load <Gtk::Grid> ("stack_table");
      }

    ~StackInfoDialog ()
      {
        if (m_army_info_tip)
          delete m_army_info_tip;
      }

    void setup (Vector<int> pos)
      {
        m_tile = pos;
        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        m_army_info_tip = new ArmyInfoTip ();

        m_group_button->signal_clicked ().connect
          ([this] ()
           {
             StackTile *stile = GameMap::getStacks (m_tile);
             Stack *stack = stile->group (Playerlist::getActiveplayer ());
             m_currently_selected_stack = stack;
             stack->sortForViewing (true);
             fill_stacks ();
           });

        m_ungroup_button->signal_clicked ().connect
          ([this] ()
           {
             StackTile *stile = GameMap::getStacks (m_tile);
             stile->ungroup (Playerlist::getActiveplayer ());
             fill_stacks ();
           });

        fill_stacks ();
      }

private:
    Gtk::Button *m_close_button = NULL;
    Gtk::Button *m_group_button = NULL;
    Gtk::Button *m_ungroup_button = NULL;
    Gtk::Grid *m_stack_table = NULL;
    ArmyInfoTip* m_army_info_tip = NULL;
    std::vector<Gtk::ToggleButton *> m_toggles;
    std::vector<const Army*> m_armies;
    std::vector<Gtk::ToggleButton *> m_radios;
    Vector<int> m_tile;
    Stack *m_currently_selected_stack = NULL;

    void fill_stacks ()
      {
        StackTile *stile = GameMap::getStacks (m_tile);
        guint32 idx = 1;

        m_armies.clear ();
        m_toggles.clear ();
        m_radios.clear ();
        while (auto* child = m_stack_table->get_first_child ())
          m_stack_table->remove (*child);

        for (int i = 0; i < 6; i++)
          m_stack_table->insert_row (i);
        for (unsigned int i = 0; i < MAX_ARMIES_ON_A_SINGLE_TILE; i++)
          m_stack_table->insert_column (i);

        Gtk::Label *str = Gtk::make_managed<Gtk::Label>(_("Str"));
        str->add_css_class ("att-title");
        m_stack_table->attach (*str, 3, 0, 1, 1);

        Gtk::Label *moves = Gtk::make_managed<Gtk::Label>(_("Move"));
        moves->add_css_class ("att-title");
        m_stack_table->attach (*moves, 4, 0, 1, 1);

        Gtk::Label *bonus = Gtk::make_managed<Gtk::Label>(_("Bonus"));
        bonus->add_css_class ("att-title");
        m_stack_table->attach (*bonus, 6, 0, 1, 1);

        std::vector<Stack*> stks;
        stks = stile->getFriendlyStacks (Playerlist::getActiveplayer ());
  
        if (m_currently_selected_stack == NULL)
          m_currently_selected_stack = stks.front ();
        for (auto i = stks.begin (); i != stks.end (); ++i)
          fill_stack (*i, idx);
      }

    void fill_stack (Stack *s, guint32 &idx)
      {
        s->sortForViewing (true);

        //we get the modified srength bonus by doing a fight
        Stack *target = new Stack (Playerlist::getNeutral (), s->getPos ());
        ArmyProto *baseproto = ArmyProto::createScout ();
        Army *army = new Army (*baseproto);
        delete baseproto;
        target->add (army);
        Fight fight (s, target, Fight::FOR_KICKS);
        delete target;

        bool first = true;
        guint32 color_id = 0;
        if (color_id == s->getOwner ()->getId ())
          color_id = Shield::get_next_shield (color_id);
        for (Stack::iterator it = s->begin (); it != s->end (); ++it)
          {
            guint32 str = fight.getModifiedStrengthBonus (*it);
            fill_army (first, s, *it, str, idx, color_id);
            if (first == true)
              first = false;
            idx++;
            color_id = Shield::get_next_shield (color_id);
            if (color_id == s->getOwner ()->getId ())
              color_id = Shield::get_next_shield (color_id);
          }
      }

    void fill_army (bool first, Stack *s, Army *a, guint32 modified_strength,
                    guint32 idx, guint32 color_id)
      {
        Player *p = a->getOwner ();

        bool greyed_out = s->getId () != m_currently_selected_stack->getId ();
        Gtk::ToggleButton *toggle = Gtk::make_managed<Gtk::ToggleButton>();
        auto im =
          ImageCache::instance ()->getCircledArmyPic
          (p->getArmyset (), a->getTypeId (), p->get_shield (), NULL,
           greyed_out, !greyed_out ? p->getId () : color_id, true,
           Lw::get_dark ());

        Gtk::Image *image = NULL;
        guint32 move_bonus = a->getStat (Army::MOVE_BONUS);
        bool ship = a->getStat (Army::SHIP);
        if (ship || move_bonus == (Tile::GRASS | Tile::WATER | Tile::FOREST | 
                                   Tile::HILLS | Tile::SWAMP | Tile::MOUNTAIN))
          {
            image = Gtk::make_managed<Gtk::Image>();
            image->set_pixel_size (LW_BUTTON_SIZE);
            image->set
              (ImageCache::instance ()->getMoveBonusPic
               (GameMap::getTileset ()->getId (), move_bonus)->to_pixbuf ());
          }

        m_armies.push_back (a);
        toggle->set_active (s->getId () == m_currently_selected_stack->getId ());
        toggle->signal_toggled ().connect
          ([this, toggle, s, a, p]()
           {
             m_group_button->set_sensitive (false);
             m_ungroup_button->set_sensitive (false);
             if (toggle->get_active () == true)
               {
                 if (s->size () > 1)
                   {
                     Stack *new_stack = p->stackSplitArmy (s, a);
                     if (new_stack)
                       p->stackJoin (m_currently_selected_stack, new_stack);
                   }
                 else
                   p->stackJoin (m_currently_selected_stack, s);
                 m_currently_selected_stack->sortForViewing (true);
               }
             else
               {
                 p->stackSplitArmy (s, a);
                 s->sortForViewing (true);
               }
             m_group_button->set_sensitive (true);
             m_ungroup_button->set_sensitive (true);
             fill_stacks ();
           });
        Gtk::Image *army_image = Gtk::make_managed<Gtk::Image> ();
        army_image->set_pixel_size (LW_BUTTON_SIZE);
        army_image->set (im->to_pixbuf ());
        toggle->set_child (*army_image);

        auto click = Gtk::GestureClick::create ();
        click->set_button (3);
        click->signal_pressed ().connect
          ([this, toggle, a](int, double x, double y)
           {
             m_army_info_tip->show
               (toggle, x, y,
                [this, a] ()
                {
                  m_army_info_tip->set (a);
                });
           });
        toggle->add_controller (click);

        auto release = Gtk::GestureClick::create ();
        release->set_button (3);
        release->signal_released ().connect
          ([this](int, double, double)
           {
             m_army_info_tip->popdown ();
           });
        toggle->add_controller (release);

        m_toggles.push_back (toggle);

        Gtk::Label *name = Gtk::make_managed<Gtk::Label> (a->getName ());
        unsigned int strength_value = a->getStat (Army::STRENGTH);
        Glib::ustring str = String::ucompose ("%1", strength_value);
        if (modified_strength != strength_value)
          str += String::ucompose (" (%1)", modified_strength);

        Gtk::Label *strength = Gtk::make_managed<Gtk::Label> (str);
        Gtk::Label *bonus =
          Gtk::make_managed<Gtk::Label>(a->getArmyBonusDescription ());
        Gtk::Label *moves =
          Gtk::make_managed<Gtk::Label>(String::ucompose ("%1",
                                                          a->getMoves ()));

        if (first)
          {
            Gtk::ToggleButton *radio = Gtk::make_managed<Gtk::ToggleButton>();
            Gtk::Image *radio_image = Gtk::make_managed<Gtk::Image>();
            radio->set_child (*radio_image);
            radio->set_vexpand (false);
            radio->set_valign (Gtk::Align::CENTER);
            if (m_radios.empty () == false)
              radio->set_group (*m_radios.front ());
            m_radios.push_back (radio);
            radio->set_active
              (s->getId () == m_currently_selected_stack->getId ());
            std::string file;
            if (radio->get_active ())
              file = File::getVariousFile ("army-unit-selected.svg");
            else
              file = File::getVariousFile ("army-unit-unselected.svg");
            radio_image->set (file);
            radio->signal_toggled ().connect
              ([this, radio, s] ()
               {
                 if (radio->get_active () == true)
                   {
                     if (s == m_currently_selected_stack)
                       return;
                     m_currently_selected_stack = s;
                     fill_stacks ();
                   }
               });
            m_stack_table->attach (*radio, 0, idx, 1, 1);
          }

        m_stack_table->attach (*toggle, 1, idx, 1, 1);
        m_stack_table->attach (*name, 2, idx, 1, 1);
        m_stack_table->attach (*strength, 3, idx, 1, 1);
        m_stack_table->attach (*moves, 4, idx, 1, 1);
        if (image)
          m_stack_table->attach (*image, 5, idx, 1, 1);
        m_stack_table->attach (*bonus, 6, idx, 1, 1);
      }
};
#endif
