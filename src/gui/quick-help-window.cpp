//  Copyright (C) 2020 Ben Asselstine
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
#include <map>
#include <sigc++/functors/mem_fun.h>

#include "defs.h"
#include "quick-help-window.h"

#include "builder-cache.h"

#define method(x) sigc::mem_fun(*this, &QuickHelpWindow::x)
QuickHelpWindow::QuickHelpWindow()
 : xml (BuilderCache::get("quick-help-window.ui"))
{
  xml->get_widget("window", window);
  window->set_title (_("Quick Help"));
  xml->get_widget("close_button", close_button);
  close_button->signal_clicked().connect (method(on_close_button_clicked));
  
  Gtk::Box *box;
  xml->get_widget("group_stack_box", box);
  add_shortcut (box);
  xml->get_widget("deselect_stack_box", box);
  add_shortcut (box);
  xml->get_widget("next_stack_box", box);
  add_shortcut (box);
  xml->get_widget("pick_up_box", box);
  add_shortcut (box);
  xml->get_widget("search_box", box);
  add_shortcut (box);
  xml->get_widget("move_all_stacks_box", box);
  add_shortcut (box);
  xml->get_widget("disband_stack_box", box);
  add_shortcut (box);
  xml->get_widget("fight_order_box", box);
  add_shortcut (box);
  xml->get_widget("stack_bonuses_box", box);
  add_shortcut (box);
  xml->get_widget("army_report_box", box);
  add_shortcut (box);
  xml->get_widget("city_report_box", box);
  add_shortcut (box);
  xml->get_widget("gold_report_box", box);
  add_shortcut (box);
  xml->get_widget("production_report_box", box);
  add_shortcut (box);
  xml->get_widget("winning_report_box", box);
  add_shortcut (box);
  xml->get_widget("diplomacy_report_box", box);
  add_shortcut (box);
  xml->get_widget("quest_report_box", box);
  add_shortcut (box);
  xml->get_widget("hero_report_box", box);
  add_shortcut (box);
  xml->get_widget("ruin_report_box", box);
  add_shortcut (box);
  xml->get_widget("hasten_battles_box", box);
  add_shortcut (box);
  xml->get_widget("load_game_box", box);
  add_shortcut (box);
  xml->get_widget("save_game_box", box);
  add_shortcut (box);
  xml->get_widget("game_lobby_box", box);
  add_shortcut (box);
  xml->get_widget("fullscreen_box", box);
  add_shortcut (box);
  xml->get_widget("preferences_box", box);
  add_shortcut (box);
  xml->get_widget("grid_box", box);
  add_shortcut (box);
  xml->get_widget("end_turn_box", box);
  add_shortcut (box);
  xml->get_widget("resign_box", box);
  add_shortcut (box);
  xml->get_widget("army_abilities_report_box", box);
  add_shortcut (box);
  xml->get_widget("item_abilities_report_box", box);
  add_shortcut (box);
  xml->get_widget("city_history_report_box", box);
  add_shortcut (box);
  xml->get_widget("event_history_report_box", box);
  add_shortcut (box);
  xml->get_widget("gold_history_report_box", box);
  add_shortcut (box);
  xml->get_widget("winning_history_report_box", box);
  add_shortcut (box);
  xml->get_widget("triumphs_report_box", box);
  add_shortcut (box);
  xml->get_widget("city_production_box", box);
  add_shortcut (box);
  xml->get_widget("vectoring_box", box);
  add_shortcut (box);
}

void QuickHelpWindow::add_shortcut (Gtk::Box *box)
{
  std::vector<Gtk::Widget*> widgets = box->get_children();
  Gtk::Label *title = dynamic_cast<Gtk::Label*>(widgets[0]);
  Gtk::Label *accel = dynamic_cast<Gtk::Label*>(widgets[1]);
  Gtk::ShortcutsShortcut *s = new Gtk::ShortcutsShortcut ();
  s->property_accelerator () = accel->get_label ();
  s->property_title () = title->get_label ();
  box->foreach(sigc::mem_fun(box, &Gtk::Container::remove));
  box->add (*s);
}

void QuickHelpWindow::on_close_button_clicked ()
{
  window->hide ();
}
