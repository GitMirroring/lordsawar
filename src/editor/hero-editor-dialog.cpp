//  Copyright (C) 2009, 2014, 2020, 2021 Ben Asselstine
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

#include "hero-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "hero.h"
#include "backpack-editor-dialog.h"
#include "Backpack.h"
#include "hero-editor-actions.h"
#include "select-character-dialog.h"
#include "character.h"
#include "herotemplates.h"
#include "playerlist.h"

#define method(x) sigc::mem_fun(*this, &HeroEditorDialog::x)

HeroEditorDialog::HeroEditorDialog(Gtk::Window &parent, Hero *hero)
 : LwEditorDialog(parent, "hero-editor-dialog.ui")
{
  d_changed = false;
  d_hero = hero;
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));

  xml->get_widget("edit_backpack_button", edit_backpack_button);
  edit_backpack_button->signal_clicked().connect (method(on_edit_backpack_clicked));
  xml->get_widget("edit_character_button", edit_character_button);
  edit_character_button->signal_clicked().connect (method(on_edit_character_clicked));
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
  xml->get_widget("gender_combobox", gender_combobox);
  xml->get_widget("name_entry", name_entry);
  umgr->addCursor (name_entry);
  connect_signals ();
  update ();
}

HeroEditorDialog::~HeroEditorDialog()
{
  delete umgr;
}

bool HeroEditorDialog::run()
{
  dialog->show_all();
  dialog->run ();
  return d_changed;
}

void HeroEditorDialog::on_edit_backpack_clicked()
{
  HeroEditorAction_Backpack *action =
    new HeroEditorAction_Backpack (d_hero->getBackpack ());
  BackpackEditorDialog d(*dialog, d_hero->getBackpack());
  if (d.run())
    {
      umgr->add (action);
      d_changed = true;
    }
  else
    delete action;
  update_buttons ();
  return;
}

void HeroEditorDialog::on_edit_character_clicked()
{
  HeroEditorAction_Character *action =
    new HeroEditorAction_Character (d_hero->getHeroTypeId());
  SelectCharacterDialog d(*dialog, d_hero->getOwner (),
                          d_hero->getHeroTypeId ());
  if (d.run())
    {
      if (d.get_selected_id () >= 0)
        d_hero->setHeroTypeId (d.get_selected_id ());
      umgr->add (action);
      d_changed = true;
    }
  else
    delete action;
  update_buttons ();
  return;
}

void HeroEditorDialog::on_name_changed ()
{
  umgr->add (new HeroEditorAction_Name (d_hero->getName (), umgr, name_entry));
  d_hero->setName (String::utrim (name_entry->get_text ()));
  d_changed = true;
}

void HeroEditorDialog::on_gender_changed ()
{
  umgr->add (new HeroEditorAction_Gender (Hero::Gender (d_hero->getGender ())));
  d_hero->setGender(Hero::Gender(gender_combobox->get_active_row_number()+1));
  d_changed = true;
}

void HeroEditorDialog::update_buttons ()
{
  edit_backpack_button->set_label
    (String::ucompose (ngettext ("Carrying %1 item",
                                 "Carrying %1 items",
                                 d_hero->getBackpack()->size ()),
                       d_hero->getBackpack()->size ()));
  if (d_hero->getOwner () == Playerlist::getInstance ()->getNeutral ())
    {
      edit_character_button->set_label (_("No character set"));
      edit_character_button->set_sensitive (false);
    }
  else
    {
      Character *c =
        HeroTemplates::getInstance ()->getCharacterById (d_hero->getHeroTypeId ());
      if (c)
        edit_character_button->set_label (c->name);
      else
        edit_character_button->set_label (_("No character set"));
    }
}

void HeroEditorDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
}

void HeroEditorDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void HeroEditorDialog::update ()
{
  disconnect_signals ();
  name_entry->set_text(d_hero->getName());
  gender_combobox->set_active(d_hero->getGender()-1);
  update_buttons ();
  umgr->setCursors ();
  connect_signals ();
}

void HeroEditorDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (gender_combobox->signal_changed ().connect (method (on_gender_changed)));
  connections.push_back
    (name_entry->signal_changed ().connect (method (on_name_changed)));
}

void HeroEditorDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *HeroEditorDialog::executeAction (UndoAction *action2)
{
  HeroEditorAction *action = dynamic_cast<HeroEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case HeroEditorAction::NAME:
          {
            HeroEditorAction_Name *a =
              dynamic_cast<HeroEditorAction_Name*>(action);
            out = new HeroEditorAction_Name (d_hero->getName (), umgr,
                                             name_entry);

            d_hero->setName (a->getName ());
          } 
        break;
      case HeroEditorAction::GENDER:
          {
            HeroEditorAction_Gender *a =
              dynamic_cast<HeroEditorAction_Gender*>(action);
            out = new HeroEditorAction_Gender
              (Hero::Gender (d_hero->getGender ()));

            d_hero->setGender (a->getGender ());
          }
        break;
      case HeroEditorAction::BACKPACK:
          {
            HeroEditorAction_Backpack *a =
              dynamic_cast<HeroEditorAction_Backpack*>(action);
            out = new HeroEditorAction_Backpack (d_hero->getBackpack ());

            d_hero->getBackpack ()->removeAllFromBackpack ();
            d_hero->getBackpack ()->add (a->getBackpack ());
          }
        break;
      case HeroEditorAction::CHARACTER:
          {
            HeroEditorAction_Character *a =
              dynamic_cast<HeroEditorAction_Character*>(action);
            out = new HeroEditorAction_Character (d_hero->getHeroTypeId ());

            d_hero->setHeroTypeId (a->getHeroTypeId ());
          }
        break;
    }
  return out;
}
