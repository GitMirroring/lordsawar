//  Copyright (C) 2026 Ben Asselstine
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

#pragma once

#include <gtkmm.h>

#ifndef LW_COLUMN_H
#define LW_COLUMN_H

template<typename RowType>
class NumberCell : public Gtk::Box
{
public:
    NumberCell (Glib::RefPtr<Gtk::Adjustment> adj, const Glib::RefPtr<Gtk::ListItem>& item)
      : Gtk::Box (Gtk::Orientation::HORIZONTAL, 6), m_label ("0"),
      m_btn_edit (), m_popover (), m_spinbutton (), m_item (item)
  {
    set_size_request (LW_BUTTON_SIZE * 1.25, -1);
    set_halign (Gtk::Align::START);
    set_valign (Gtk::Align::CENTER);

    m_label.set_halign (Gtk::Align::START);
    m_btn_edit.set_icon_name ("document-edit-symbolic");
    m_btn_edit.add_css_class ("compact-button");
    m_btn_edit.add_css_class ("flat");
    m_btn_edit.add_css_class ("small");
    m_btn_edit.set_valign (Gtk::Align::CENTER);
    m_btn_edit.set_visible (false);
    m_popover.set_parent (m_btn_edit);
    m_popover.set_position (Gtk::PositionType::BOTTOM);
    m_spinbutton.set_sensitive (true);
    m_spinbutton.set_numeric (true);
    m_spinbutton.set_orientation (Gtk::Orientation::VERTICAL);
    m_spinbutton.set_adjustment (adj);
    m_spinbutton.set_width_chars (3);

    auto* popover_box =
      Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 6);
    popover_box->set_margin (6);
    popover_box->append (m_spinbutton);

    m_popover.set_child (*popover_box);

    m_popover.signal_map ().connect
      ([this] ()
       {
         m_spinbutton.grab_focus ();
       });

    m_popover.signal_closed ().connect
      ([this] ()
       {
         if (!m_ignore_close)
           {
             m_value = m_spinbutton.get_value ();
             update_label ();
             m_signal_changed.emit (m_spinbutton.get_value ());
           }
         if (!m_is_hovered)
           {
             m_btn_edit.set_visible (false);
           }
       });

    m_btn_edit.signal_clicked ().connect
      ([this] ()
       {
         if (m_popover.get_visible ())
           return;
         if (!m_item->get_selected ())
           {
             m_signal_clicked_on_unselected_cell.emit ();
             return;
           }

         m_spinbutton.set_value (m_value);
         m_popover.popup ();
       });

    append (m_label);
    append (m_btn_edit);

    auto motion = Gtk::EventControllerMotion::create ();
    motion->signal_enter ().connect
      ([this](double, double)
       {
         m_is_hovered = true;
         m_btn_edit.set_visible (true);
       });

    motion->signal_leave ().connect
      ([this] ()
       {
         m_is_hovered = false;
         if (!m_popover.get_visible ())
           {
             m_btn_edit.set_visible (false);
           }
       });
    add_controller (motion);
  }

    ~NumberCell () override
      {
        m_ignore_close = true;
        m_popover.popdown ();
        m_popover.unparent ();
      }

    void set_item (double value)
      {
        m_ignore_close = true;
        m_popover.popdown ();
        m_ignore_close = false;

        m_value = value;
        update_label ();
      }

    sigc::signal<void(double)> signal_changed ()
      {
        return m_signal_changed;
      }

    sigc::signal<void()> signal_clicked_on_unselected_cell ()
      {
        return m_signal_clicked_on_unselected_cell;
      }
private:
    void update_label ()
      {
        m_label.set_text (String::ucompose ("%1", (guint32) m_value));
      }

    bool m_is_hovered = false;
    bool m_ignore_close = false;
    Gtk::Label m_label;
    Gtk::Button m_btn_edit;
    Gtk::Popover m_popover;
    Gtk::SpinButton m_spinbutton;
    const Glib::RefPtr<Gtk::ListItem> m_item;
    double m_value;
    sigc::signal<void(double)> m_signal_changed;
    sigc::signal<void()> m_signal_clicked_on_unselected_cell;
};

class LwColumn
{
public:
    // the name column differs in that it expects RowType to have a changed
    // signal.  it's used in cases where the name column needs to be updated.
    template<typename RowType, typename Func>
      static void setup_name_column (Gtk::ColumnView *treeview, bool expand,
                                     const Glib::ustring& title,
                                     Func&& text_func)
        {
          Glib::ustring quark = "name_label";
          auto factory = Gtk::SignalListItemFactory::create ();

          factory->signal_setup ().connect
            ([quark] (const Glib::RefPtr<Gtk::ListItem>& item)
             {
               auto label = Gtk::make_managed<Gtk::Label>();
               label->set_halign (Gtk::Align::START);
               label->set_justify (Gtk::Justification::LEFT);
               item->set_child (*label);
               item->set_data (quark, label);
             });

          factory->signal_bind () .connect
            ([quark, func = std::forward<Func>(text_func)]
             (const Glib::RefPtr<Gtk::ListItem>& item)
             {
               auto row = std::dynamic_pointer_cast<RowType>(item->get_item ());
               auto label = static_cast<Gtk::Label*>(item->get_data (quark));

               if (label && row)
                 {
                   label->set_text (func (row));

                   row->signal_changed ().connect
                     ([row, label, func] ()
                      {
                        label->set_text (func (row));
                      });
                 }
             });

          auto column = Gtk::ColumnViewColumn::create (title, factory);
          column->set_expand (expand);
          treeview->append_column (column);
        }

    template<typename RowType, typename Func>
      static void setup_text_column (Gtk::ColumnView *treeview,
                                     const Glib::ustring& quark,
                                     bool expand, Gtk::Justification j,
                                     const Glib::ustring& title,
                                     Func&& text_func)
        {
          auto factory = Gtk::SignalListItemFactory::create ();

          factory->signal_setup ().connect
            ([quark, j] (const Glib::RefPtr<Gtk::ListItem>& item)
             {
               auto label = Gtk::make_managed<Gtk::Label>();
               label->set_halign (Gtk::Align::START);
               label->set_justify (j);
               item->set_child (*label);
               item->set_data (quark, label);
             });

          factory->signal_bind () .connect
            ([quark, func = std::forward<Func>(text_func)]
             (const Glib::RefPtr<Gtk::ListItem>& item)
             {
               auto row = std::dynamic_pointer_cast<RowType>(item->get_item ());
               auto label = static_cast<Gtk::Label*>(item->get_data (quark));

               if (label && row)
                 label->set_text (func (row));
             });

          auto column = Gtk::ColumnViewColumn::create (title, factory);
          column->set_expand (expand);
          treeview->append_column (column);
        }

    template<typename RowType, typename IFunc>
      static void setup_picture_column (Gtk::ColumnView *treeview,
                                        int width, int height,
                                        const Glib::ustring& q,
                                        const Glib::ustring& title, IFunc ifunc)
        {
          auto factory = Gtk::SignalListItemFactory::create ();

          factory->signal_setup ().connect
            ([q, width, height] (const Glib::RefPtr<Gtk::ListItem>& item)
             {
               auto picture = Gtk::make_managed<Gtk::Picture> ();
               picture->set_can_shrink (true);
               picture->set_size_request (width, height);
               item->set_child (*picture);
               item->set_data (q, picture);
             });

          factory->signal_bind ().connect
            ([q, ifunc](const Glib::RefPtr<Gtk::ListItem>& item)
             {
               auto row = std::dynamic_pointer_cast<RowType>(item->get_item ());
               auto picture = static_cast<Gtk::Picture*>(item->get_data (q));

               if (row && picture)
                 {
                   auto paintable = ifunc (row);
                   picture->set_paintable (paintable);
                 }
             });

          auto column = Gtk::ColumnViewColumn::create (title, factory);

          treeview->append_column (column);
        }

    //Ifunc returns a number suitable for set_value on a spinbutton
    //Jfunc has parameters row, and value, so we can change the model
    template<typename RowType, typename IFunc, typename JFunc>
      static void setup_number_column (Gtk::ColumnView *treeview,
                                       const Glib::ustring& q,
                                       sigc::connection &conn,
                                       Glib::RefPtr<Gtk::Adjustment> adj,
                                       const Glib::ustring& title, IFunc ifunc,
                                       JFunc jfunc)
        {
          auto factory = Gtk::SignalListItemFactory::create ();

          factory->signal_setup ().connect
            ([q, adj, treeview] (const Glib::RefPtr<Gtk::ListItem>& item) mutable
             {
               auto cell = Gtk::make_managed<NumberCell<RowType>> (adj, item);
               cell->signal_clicked_on_unselected_cell ().connect
                 ([treeview, item, cell] ()
                  {
                    auto selection =
                      std::dynamic_pointer_cast<Gtk::SingleSelection>(treeview->get_model ());
                    selection->set_selected (item->get_position ());
                  });
               item->set_child (*cell);
             });

          factory->signal_bind ().connect
            ([q, ifunc, jfunc, conn](const Glib::RefPtr<Gtk::ListItem>& item) mutable
             {
               auto* cell =
                 dynamic_cast<NumberCell<RowType>*>(item->get_child ());
               auto num =
                 std::dynamic_pointer_cast<RowType>(item->get_item ());
               if (cell && num)
                 {
                   conn.block ();
                   cell->set_item (ifunc (num));
                   conn.disconnect ();
                   //we closed a spinbutton and now we want to update the model
                   conn = cell->signal_changed ().connect
                     ([num, jfunc] (double value)
                      {
                        jfunc (num, (guint32)value);
                      });

                   //we did an undo and we want to reflect the change in the
                   //model
                   num->signal_changed ().connect
                     ([num, cell, ifunc] ()
                      {
                        cell->set_item (ifunc (num));
                      });
                 }
             });

          auto column = Gtk::ColumnViewColumn::create (title, factory);

          treeview->append_column (column);
        }

};

#endif
