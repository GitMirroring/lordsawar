//  Copyright (C) 2020, 2021, 2026 Ben Asselstine
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
#ifndef CHARACTER_EDITOR_DIALOG_H
#define CHARACTER_EDITOR_DIALOG_H
#include "character-undo-actions.h"
#include "select-strategy-dialog.h"
#include "starting-items-dialog.h"

class CharacterEditorRow: public Glib::Object
{
public:
    Character *m_character;

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    static Glib::RefPtr<CharacterEditorRow> create (Character *c)
      {
        return
          Glib::make_refptr_for_instance<CharacterEditorRow>
          (new CharacterEditorRow (c));
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }
protected:
    CharacterEditorRow (Character *c)
      : m_character (c)
      {
      }
private:
    sigc::signal<void()> m_signal_changed;
};

class CharacterEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "character-editor.ui";
      }

    CharacterEditorDialog (BaseObjectType* o,
                           const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_description_entry = load <Gtk::Entry> ("description_entry");
        m_items_button = load <Gtk::Button> ("items_button");
        m_strategy_button = load <Gtk::Button> ("strategy_button");
        m_gender_combobox = load <Gtk::Box> ("gender_combobox");
        m_scrolled_window = load <Gtk::ScrolledWindow> ("scrolled_window");
      }

    ~CharacterEditorDialog ()
      {
        disconnect_signals ();
        delete m_hero_templates;
        delete m_umgr;
      }

    HeroTemplates *get_hero_templates ()
      {
        return m_hero_templates;
      }

    void setup (Shield::Color shield, HeroTemplates *hero_templates)
      {
        m_hero_templates = hero_templates->copy ();
        m_shield = shield;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_store = Gio::ListStore<CharacterEditorRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        m_add_button->set_icon_name ("list-add-symbolic");
        m_remove_button->set_icon_name ("list-remove-symbolic");

        fill_gender_combo ();
        setup_undo ();

        setup_name_column ();

        fill_treeview ();

        update ();

      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

    void update ()
      {
        disconnect_signals ();


        auto c = get_selected_character ();
        bool sensitive = false;
        if (c)
          {
            sensitive = true;

            m_name_entry->set_text (c->get_name ());

            m_description_entry->set_text (c->get_description ());

            if (c->get_starting_item_ids ().size () == 0)
              m_items_button->set_label (_("No starting items"));
            else
              m_items_button->set_label
                (String::ucompose
                 (ngettext ("Starting with %1 item",
                            "Starting with %1 items",
                            c->get_starting_item_ids ().size ()),
                  c->get_starting_item_ids ().size ()));

            if (c->get_strategy () == NULL)
              m_strategy_button->set_label (_("No strategy set"));
            else
              m_strategy_button->set_label
                (c->get_strategy ()->getDescription ());

            switch (c->get_gender ())
              {
              case Hero::MALE:
                m_gender_combo->set_active (1);
                break;

              case Hero::FEMALE:
                m_gender_combo->set_active (2);
                break;

              default:
                m_gender_combo->set_active (0);
                break;
              }


          }
        else
          {
            sensitive = false;
            m_name_entry->set_text ("");
            m_gender_combo->set_active (0);
            m_items_button->set_label (_("No starting items"));
            m_strategy_button->set_label (_("No strategy set"));
            m_description_entry->set_text ("");
          }

        m_name_entry->set_sensitive (sensitive);
        m_gender_combo->set_sensitive (sensitive);
        m_items_button->set_sensitive (sensitive);
        m_strategy_button->set_sensitive (sensitive);
        m_description_entry->set_sensitive (sensitive);


        update_buttons ();

        connect_signals ();
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    Gtk::ColumnView *m_treeview;
    Gtk::Entry *m_name_entry;
    Gtk::Entry *m_description_entry;
    Gtk::Button *m_items_button;
    Gtk::Button *m_strategy_button;
    Gtk::Box *m_gender_combobox;
    Gtk::ScrolledWindow *m_scrolled_window;
    LwCombo *m_gender_combo;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<CharacterEditorRow>> m_store;
    HeroTemplates *m_hero_templates;

    UndoMgr *m_umgr;
    Shield::Color m_shield;
    std::list<sigc::connection> m_connections;

    void setup_name_column ()
      {
        LwColumn::setup_name_column<CharacterEditorRow> 
          (m_treeview, true, _("Heroes"),
           [] (const auto& row)
           {
             return row->m_character->get_name ();
           });
      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        for (auto c : m_hero_templates->getHeroes (m_shield))
          m_store->append (CharacterEditorRow::create (c));
      }

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
        add_connection
          (m_selection_model->signal_selection_changed ().connect
           ([this] (const guint &, const guint &)
            {
              auto item = m_selection_model->get_selected_item ();
              if (item)
                {
                  //auto row = std::dynamic_pointer_cast<CharacterEditorRow>(item);
                  update ();
                }
            }));

        add_connection
          (m_add_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add
                (new CharacterUndoAction_Add
                 (m_hero_templates->copy ()));

              std::list<guint32> ids;
              guint32 id = m_hero_templates->getNextAvailableId ();
              Character *c = new 
                Character (m_shield, _("Unnamed Hero"), "", id,
                           Hero::FEMALE, ids, new HeroStrategy_None);

              m_store->append (CharacterEditorRow::create (c));

              guint n = m_selection_model->get_n_items ();
              if (n > 0)
                m_selection_model->set_selected (n - 1);

              scroll_character_to_top ();
              scroll_treeview_to_bottom ();
              update_hero_templates ();

              update ();
            }));

        add_connection
          (m_remove_button->signal_clicked ().connect
           ([this] ()
            {
              auto selected = m_selection_model->get_selected_item ();
              if (selected)
                {
                  m_umgr->add
                    (new CharacterUndoAction_Remove
                     (m_hero_templates->copy ()));

                  m_store->remove (m_selection_model->get_selected ());
                  scroll_character_to_top ();
                  update_hero_templates ();
                  update ();
                }
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              auto chr = get_selected_character ();
              m_umgr->add
                (new CharacterUndoAction_Name
                 (m_selection_model->get_selected (), chr->get_name (),
                  m_umgr, m_name_entry));

              chr->set_name (m_name_entry->get_text ());

              auto i = m_selection_model->get_selected_item ();
              if (i)
                {
                  auto row = std::dynamic_pointer_cast<CharacterEditorRow>(i);
                  row->changed ();
                }
            }));

        add_connection
          (m_strategy_button->signal_clicked ().connect
           ([this] ()
            {
              Character *character = get_selected_character ();
              if (!character)
                return;
              auto d = LwDialog::build<SelectStrategyDialog> (this);
              int type = -1;
              if (character->get_strategy ())
                type = (int)character->get_strategy ()->getType ();
              d->setup (type);

              d->signal_response ().connect
                ([this, character, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                         {
                           auto strategy = d->get_selected_strategy ();
                           if (strategy)
                             {
                               m_umgr->add
                                 (new CharacterUndoAction_Strategy
                                  (m_selection_model->get_selected (),
                                   character->get_strategy ()));
                               character->set_strategy
                                 (HeroStrategy::copy (strategy));
                               update_hero_templates ();
                               update ();
                             }
                           break;
                         }
                     default:
                       break;
                     }
                   delete d;
                 });
            }));

        add_connection
          (m_items_button->signal_clicked ().connect
           ([this] ()
            {
              Character *character = get_selected_character ();
              if (!character)
                return;
              auto d = LwDialog::build<StartingItemsDialog> (this);
              d->setup (character->get_starting_item_ids ());
              d->signal_response ().connect
                ([this, character, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       if (d->is_changed ())
                         {
                           m_umgr->add
                             (new CharacterUndoAction_Backpack
                              (m_selection_model->get_selected (),
                               character->get_starting_item_ids ()));

                           character->set_starting_item_ids
                             (d->get_starting_item_ids ());
                           update_hero_templates ();
                           update ();
                         }
                       break;

                     default:
                       break;
                     }
                   delete d;
                 });
            }));
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        CharacterUndoAction *action = dynamic_cast<CharacterUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case CharacterUndoAction::GENDER:
              {
                auto a = dynamic_cast<CharacterUndoAction_Gender*>(action);
                out = new CharacterUndoAction_Gender
                  (a->get_index (), get_character_by_index (a)->get_gender ());

                get_character_by_index (a)->set_gender (a->get_gender ());
              }
            break;

          case CharacterUndoAction::NAME:
              {
                auto a = dynamic_cast<CharacterUndoAction_Name*>(action);
                out = new CharacterUndoAction_Name
                  (a->get_index (), get_character_by_index (a)->get_name (),
                   m_umgr, m_name_entry);

                get_character_by_index (a)->set_name (a->get_name ());
                auto item = m_store->get_item (a->get_index ());
                if (item)
                  {
                    auto row =
                      std::dynamic_pointer_cast<CharacterEditorRow>(item);
                    row->changed ();
                  }
              } 
            break;

          case CharacterUndoAction::ADD:
              {
                auto a = dynamic_cast<CharacterUndoAction_Add*>(action);
                out = new CharacterUndoAction_Add
                  (m_hero_templates->copy ());
                delete m_hero_templates;
                m_hero_templates = a->get_heroes ();
                a->clear_heroes ();
                fill_treeview ();
              }
            break;

          case CharacterUndoAction::REMOVE:
              {
                auto a = dynamic_cast<CharacterUndoAction_Remove*>(action);
                out = new CharacterUndoAction_Remove
                  (m_hero_templates->copy ());
                delete m_hero_templates;
                m_hero_templates = a->get_heroes ();
                a->clear_heroes ();
                fill_treeview ();
              }
            break;

          case CharacterUndoAction::STRATEGY:
              {
                auto a = dynamic_cast<CharacterUndoAction_Strategy*>(action);
                out = new CharacterUndoAction_Strategy
                  (a->get_index (),
                   get_character_by_index (a)->get_strategy ());

                get_character_by_index (a)->set_strategy
                  (HeroStrategy::copy (a->get_strategy ()));
              }
            break;

          case CharacterUndoAction::BACKPACK:
              {
                CharacterUndoAction_Backpack *a =
                  dynamic_cast<CharacterUndoAction_Backpack*>(action);
                out = new CharacterUndoAction_Backpack
                  (a->get_index (),
                   get_character_by_index (a)->get_starting_item_ids ());

                get_character_by_index (a)->set_starting_item_ids
                  (a->get_backpack ());
              }
            break;

          case CharacterUndoAction::DESCRIPTION:
              {
                CharacterUndoAction_Desc *a =
                  dynamic_cast<CharacterUndoAction_Desc*>(action);
                out = new CharacterUndoAction_Desc
                  (a->get_index (),
                   get_character_by_index (a)->get_description (), m_umgr,
                   m_description_entry);

                get_character_by_index (a)->set_description
                  (a->get_description ());
              } 
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &CharacterEditorDialog::execute_action));

        setup_undo_and_redo ();

        signal_undo ().connect
           ([this] ()
            {
              m_umgr->undo ();
              update ();
            });

        signal_redo ().connect
           ([this] ()
            {
              m_umgr->redo ();
              update ();
            });
        m_umgr->add_cursor (m_name_entry);
        m_umgr->add_cursor (m_description_entry);
      }

    void fill_gender_combo ()
      {
        m_gender_combo = Gtk::make_managed<LwCombo> ();
        m_gender_combo->append (_("None"));
        m_gender_combo->append (_("Male"));
        m_gender_combo->append (_("Female"));
        m_gender_combo->set_hexpand (false);
        m_gender_combobox->append (*m_gender_combo);
      }

    Character *get_character_by_index (CharacterUndoAction_Index *action)
      {
        auto item = m_store->get_item (action->get_index ());
        auto row = std::dynamic_pointer_cast<CharacterEditorRow>(item);
        if (row)
          return row->m_character;
        else
          return NULL;
      }

    void scroll_character_to_top ()
      {
        m_scrolled_window->get_vadjustment ()->set_value (0.0);
      }

    void scroll_treeview_to_bottom ()
      {
        guint n = m_store->get_n_items ();
        Glib::signal_idle ().connect_once
          ([this, n] ()
           {
             m_treeview->scroll_to (n - 1);
           });
      }

    void update_hero_templates ()
      {
        std::vector<Character*> heroes;
        guint n = m_store->get_n_items ();
        for (guint i = 0; i < n; i++)
          {
            auto item = m_store->get_item (i);
            if (!item)
              continue;
            auto row = std::dynamic_pointer_cast<CharacterEditorRow>(item);
            auto character = row->m_character;
            heroes.push_back (Character::copy (character));
          }
        m_hero_templates->replaceHeroes (m_shield, heroes);
      }

    Character *get_selected_character ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<CharacterEditorRow> (item);
            return row->m_character;
          }
        return NULL;
      }

    void update_buttons ()
      {
        guint n = m_store->get_n_items ();
        m_remove_button->set_sensitive (n > 0);
      }

};
#endif
