//  Copyright (C) 2008, 2009, 2011, 2014, 2020 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <sigc++/functors/mem_fun.h>

#include "stack-info-dialog.h"

#include <gtkmm.h>
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "player.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "stack.h"
#include "ImageCache.h"
#include "Tile.h"
#include <assert.h>
#include "stacktile.h"
#include "GameMap.h"
#include "tileset.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &StackInfoDialog::x)

StackInfoDialog::StackInfoDialog(Gtk::Window &parent, Vector<int> pos)
 : LwDialog(parent, "stack-info-dialog.ui"), tile (pos)
{
  army_info_tip = NULL;
  currently_selected_stack = NULL;

  xml->get_widget("stack_table", stack_table);
  xml->get_widget("group_button", group_button);
  group_button->signal_clicked().connect (method(on_group_clicked));
  xml->get_widget("ungroup_button", ungroup_button);
  ungroup_button->signal_clicked().connect (method(on_ungroup_clicked));
  fill_stack_info();
}

StackInfoDialog::~StackInfoDialog()
{
  if (army_info_tip != NULL)
    delete army_info_tip;
}

void StackInfoDialog::addStack(Stack *s, guint32 &idx)
{
  s->sortForViewing(true);
  Stack *target = new Stack(Playerlist::getInstance()->getNeutral(), s->getPos());
  ArmyProto *baseproto = ArmyProto::createScout();
  Army *army = new Army(*baseproto);
  delete baseproto;
  target->add(army);
  Fight fight(s, target, Fight::FOR_KICKS);
  delete target;

  bool first = true;
  guint32 color_id = 0;
  if (color_id == s->getOwner()->getId())
    color_id = Shield::get_next_shield(color_id);
  for (Stack::iterator it = s->begin(); it != s->end(); ++it)
    {
      guint32 str = fight.getModifiedStrengthBonus(*it);
      addArmy(first, s, *it, str, idx, color_id);
      if (first == true)
	first = false;
      idx++;
      color_id = Shield::get_next_shield(color_id);
      if (color_id == s->getOwner()->getId())
        color_id = Shield::get_next_shield(color_id);
    }
}

void StackInfoDialog::addArmy (bool first, Stack *s, Army *h, guint32 modified_strength, guint32 idx, guint32 color_id)
{
  ImageCache *gc = ImageCache::getInstance();
  Player *player = h->getOwner();
    
  bool greyed_out = s->getId() != currently_selected_stack->getId();
  Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);
  Glib::RefPtr<Gdk::Pixbuf> pixbuf= 
    gc->getCircledArmyPic(player->getArmyset(), h->getTypeId(), player, NULL,
                          greyed_out, !greyed_out ? player->getId() : color_id,
                          true,
                          FontSize::getInstance()->get_height ())->to_pixbuf();
  
  Gtk::Image *image = NULL;
  guint32 move_bonus = h->getStat(Army::MOVE_BONUS);
  bool ship = h->getStat(Army::SHIP);
  if (ship || move_bonus == (Tile::GRASS | Tile::WATER | Tile::FOREST | 
			     Tile::HILLS | Tile::SWAMP | Tile::MOUNTAIN))
    {
      image = new Gtk::Image();
      image->property_pixbuf() =
        gc->getMoveBonusPic(GameMap::getTileset()->getId(), move_bonus,
                            FontSize::getInstance ()->get_height ())->to_pixbuf();
    }

  armies.push_back(h);
  toggle->set_active(s->getId() == currently_selected_stack->getId());
  toggle->signal_toggled().connect
    (sigc::bind(method(on_army_toggled), toggle, s, h));
  toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  toggles.push_back(toggle);
  toggle->signal_button_press_event().connect
    (sigc::bind(method(on_army_button_event), toggle), false);
  toggle->signal_button_release_event().connect
    (sigc::bind(method(on_army_button_event), toggle), false);
  Gtk::Image *army_image = new Gtk::Image();
  army_image->property_pixbuf() = pixbuf;
  toggle->add(*manage(army_image));

  Gtk::Label *name = new Gtk::Label(h->getName());
  unsigned int strength_value = h->getStat(Army::STRENGTH);
  Glib::ustring str = String::ucompose("%1", strength_value);
  if (modified_strength != strength_value)
    str += String::ucompose(" (%1)", modified_strength);
      
  Gtk::Label *strength = new Gtk::Label(str);
  Gtk::Label *bonus = new Gtk::Label(h->getArmyBonusDescription());
  Gtk::Label *moves = new Gtk::Label(String::ucompose("%1", h->getMoves()));
	
  if (first)
    {
      Gtk::RadioButton *radio;
      if (radios.size() > 0)
	{
	  Gtk::RadioButtonGroup b;
	  b = radios.front()->get_group();
	  radio = new Gtk::RadioButton(b);
	}
      else
	radio = new Gtk::RadioButton();
      radios.push_back(radio);
      radio->set_active(s->getId() == currently_selected_stack->getId());
      radio->signal_toggled().connect
	(sigc::bind(method(on_stack_toggled), radio, s));
      stack_table->attach(*manage(radio), 0, idx, 1, 1);
    }

  stack_table->attach(*manage(toggle), 1, idx, 1, 1);
  stack_table->attach(*manage(name), 2, idx, 1, 1);
  stack_table->attach(*manage(strength), 3, idx, 1, 1);
  stack_table->attach(*manage(moves), 4, idx, 1, 1);
  if (image)
    stack_table->attach(*manage(image), 5, idx, 1, 1);
  stack_table->attach(*manage(bonus), 6, idx, 1, 1);
}
       
void StackInfoDialog::on_group_clicked()
{
  StackTile *stile = GameMap::getStacks(tile);
  Stack *stack = stile->group(Playerlist::getActiveplayer());
  currently_selected_stack = stack;
  stack->sortForViewing(true);
  fill_stack_info();
}

void StackInfoDialog::on_ungroup_clicked()
{
  StackTile *stile = GameMap::getStacks(tile);
  stile->ungroup(Playerlist::getActiveplayer());
  fill_stack_info();
}

void StackInfoDialog::fill_stack_info()
{
  StackTile *stile = GameMap::getStacks(tile);
  guint32 idx = 1;

  armies.clear();
  toggles.clear();
  radios.clear();
  stack_table->foreach(sigc::mem_fun(stack_table, &Gtk::Container::remove));
  for (int i = 0; i < 6; i++)
    stack_table->insert_row (i);
  for (unsigned int i = 0; i < MAX_ARMIES_ON_A_SINGLE_TILE; i++)
    stack_table->insert_column (i);

  Pango::AttrList attrs;
  Pango::Attribute weight = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
  attrs.insert(weight);
  Gtk::Label *str = new Gtk::Label(_("Str"));
  str->set_attributes(attrs);
  stack_table->attach(*manage(str), 3, 0, 1, 1);

  Gtk::Label *moves = new Gtk::Label(_("Move"));
  moves->set_attributes(attrs);
  stack_table->attach(*manage(moves), 4, 0, 1, 1);

  Gtk::Label *bonus = new Gtk::Label(_("Bonus"));
  bonus->set_attributes(attrs);
  stack_table->attach(*manage(bonus), 6, 0, 1, 1);

  std::vector<Stack*> stks;
  stks = stile->getFriendlyStacks(Playerlist::getActiveplayer());
  if (currently_selected_stack == NULL)
    currently_selected_stack = stks.front();
  for (std::vector<Stack *>::iterator i = stks.begin(); i != stks.end(); ++i)
    addStack(*i, idx);
  stack_table->show_all();
}

bool StackInfoDialog::on_army_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle)
{
  MouseButtonEvent event = to_input_event(e);
  if (event.button == MouseButtonEvent::RIGHT_BUTTON && 
      event.state == MouseButtonEvent::PRESSED) 
    {
      int slot = -1;
      for (unsigned int i = 0; i < toggles.size(); ++i) 
	{
	  if (toggle == toggles[i])
	    slot = i;
	}
      assert(slot != -1);

      const Army *army = armies[slot];

      if (army)
	{
	  if (army_info_tip != NULL)
	    delete army_info_tip;
	  army_info_tip = new ArmyInfoTip(toggle, army);
	}
      return true;
    }
  else if (event.button == MouseButtonEvent::RIGHT_BUTTON && 
           event.state == MouseButtonEvent::RELEASED) 
    {
      if (army_info_tip != NULL)
	{
      	  delete army_info_tip;
	  army_info_tip = NULL;
	}
      return true;
    }

  return false;
}

void StackInfoDialog::on_army_toggled(Gtk::ToggleButton *toggle, Stack *stack, Army *army)
{
  Player *p = Playerlist::getActiveplayer();
  group_button->set_sensitive(false);
  ungroup_button->set_sensitive(false);
  if (toggle->get_active() == true)
    {
      if (stack->size() > 1)
	{
	  Stack *new_stack = p->stackSplitArmy(stack, army);
	  if (new_stack)
	    p->stackJoin(currently_selected_stack, new_stack);
	}
      else
	p->stackJoin(currently_selected_stack, stack);
      currently_selected_stack->sortForViewing(true);
    }
  else
    {
      p->stackSplitArmy(stack, army);
      stack->sortForViewing(true);
    }
  group_button->set_sensitive(true);
  ungroup_button->set_sensitive(true);
  fill_stack_info();
}

void StackInfoDialog::on_stack_toggled(Gtk::RadioButton *radio, Stack *stack)
{
  if (radio->get_active() == true)
    {
      if (stack == currently_selected_stack)
	return;
      currently_selected_stack = stack;
      fill_stack_info();
    }
}

