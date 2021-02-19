//  Copyright (C) 2020, 2021 Ben Asselstine
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

#include "heroes-dialog.h"
#include "herotemplates.h"

#include <gtkmm.h>
#include "player.h"
#include "ucompose.hpp"
#include "heroproto.h"
#include "playerlist.h"
#include "heroes-editor-actions.h"
#include "hero-strategy-dialog.h"
#include "Backpack.h"
#include "Item.h"
#include "Itemlist.h"
#include "ItemProto.h"
#include "backpack-editor-dialog.h"
#include "timed-message-dialog.h"

#define method(x) sigc::mem_fun(*this, &HeroesDialog::x)

HeroesDialog::HeroesDialog(Gtk::Window &parent, guint32 player_id, Glib::ustring player_name)
 : LwEditorDialog(parent, "heroes-dialog.ui"),
    name_column(_("Name"), name_renderer)
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  d_warn_herotemplates_change_affects_heroes = false;
  d_player_id = player_id;
  xml->get_widget("treeview", treeview);
  xml->get_widget("panel_box", panel_box);

  int pw, ph;
  parent.get_size (pw, ph);
  dialog->set_size_request (ph * (16.0/9.0), ph);

  dialog->set_title (String::ucompose (_("Heroes of %1"), player_name));

  // setup the hero settings
  hero_list = Gtk::ListStore::create(hero_columns);
  treeview->set_model(hero_list);

  // name column
  name_renderer.property_editable() = false;
  name_column.set_cell_data_func(name_renderer, method(cell_data_name));
  treeview->append_column(name_column);

  xml->get_widget("name_entry", name_entry);
  umgr->addCursor (name_entry);

  xml->get_widget("description_entry", description_entry);
  umgr->addCursor (description_entry);

  Gtk::Box *box;
  xml->get_widget("gender_box", box);
  gender_combobox = manage(new Gtk::ComboBoxText);
  gender_combobox->append (Hero::genderToFriendlyName(Hero::MALE));
  gender_combobox->append (Hero::genderToFriendlyName (Hero::FEMALE));
  box->set_center_widget (*gender_combobox);

  fill_heroes ();

  xml->get_widget("add_button", add_button);
  add_button->signal_clicked().connect (method (on_add_pressed));
  xml->get_widget("remove_button", remove_button);
  remove_button->signal_clicked().connect (method (on_remove_pressed));
  xml->get_widget("strategy_button", strategy_button);
  strategy_button->signal_clicked().connect (method (on_strategy_pressed));
  xml->get_widget("items_button", items_button);
  items_button->signal_clicked ().connect (method (on_items_pressed));
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));
  
  if (hero_list->children().size () > 0)
    treeview->set_cursor (Gtk::TreePath ("0"));
  connect_signals ();
  update ();
}

bool HeroesDialog::run()
{
  dialog->show_all ();
  dialog->run();
  return d_changed;
}

void HeroesDialog::on_add_pressed ()
{
  umgr->add (new HeroesEditorAction_Add
             (HeroTemplates::getInstance ()->copy ()));
  Gtk::TreeIter i = hero_list->append();
  (*i)[hero_columns.name] = _("Unnamed Hero");
  std::list<guint32> ids;
  guint32 id = HeroTemplates::getInstance ()->getNextAvailableId ();
  Character *hero = new 
    Character (d_player_id, (*i)[hero_columns.name], "", id,
               Hero::FEMALE, ids, new HeroStrategy_None);
  (*i)[hero_columns.hero] = hero;
  update_hero_templates ();
  treeview->scroll_to_row (treeview->get_model ()->get_path (i));
  treeview->get_selection ()->select (i);
}

void HeroesDialog::on_strategy_pressed ()
{
  Character *hero = get_selected_hero ();
  if (hero)
    {
      HeroStrategyDialog d (*dialog, hero->strategy);
      if (d.run ())
        {
          d_changed = true;
          umgr->add (new HeroesEditorAction_Strategy (getCurIndex (),
                                                      hero->strategy));
          HeroStrategy *h = hero->strategy;
          delete h;
          hero->strategy = HeroStrategy::copy (d.get_strategy ());
          update_hero_templates ();
        }
    }
  update ();
}

void HeroesDialog::on_remove_pressed ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      umgr->add (new HeroesEditorAction_Remove
                 (HeroTemplates::getInstance ()->copy ()));
      if (d_warn_herotemplates_change_affects_heroes)
        {
          Player *p = Playerlist::getInstance ()->getPlayer (d_player_id);
          if (p->getHeroes ().empty () == false)
            {
              d_warn_herotemplates_change_affects_heroes = true;
              TimedMessageDialog
                d(*dialog, _("Removing hero types can break heros.\nWe'd like to automatically fix this for you but we can't!\nYou must manually fix them."), 0);
              d.run_and_hide ();
            }
        }
      Gtk::TreeModel::Row row = *iterrow;
      Character *h = row[hero_columns.hero];
      hero_list->erase(iterrow);
      delete h;
      update_hero_templates ();
    }
}

void HeroesDialog::fill_heroes ()
{
  clear_heroes ();
  std::vector<Character *> heroes =
    HeroTemplates::getInstance ()->getHeroes (d_player_id);
  for (auto h : heroes)
    {
      Gtk::TreeIter i = hero_list->append();
      (*i)[hero_columns.name] = h->name;
      (*i)[hero_columns.hero] = h;
    }
}

void HeroesDialog::cell_data_name(Gtk::CellRenderer *renderer,
                                  const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[hero_columns.name]);
}

void HeroesDialog::update_buttons ()
{
  remove_button->set_sensitive (get_selected_hero () != NULL);
}
  
Character* HeroesDialog::get_selected_hero ()
{
  Gtk::TreeIter i = treeview->get_selection()->get_selected();
  if (i)
    {
      Character *h = (*i)[hero_columns.hero];
      return h;
    }
  else
    return NULL;
}

void HeroesDialog::on_hero_selected ()
{
  update ();
}

void HeroesDialog::update_hero_templates ()
{
  d_changed = true;
  std::vector<Character*> heroes;
  for (auto i : hero_list->children ())
    {
      Character *hero = (*i)[hero_columns.hero];
      heroes.push_back (Character::copy (hero));
    }
  HeroTemplates::getInstance ()->replaceHeroes (d_player_id, heroes);
}

void HeroesDialog::clear_heroes ()
{
  for (auto i : hero_list->children ())
    {
      Character *hero = (*i)[hero_columns.hero];
      delete hero;
    }
  hero_list->clear ();
}

HeroesDialog::~HeroesDialog ()
{
  disconnect_signals ();
  clear_heroes ();
  delete umgr;
}

int HeroesDialog::getCurIndex ()
{
  int idx = -1;
  Gtk::TreeIter i = treeview->get_selection()->get_selected();
  if (i)
    {
      auto path = treeview->get_model ()->get_path (i);
      idx = atoi (path.to_string ().c_str ());
    }
  return idx;
}

void HeroesDialog::on_name_changed ()
{
  Character *hero = get_selected_hero ();
  if (hero)
    {
      umgr->add (new HeroesEditorAction_Name (getCurIndex (), hero->name,
                                              umgr, name_entry));
      Glib::RefPtr<Gtk::TreeSelection> selection = treeview->get_selection();
      Gtk::TreeModel::iterator iterrow = selection->get_selected();
      if (iterrow) 
        {
          Gtk::TreeModel::Row row = *iterrow;
          row[hero_columns.name] = String::utrim (name_entry->get_text());
        }
      hero->name = String::utrim (name_entry->get_text ());
      update_hero_templates ();
    }
}

void HeroesDialog::on_description_changed ()
{
  Character *hero = get_selected_hero ();
  if (hero)
    {
      umgr->add (new HeroesEditorAction_Desc (getCurIndex (),
                                              hero->description,
                                              umgr, description_entry));
      hero->description = String::utrim (description_entry->get_text ());
      update_hero_templates ();
    }
}


void HeroesDialog::on_gender_changed ()
{
  Character *hero = get_selected_hero ();
  if (hero)
    {
      umgr->add (new HeroesEditorAction_Gender
                 (getCurIndex (), Hero::Gender(hero->gender)));
      if (gender_combobox->get_active_row_number () == 0)
        hero->gender = Hero::MALE;
      else
        hero->gender = Hero::FEMALE;
      update_hero_templates ();
    }
  update ();
}

void HeroesDialog::on_items_pressed ()
{
  Character *hero = get_selected_hero ();
  if (hero)
    {
      HeroesEditorAction_Backpack *action =
        new HeroesEditorAction_Backpack (getCurIndex (), hero->item_ids);
      Backpack *backpack = new Backpack ();
      for (auto id : hero->item_ids)
        {
          if (id < Itemlist::getInstance ()->size ())
            {
              ItemProto *proto = (*Itemlist::getInstance ())[id];
              if (proto)
                backpack->addToBackpack (new Item (*proto, id));
            }
        }
      BackpackEditorDialog d(*dialog, backpack);
      if (d.run())
        {
          std::list<guint32> item_ids;
          for (auto item : *backpack)
            item_ids.push_back (item->getType());
          hero->item_ids = item_ids;
          umgr->add (action);
          d_changed = true;
        }
      else
        delete action;
      delete backpack;
      update_hero_templates ();
      update ();
    }
  return;
}

void HeroesDialog::update_panel ()
{
  Character *hero = get_selected_hero ();
  if (hero)
    {
      if (name_entry->get_text () != hero->name)
        name_entry->set_text (hero->name);
      switch (hero->gender)
        {
        case Hero::MALE:
          gender_combobox->set_active (0);
          break;
        default:
        case Hero::FEMALE:
          gender_combobox->set_active (1);
          break;
        }
      if (hero->strategy == NULL)
        strategy_button->set_label (_("No strategy set"));
      else
        strategy_button->set_label (hero->strategy->getDescription ());
      strategy_button->set_sensitive (false);

      if (hero->item_ids.size () == 0)
        items_button->set_label (_("No starting items"));
      else
        items_button->set_label
          (String::ucompose (ngettext ("Starting with %1 item",
                                       "Starting with %1 items",
                                       hero->item_ids.size ()),
                             hero->item_ids.size ()));
      if (description_entry->get_text () != hero->description)
        description_entry->set_text (hero->description);
    }
  else
    {
      name_entry->set_text ("");
      gender_combobox->set_active (0);
      strategy_button->set_label (_("No strategy set"));
      items_button->set_label (_("No starting items"));
      description_entry->set_text ("");
    }
  panel_box->set_sensitive (hero != NULL);
}

void HeroesDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (gender_combobox->signal_changed ().connect (method (on_gender_changed)));
  connections.push_back
    (name_entry->signal_changed ().connect (method (on_name_changed)));
  connections.push_back
    (description_entry->signal_changed ().connect
     (method (on_description_changed)));
  connections.push_back
    (treeview->get_selection ()->signal_changed ().connect
     (method (on_hero_selected)));
}

void HeroesDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void HeroesDialog::on_undo_activated ()
{
  umgr->undo ();
  update_hero_templates ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void HeroesDialog::on_redo_activated ()
{
  umgr->redo ();
  update_hero_templates ();
  d_changed = true;
  update ();
}

void HeroesDialog::update ()
{
  disconnect_signals ();
  update_panel ();
  update_buttons ();
  umgr->setCursors ();
  connect_signals ();
}

Character* HeroesDialog::getHeroByIndex (HeroesEditorAction_Index *a)
{
  auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
  auto iterrow = treeview->get_model ()->get_iter (path);
  Gtk::TreeModel::Row row = *iterrow;
  Character *hero = row[hero_columns.hero];
  return hero;
}

UndoAction *HeroesDialog::executeAction (UndoAction *action2)
{
  HeroesEditorAction *action = dynamic_cast<HeroesEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case HeroesEditorAction::GENDER:
          {
            HeroesEditorAction_Gender *a =
              dynamic_cast<HeroesEditorAction_Gender*>(action);
            out = new HeroesEditorAction_Gender
              (a->getIndex (), getHeroByIndex (a)->gender);

            getHeroByIndex (a)->gender = a->getGender ();
          }
        break;
      case HeroesEditorAction::NAME:
          {
            HeroesEditorAction_Name *a =
              dynamic_cast<HeroesEditorAction_Name*>(action);
            out = new HeroesEditorAction_Name
              (a->getIndex (), getHeroByIndex (a)->name, umgr,
               name_entry);

            getHeroByIndex (a)->name = a->getName ();
            auto iterrow = treeview->get_model ()->get_iter
              (String::ucompose ("%1", a->getIndex ()));
            if (iterrow)
              {
                Gtk::TreeModel::Row row = *iterrow;
                row[hero_columns.name] = a->getName ();
              }
          } 
        break;
      case HeroesEditorAction::ADD:
          {
            HeroesEditorAction_Add *a =
              dynamic_cast<HeroesEditorAction_Add*>(action);
            out = new HeroesEditorAction_Add
              (HeroTemplates::getInstance ()->copy ());
            HeroTemplates::reset (a->getHeroes ());
            a->clearHeroes ();
            fill_heroes ();

          }
        break;
      case HeroesEditorAction::REMOVE:
          {
            HeroesEditorAction_Remove *a =
              dynamic_cast<HeroesEditorAction_Remove*>(action);
            out = new HeroesEditorAction_Remove
              (HeroTemplates::getInstance ()->copy ());
            HeroTemplates::reset (a->getHeroes ());
            a->clearHeroes ();
            fill_heroes ();
          }
        break;
      case HeroesEditorAction::STRATEGY:
          {
            HeroesEditorAction_Strategy *a =
              dynamic_cast<HeroesEditorAction_Strategy*>(action);
            out = new HeroesEditorAction_Strategy
              (a->getIndex (), getHeroByIndex (a)->strategy);

            HeroStrategy *h = getHeroByIndex (a)->strategy;
            if (h)
              delete h;
            getHeroByIndex (a)->strategy =
              HeroStrategy::copy (a->getStrategy ());
          }
        break;
      case HeroesEditorAction::BACKPACK:
          {
            HeroesEditorAction_Backpack *a =
              dynamic_cast<HeroesEditorAction_Backpack*>(action);
            out = new HeroesEditorAction_Backpack
              (a->getIndex (), getHeroByIndex (a)->item_ids);

            getHeroByIndex (a)->item_ids = a->getBackpack ();
          }
        break;
      case HeroesEditorAction::DESCRIPTION:
          {
            HeroesEditorAction_Desc *a =
              dynamic_cast<HeroesEditorAction_Desc*>(action);
            out = new HeroesEditorAction_Desc
              (a->getIndex (), getHeroByIndex (a)->description, umgr,
               description_entry);

            getHeroByIndex (a)->description = a->getDescription();
          } 
        break;
    }
  return out;
}
