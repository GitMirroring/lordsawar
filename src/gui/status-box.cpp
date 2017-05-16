//  Copyright (C) 2011, 2014, 2015 Ben Asselstine
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

#include "status-box.h"
#include "builder-cache.h"
#include "ucompose.hpp"

#include "stacktile.h"
#include "stack.h"
#include "defs.h"
#include "ImageCache.h"
#include "File.h"
#include "playerlist.h"
#include "player.h"
#include "armysetlist.h"
#include "GameMap.h"

Glib::ustring StatusBox::get_file(Configuration::UiFormFactor factor)
{
  Glib::ustring file = "";
  switch (factor)
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
      file = "status-box-desktop.ui";
      break;
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      file = "status-box-netbook.ui";
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      file = "status-box-large-screen.ui";
      break;
    }
  return file;
}

StatusBox * StatusBox::create(guint32 factor)
{
  Glib::ustring file = get_file(Configuration::UiFormFactor(factor));
  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get(file);

  StatusBox *box;
  xml->get_widget_derived("box", box);
  box->d_factor = factor;
  return box;
}

void StatusBox::pad_image(Gtk::Image *image)
{
  int padding = 0;
  switch (Configuration::UiFormFactor(d_factor))
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
      padding = 0;
      break;
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      padding = 0;
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      padding = 3;
      break;
    }
  image->property_xpad() = padding;
  image->property_ypad() = padding;
}

StatusBox::StatusBox(BaseObjectType* baseObject, const Glib::RefPtr<Gtk::Builder> &xml)
  : Gtk::Box(baseObject)
{
  d_height_fudge_factor = 0;
  xml->get_widget("info_notebook", notebook);
  xml->get_widget("stats_box", stats_box);
  xml->get_widget("tartan_box", tartan_box);
  xml->get_widget("stack_info_container", stack_info_container);
  xml->get_widget("progress_status_label", progress_status_label);
  xml->get_widget("cities_stats_image", cities_stats_image);
  cities_stats_image->property_pixbuf() = 
    Gdk::Pixbuf::create_from_file(File::getVariousFile("smallcity.png"));

  xml->get_widget("gold_stats_image", gold_stats_image);
  gold_stats_image->property_pixbuf() = 
    Gdk::Pixbuf::create_from_file(File::getVariousFile("smalltreasury.png"));
  xml->get_widget("income_stats_image", income_stats_image);
  income_stats_image->property_pixbuf() = 
    Gdk::Pixbuf::create_from_file(File::getVariousFile("smallincome.png"));
  xml->get_widget("upkeep_stats_image", upkeep_stats_image);
  upkeep_stats_image->property_pixbuf() =
    Gdk::Pixbuf::create_from_file(File::getVariousFile("smallupkeep.png"));

  xml->get_widget("cities_stats_label", cities_stats_label);
  xml->get_widget("gold_stats_label", gold_stats_label);
  xml->get_widget("income_stats_label", income_stats_label);
  xml->get_widget("upkeep_stats_label", upkeep_stats_label);
  xml->get_widget("stack_tile_box_container", stack_tile_box_container);
  stack_tile_box = Gtk::manage(StackTileBox::create(Configuration::s_ui_form_factor));
  stack_tile_box->reparent(*stack_tile_box_container);
  stack_tile_box->stack_composition_modified.connect
    (sigc::mem_fun(stack_composition_modified, 
                   &sigc::signal<void, Stack*>::emit));
  stack_tile_box->stack_tile_group_toggle.connect
    (sigc::mem_fun(stack_tile_group_toggle, &sigc::signal<void, bool>::emit));
  turn_progressbar = NULL;
}

StatusBox::~StatusBox()
{
  if (turn_progressbar)
    delete turn_progressbar;
}

void StatusBox::on_stack_info_changed(Stack *s)
{
  stack_tile_box->set_selected_stack(s);

  if (!s)
    {
      if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
	show_stats();
      else
	show_progress();
    }
  else
    {
      if (s->getOwner()->getType() == Player::HUMAN)
	{
	  StackTile *stile = GameMap::getStacks(s->getPos());
	  stile->setDefending(s->getOwner(), false);
	  stile->setParked(s->getOwner(), false);
	  show_stack(stile);
	}
      else
	show_progress();
    }
  return;
}

void StatusBox::show_stats()
{
  notebook->set_current_page(1);
}

void StatusBox::show_progress()
{
  notebook->set_current_page(2);
  if (!turn_progressbar)
    {
      turn_progressbar = new TartanProgressBar (Playerlist::getActiveplayer());
      tartan_box->add(*manage(turn_progressbar));
      turn_progressbar->set_hexpand(true);
      tartan_box->show_all();
    }
  else
    turn_progressbar->pulse(Playerlist::getActiveplayer());
  if (Playerlist::getActiveplayer() == Playerlist::getInstance()->getNeutral())
    progress_status_label->set_text("");
  else
    progress_status_label->set_markup("<b>" + Playerlist::getActiveplayer()->getName() + "</b>");
}

void StatusBox::show_stack(StackTile *s)
{
  stack_info_container->show_all();
  stack_tile_box->show_stack(s);
  notebook->set_current_page(0);
}

void StatusBox::update_sidebar_stats(SidebarStats s)
{
  cities_stats_label->set_markup(String::ucompose("<b>%1</b>", s.cities));
  gold_stats_label->set_markup(String::ucompose("<b>%1</b>", s.gold));
  income_stats_label->set_markup(String::ucompose("<b>%1</b>", s.income));
  upkeep_stats_label->set_markup(String::ucompose("<b>%1</b>", s.upkeep));

  Glib::ustring tip;
  tip = String::ucompose(ngettext("You have %1 city!",
                                  "You have %1 cities!", s.cities), s.cities);
  cities_stats_image->set_tooltip_text(tip);
  cities_stats_label->set_tooltip_text(tip);
  tip = String::ucompose(ngettext("You have %1 gold piece in your treasury!",
				  "You have %1 gold pieces in your treasury!", 
                                  s.gold), s.gold);
  gold_stats_image->set_tooltip_text(tip);
  gold_stats_label->set_tooltip_text(tip);
  tip = String::ucompose(ngettext("You earn %1 gold piece in income!",
				  "You earn %1 gold pieces in income!", s.income), 
                         s.income);
  income_stats_image->set_tooltip_text(tip);
  income_stats_label->set_tooltip_text(tip);
  tip = String::ucompose(ngettext("You pay %1 gold piece in upkeep!",
				  "You pay %1 gold pieces in upkeep!", s.upkeep), 
                         s.upkeep);
  upkeep_stats_image->set_tooltip_text(tip);
  upkeep_stats_label->set_tooltip_text(tip);
}
    
void StatusBox::set_progress_label(Glib::ustring s)
{
  progress_status_label->set_markup("<b>" + s + "</b>");
}

void StatusBox::pulse()
{
  //warning: pulsing too quickly and can cause crashing bugs.
  turn_progressbar->pulse(Playerlist::getActiveplayer());
}
  
void StatusBox::toggle_group_ungroup()
{
  stack_tile_box->toggle_group_ungroup();
}

void StatusBox::enforce_height()
{
  int height = 
    Armysetlist::getInstance()->getTileSize(Playerlist::getActiveplayer
                                            ()->getArmyset());
  height += d_height_fudge_factor;
  height += 30; //button border pixels + radio button height.

  if (d_factor == Configuration::UI_FORM_FACTOR_LARGE_SCREEN)
    height += 50;
  else if (d_factor == Configuration::UI_FORM_FACTOR_NETBOOK)
    height -= 5;

  stats_box->get_parent()->property_height_request() = height;
}

void StatusBox::reset_progress()
{
  if (turn_progressbar)
    delete turn_progressbar;
  turn_progressbar = new TartanProgressBar (Playerlist::getActiveplayer());
  tartan_box->add(*manage(turn_progressbar));
  turn_progressbar->set_hexpand(true);
  tartan_box->show_all();
  turn_progressbar->clear();
}
