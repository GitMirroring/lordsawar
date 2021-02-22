//  Copyright (C) 2011, 2014, 2015, 2020 Ben Asselstine
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

#include <gtkmm.h>

#include "stack-tile-box.h"
#include "stacktile.h"
#include "stack.h"
#include "stack-army-button.h"
#include "army-info-tip.h"
#include "builder-cache.h"

#include "ucompose.hpp"
#include "defs.h"
#include "ImageCache.h"
#include "File.h"
#include "playerlist.h"
#include "player.h"
#include "GameMap.h"
#include "army.h"
#include "shield.h"
#include "tileset.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &StackTileBox::x)

StackTileBox * StackTileBox::create()
{
  Glib::ustring file = "stack-tile-box-large-screen.ui";
  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get(file);

  StackTileBox *box;
  xml->get_widget_derived("box", box);
  return box;
}

void StackTileBox::pad_image(Gtk::Image *image)
{
  int padding = 3;
  image->property_margin_start () = padding;
  image->property_margin_end () = padding;
  image->property_margin_top () = padding;
  image->property_margin_bottom () = padding;
}

StackTileBox::StackTileBox(BaseObjectType* baseObject, const Glib::RefPtr<Gtk::Builder> &xml)
  : Gtk::Box(baseObject)
{
  d_inhibit = false;
  army_info_tip = NULL;
  xml->get_widget("stack_info_box", stack_info_box);
  xml->get_widget("stack_info_container", stack_info_container);
  xml->get_widget("group_moves_label", group_moves_label);
  xml->get_widget("group_togglebutton", group_ungroup_toggle);
  group_ungroup_toggle->signal_toggled().connect
    (sigc::bind(method(on_group_toggled), group_ungroup_toggle));
  xml->get_widget("terrain_image", terrain_image);

  //okay let's make our army buttons.
  for (unsigned int i = 0; i < MAX_ARMIES_ON_A_SINGLE_TILE; i++)
    {
      //we put them in a vbox so that the buttons don't expand horizontally.
      StackArmyButton *button = StackArmyButton::create();
      button->get_parent()->remove(*button);
      Gtk::Box *box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
      box->pack_start(*Gtk::manage(button), Gtk::PACK_SHRINK);
      button->reparent(*box);
      stack_army_buttons.push_back(button);
      stack_info_box->pack_start(*Gtk::manage(box), Gtk::PACK_SHRINK);
    }
  d_inhibit_group_toggle = false;
}

void StackTileBox::reset_army_buttons()
{
  for (unsigned int i = 0; i < stack_army_buttons.size(); i++)
    stack_army_buttons[i]->reset();
  for (unsigned int i = 0; i < MAX_ARMIES_ON_A_SINGLE_TILE; i++)
    {
      army_conn[i].disconnect();
      stack_conn[i].disconnect();
    }
}

StackTileBox::~StackTileBox()
{
  if (army_info_tip)
    delete army_info_tip;
}

void StackTileBox::on_stack_toggled(Stack *stack)
{
  if (d_inhibit || d_inhibit_group_toggle)
    return;
  if (stack == currently_selected_stack)
    return;
  currently_selected_stack = stack;
  Playerlist::getActiveplayer()->setActivestack(stack);
  on_stack_info_changed(stack);
  stack_composition_modified.emit(stack);
}

void StackTileBox::on_army_toggled(StackArmyButton *toggle, Stack *stack, Army *army)
{
  if (d_inhibit || d_inhibit_group_toggle)
    return;
  for (stack_army_buttons_type::iterator i = stack_army_buttons.begin(),
       end = stack_army_buttons.end(); i != end; ++i)
    (*i)->update_stack_button(toggle == *i);
  Player *p = Playerlist::getActiveplayer();
  Stack *s = p->getActivestack();
  group_ungroup_toggle->set_sensitive(false);
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
  on_stack_info_changed(s);
  group_ungroup_toggle->set_sensitive(true);
  stack_composition_modified.emit(s);
}

void StackTileBox::on_group_toggled(Gtk::ToggleButton *toggle)
{
  if (d_inhibit_group_toggle)
    return;
  if (toggle->get_sensitive() == false)
    return;
  bool active = toggle->get_active();
      
  StackTile *s = GameMap::getStacks(currently_selected_stack->getPos());
  stack_tile_group_toggle.emit(true);
  if (active)
    {
      s->group(Playerlist::getActiveplayer(), currently_selected_stack);
      currently_selected_stack->sortForViewing(true);
    }
  else
    s->ungroup(Playerlist::getActiveplayer());
  stack_tile_group_toggle.emit(false);

  on_stack_info_changed(currently_selected_stack);
  stack_composition_modified.emit(currently_selected_stack);
}

void StackTileBox::show_stack(StackTile *s)
{
  reset_army_buttons();
  Player *p = Playerlist::getActiveplayer();
  std::vector<Stack *> stks;
  stks = s->getFriendlyStacks(p);
  unsigned int count= 0;
	    
  guint32 color_id = 0;
  if (color_id == p->getId())
    color_id = Shield::get_next_shield(color_id);
  for (std::vector<Stack *>::iterator j = stks.begin(); j != stks.end(); ++j)
    {
      bool first = true;
      for (Stack::iterator i = (*j)->begin(); i != (*j)->end(); ++i)
        {
          Stack *stack = NULL;
          if (first == true)
            {
              first = false;
              stack = *j;
            }
          if (count >= MAX_ARMIES_ON_A_SINGLE_TILE)
            break;

          StackArmyButton *button = stack_army_buttons[count];
          button->draw(stack, *i, color_id, (*j) == currently_selected_stack);
          army_conn[count].disconnect();
          army_conn[count] = button->army_toggled.connect
            (sigc::bind(method(on_army_toggled), button, *j, *i));
          stack_conn[count].disconnect();
          stack_conn[count] = button->stack_clicked.connect 
            (sigc::bind(method(on_stack_toggled), *j));
          count++;
        }

      color_id = Shield::get_next_shield(color_id);
      if (color_id== p->getId())
        color_id = Shield::get_next_shield(color_id);
    }

  fill_in_group_info(s, currently_selected_stack);
}

void StackTileBox::toggle_group_ungroup()
{
  group_ungroup_toggle->set_active(!group_ungroup_toggle->get_active());
}

void StackTileBox::fill_in_group_info (StackTile *stile, Stack *s)
{
  guint32 bonus = s->calculateMoveBonus();
  ImageCache *gc = ImageCache::getInstance();
  terrain_image->property_pixbuf() =
    gc->getMoveBonusPic(GameMap::getTileset()->getId(), bonus,
                        FontSize::getInstance ()->get_height ())->to_pixbuf();
  group_moves_label->set_markup(String::ucompose("<b>%1</b>", s->getMoves()));
  group_ungroup_toggle->set_sensitive(false);
  d_inhibit_group_toggle = true;
  if (stile->getFriendlyStacks(s->getOwner()).size() != 1)
    group_ungroup_toggle->set_active(false);
  else
    group_ungroup_toggle->set_active(true);
  if (group_ungroup_toggle->get_active() == true)
    group_ungroup_toggle->set_label(_("UnGrp"));
  else
    group_ungroup_toggle->set_label(_("Grp"));
  group_ungroup_toggle->set_sensitive(true);
  d_inhibit_group_toggle = false;
}

void StackTileBox::on_stack_info_changed(Stack *s)
{
  set_selected_stack(s);

  if (s->getOwner()->getType() == Player::HUMAN)
    {
      StackTile *stile = GameMap::getStacks(s->getPos());
      stile->setDefending(s->getOwner(), false);
      stile->setParked(s->getOwner(), false);
      show_stack(stile);
    }
}
