//  Copyright (C) 2021 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "hero-strategy-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "hero-strategy.h"
#include "hero-strategy-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &HeroStrategyDialog::x)

HeroStrategyDialog::HeroStrategyDialog(Gtk::Window &parent, HeroStrategy *s)
 : LwEditorDialog(parent, "strategy-editor-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  setup_strategies (s);
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  Gtk::Box *type_box;
  xml->get_widget("type_box", type_box);
  strategy_type_combobox = manage (new Gtk::ComboBoxText ());
  strategy_type_combobox->append (d_none_strategy->getDescription ());
  strategy_type_combobox->append (d_random_strategy->getDescription ());
  strategy_type_combobox->append (d_simple_quester_strategy->getDescription ());
  type_box->pack_start(*strategy_type_combobox, Gtk::PACK_SHRINK);


  xml->get_widget("notebook", notebook);
  xml->get_widget("turns_spinbutton", turns_spinbutton);
  xml->get_widget("num_helpers_spinbutton", num_helpers_spinbutton);
  Gtk::Box *fallback_box;
  xml->get_widget("fallback_box", fallback_box);
  fallback_combobox = manage (new Gtk::ComboBoxText ());
  fallback_combobox->append (d_none_strategy->getDescription ());
  fallback_combobox->append (d_random_strategy->getDescription ());
  fallback_box->pack_start(*fallback_combobox, Gtk::PACK_SHRINK);

  connect_signals ();
  update ();
}

void HeroStrategyDialog::setup_strategies (HeroStrategy *s)
{
  if (s && s->getType () == HeroStrategy::NONE)
    d_none_strategy = new HeroStrategy_None (*dynamic_cast<HeroStrategy_None*>(s));
  else
    d_none_strategy = new HeroStrategy_None ();

  if (s && s->getType () == HeroStrategy::RANDOM)
    d_random_strategy = new HeroStrategy_Random (*dynamic_cast<HeroStrategy_Random*>(s));
  else
    d_random_strategy = new HeroStrategy_Random ();

  if (s && s->getType () == HeroStrategy::SIMPLE_QUESTER)
    d_simple_quester_strategy = new HeroStrategy_SimpleQuester (*dynamic_cast<HeroStrategy_SimpleQuester*>(s));
  else
    d_simple_quester_strategy = new HeroStrategy_SimpleQuester();

  if (!s)
    d_strategy = d_none_strategy;
  else
    switch_strategy_type (s->getType ());
}

HeroStrategyDialog::~HeroStrategyDialog ()
{
  delete d_none_strategy;
  delete d_random_strategy;
  delete d_simple_quester_strategy;
  delete umgr;
  notebook->property_show_tabs () = false;
}

void HeroStrategyDialog::fill_in_strategy_info()
{
  if (d_strategy->getType() == HeroStrategy::NONE)
    strategy_type_combobox->set_active (0);
  else if (d_strategy->getType() == HeroStrategy::RANDOM)
    {
      HeroStrategy_Random *s = static_cast<HeroStrategy_Random*>(d_strategy);
      turns_spinbutton->set_value (s->getTurns ());
      strategy_type_combobox->set_active (1);
    }
  else if (d_strategy->getType() == HeroStrategy::SIMPLE_QUESTER)
    {
      HeroStrategy_SimpleQuester *s = static_cast<HeroStrategy_SimpleQuester*>(d_strategy);
      fallback_combobox->set_active (int (s->getFallbackStrategy ()));
      num_helpers_spinbutton->set_value (s->getNumHelpers ());
      strategy_type_combobox->set_active (2);
    }
  notebook->property_page() = strategy_type_combobox->get_active_row_number();
}

bool HeroStrategyDialog::run()
{
  dialog->show_all();
  dialog->run();
  return d_changed;
}

bool HeroStrategyDialog::switch_strategy_type (HeroStrategy::Type t)
{
  if (t != d_strategy->getType ())
    {
      switch (t)
        {
        case HeroStrategy::NONE: d_strategy = d_none_strategy; break;
        case HeroStrategy::RANDOM: d_strategy = d_random_strategy; break;
        case HeroStrategy::SIMPLE_QUESTER: d_strategy = d_simple_quester_strategy; break;
        }
      return true;
    }
  return false;
}

void HeroStrategyDialog::on_type_changed()
{
  HeroStrategy::Type new_type =
    HeroStrategy::Type (strategy_type_combobox->get_active_row_number ());

  notebook->property_page() = strategy_type_combobox->get_active_row_number();
  HeroStrategyEditorAction_Type *action =
    new HeroStrategyEditorAction_Type (d_strategy);
  if (switch_strategy_type (new_type))
    {
      umgr->add (action);
      d_changed = true;
      update ();
    }
  else
    delete action;
}

void HeroStrategyDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void HeroStrategyDialog::on_redo_activated ()
{
  d_changed = true;
  umgr->redo ();
  update ();
}

void HeroStrategyDialog::update ()
{
  disconnect_signals ();
  fill_in_strategy_info ();
  connect_signals ();
}

void HeroStrategyDialog::on_turns_text_changed ()
{
  int value = atoi(turns_spinbutton->get_text().c_str());
  int newvalue = value;
  int max = 40;
  if (value >= max)
    newvalue = max - 1;
  else if (value < 1)
    newvalue = 1;

  HeroStrategy_Random *s = dynamic_cast<HeroStrategy_Random*>(d_strategy);
  if (s->getTurns () != (guint32) newvalue)
    {
      umgr->add (new HeroStrategyEditorAction_Turns (d_strategy));
      s->setTurns ((guint32) newvalue);
      d_changed = true;
    }
  update ();
}

void HeroStrategyDialog::on_num_helpers_text_changed ()
{
  int value = atoi(num_helpers_spinbutton->get_text().c_str());
  int newvalue = value;
  int max = 40;
  if (value >= max)
    newvalue = max - 1;
  else if (value < 1)
    newvalue = 1;

  HeroStrategy_SimpleQuester *s = dynamic_cast<HeroStrategy_SimpleQuester*>(d_strategy);
  if (s->getNumHelpers () != (guint32) newvalue)
    {
      umgr->add (new HeroStrategyEditorAction_NumHelpers (d_strategy));
      s->setNumHelpers ((guint32) newvalue);
      d_changed = true;
    }
  update ();
}

void HeroStrategyDialog::on_fallback_type_changed ()
{
  HeroStrategy::Type new_type =
    HeroStrategy::Type (fallback_combobox->get_active_row_number ());
  umgr->add (new HeroStrategyEditorAction_FallbackType (d_strategy));
  HeroStrategy_SimpleQuester *s = dynamic_cast<HeroStrategy_SimpleQuester*>(d_strategy);
  s->setFallbackStrategy (new_type);
  d_changed = true;
  update ();
}

void HeroStrategyDialog::connect_signals ()
{
  connections.push_back
    (strategy_type_combobox->signal_changed().connect (method(on_type_changed)));
  connections.push_back
    (turns_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_turns_text_changed)))));
  connections.push_back
    (num_helpers_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_num_helpers_text_changed)))));
  connections.push_back
    (fallback_combobox->signal_changed().connect (method(on_fallback_type_changed)));
}

void HeroStrategyDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction*
HeroStrategyDialog::executeAction (UndoAction *action2)
{
  HeroStrategyEditorAction *action = dynamic_cast<HeroStrategyEditorAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case HeroStrategyEditorAction::TYPE:
          {
            HeroStrategyEditorAction_Type *a =
              dynamic_cast<HeroStrategyEditorAction_Type*>(action);
            out = new HeroStrategyEditorAction_Type (d_strategy);
            switch_strategy_type (a->getStrategy ()->getType ());
          }
        break;
      case HeroStrategyEditorAction::TURNS:
          {
            HeroStrategyEditorAction_Turns *a =
              dynamic_cast<HeroStrategyEditorAction_Turns*>(action);
            out = new HeroStrategyEditorAction_Turns (d_strategy);
            dynamic_cast<HeroStrategy_Random*>(d_strategy)->setTurns
              (dynamic_cast<HeroStrategy_Random*>(a->getStrategy ())->getTurns ());
          }
        break;
      case HeroStrategyEditorAction::NUM_HELPERS:
          {
            HeroStrategyEditorAction_NumHelpers *a =
              dynamic_cast<HeroStrategyEditorAction_NumHelpers*>(action);
            out = new HeroStrategyEditorAction_NumHelpers (d_strategy);
            dynamic_cast<HeroStrategy_SimpleQuester*>(d_strategy)->setNumHelpers
              (dynamic_cast<HeroStrategy_SimpleQuester*>(a->getStrategy ())->getNumHelpers ());
          }
        break;
      case HeroStrategyEditorAction::FALLBACK_TYPE:
          {
            HeroStrategyEditorAction_FallbackType *a =
              dynamic_cast<HeroStrategyEditorAction_FallbackType*>(action);
            out = new HeroStrategyEditorAction_FallbackType (d_strategy);
            dynamic_cast<HeroStrategy_SimpleQuester*>(d_strategy)->setFallbackStrategy
              (dynamic_cast<HeroStrategy_SimpleQuester*>(a->getStrategy ())->getFallbackStrategy ());
          }
        break;
      }
    return out;
}
