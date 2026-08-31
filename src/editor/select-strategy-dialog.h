//  Copyright (C) 2021, 2026 Ben Asselstine
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
#ifndef SELECT_STRATEGY_DIALOG_H
#define SELECT_STRATEGY_DIALOG_H
#include "hero-strategy.h"
class SelectStrategyRow: public Glib::Object
{
public:
    HeroStrategy *m_strategy;

    static Glib::RefPtr<SelectStrategyRow> create (HeroStrategy *s)
      {
        return
          Glib::make_refptr_for_instance<SelectStrategyRow>
          (new SelectStrategyRow (s));
      }

protected:
    SelectStrategyRow (HeroStrategy *s)
      : m_strategy (s)
      {
      }
};

class SelectStrategyDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "select-strategy.ui";
      }

    enum
      {
        NO_STRATEGY_SELECTED = -1
      };

    SelectStrategyDialog (BaseObjectType* o,
                          const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_select_button = load <Gtk::Button> ("select_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    ~SelectStrategyDialog ()
      {
        clear_treeview ();
      }

    void setup (int strategy_type)
      {
        m_selected_strategy = strategy_type;

        set_response (m_select_button, Gtk::ResponseType::ACCEPT);

        m_store = Gio::ListStore<SelectStrategyRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        setup_name_column ();

        update ();

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             auto item = m_selection_model->get_selected_item ();
             if (item)
               {
                 auto row = std::dynamic_pointer_cast<SelectStrategyRow>(item);
                 m_selected_strategy = row->m_strategy->getType ();
               }
           });

             
        auto item = m_selection_model->get_selected_item ();
        if (item)
          {
            auto row = std::dynamic_pointer_cast<SelectStrategyRow>(item);
            m_selected_strategy = row->m_strategy->getType ();
          }
      }

    void update ()
      {
        fill_treeview ();
      }

    HeroStrategy* get_selected_strategy ()
      {
        if (m_selected_strategy == NO_STRATEGY_SELECTED)
          return NULL;
        return m_strategies[m_selected_strategy];
      }
private:
    Gtk::Button *m_select_button;
    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<SelectStrategyRow>> m_store;

    std::vector<HeroStrategy*> m_strategies;
    int m_selected_strategy;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<SelectStrategyRow> 
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Strategies"),
           [] (const auto& row)
           {
             return row->m_strategy->getDescription ();
           });
      }

    void fill_treeview ()
      {
        clear_treeview ();

        m_strategies.push_back (new HeroStrategy_None ());
        m_strategies.push_back (new HeroStrategy_Random ());
        m_strategies.push_back (new HeroStrategy_SimpleQuester ());

        int i = 0;
        int found = -1;
        for (auto s : m_strategies)
          {
            m_store->append (SelectStrategyRow::create (s));
            if (s->getType () == HeroStrategy::Type (m_selected_strategy))
              found = i;
            i++;
          }

        if (found >= 0)
          m_selection_model->set_selected (found);
      }

    void clear_treeview ()
      {
        for (auto s : m_strategies)
          delete s;
        m_strategies.clear ();
        m_store->remove_all ();
      }

};
#endif
