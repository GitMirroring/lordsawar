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

#ifndef LW_COMBO_H
#define LW_COMBO_H
class LwCombo: public Gtk::Button
{
public:
  LwCombo ()
  {
    m_box.set_orientation (Gtk::Orientation::HORIZONTAL);
    m_box.set_spacing (6);

    m_label.set_xalign (0.0f);
    m_label.set_hexpand (true);

    m_arrow.set_from_icon_name ("pan-down-symbolic");

    m_box.append (m_label);
    m_box.append (m_arrow);

    set_child (m_box);

    m_popover = Gtk::make_managed<Gtk::Popover> ();
    m_popover->set_has_arrow (false);
    m_popover->set_autohide (true);

    signal_realize ().connect
      ([this]()
       {
         m_popover->set_parent (*this);
       });

    signal_clicked ().connect
      ([this] ()
       {
         if (is_popped ())
           popdown ();
         else
           popup (this);
       });

    signal_unrealize ().connect
      ([this]()
       {
         if (m_popover->get_parent ())
           m_popover->unparent ();
       });

    m_scrolled.set_policy (Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    m_scrolled.set_propagate_natural_height (true);

    auto factory = Gtk::SignalListItemFactory::create ();

    factory->signal_setup ().connect
      ([](const Glib::RefPtr < Gtk::ListItem > &item)
       {
         auto label = Gtk::make_managed <Gtk::Label> ();
         label->set_xalign (0.0f);
         label->set_margin_start (6);
         label->set_margin_end (6);
         label->set_margin_top (4);
         label->set_margin_bottom (4);
         item->set_child (*label);
       });

    factory->signal_bind ().connect
      ([this] (const Glib::RefPtr <Gtk::ListItem> &item)
       {
         auto *label = dynamic_cast <Gtk::Label *>(item->get_child ());
         auto obj = item->get_item ();
         auto str = std::dynamic_pointer_cast <Gtk::StringObject> (obj);
         if (label && str)
           {
             auto context = get_pango_context ();
             auto layout = Pango::Layout::create (context);
             layout->set_text (str->get_string ());
             int w = 0, h = 0;
             layout->get_pixel_size (w, h);

             int w2 = calculate_button_width ();
             auto half = (w2 - w) / 2;
             label->set_margin_start (half - 12);
             label->set_margin_end (half - 12);
             label->set_text (str->get_string ());
           }
       });

    m_model = Gtk::StringList::create ();

    m_selection = Gtk::SingleSelection::create (m_model);

    m_list.set_model (m_selection);
    m_list.set_factory (factory);
    m_list.set_single_click_activate (true);
    m_list.set_vexpand (false);
    m_list.set_hexpand (true);

    m_scrolled.set_child (m_list);
    m_popover->set_child (m_scrolled);

    m_list.signal_activate ().connect
      (sigc::mem_fun (*this, &LwCombo:: on_item_activated));

  }

  virtual ~LwCombo () = default;

  void append (const Glib::ustring &text)
  {
    m_model->append (text);

    if (m_selected < 0)
      set_selected (0);

    update_button_width ();
  }

  void remove_all ()
    {
      m_model = Gtk::StringList::create ();

      m_selection = Gtk::SingleSelection::create (m_model);
      m_list.set_model (m_selection);

      m_selected = -1;
      m_label.set_text ("");
    }

  int get_selected () const
  {
    return m_selected;
  }

  Glib::ustring get_active_text () const
    {
      return get_text ();
    }

  Glib::ustring get_text () const
  {
    return m_label.get_text ();
  }

  void set_active (int i)
    {
      set_selected (i);
    }

  void set_active (const Glib::ustring& text)
    {
      if (!m_model)
        return;

      const guint n = m_model->get_n_items ();

      for (guint i = 0; i < n; ++i)
        {
          auto item = m_model->get_string (i);

          if (item == text)
            {
              set_selected ((int)i);
              return;
            }
        }
    }

  void set_selected (int index)
  {
    if (index < 0 || index >= (int) m_model->get_n_items ())
      return;

    m_selected = index;

    auto text = m_model->get_string (index);
    m_label.set_text (text);

    m_signal_changed.emit ();
  }

  static LwCombo * replace (Gtk::ComboBoxText * combo)
    {
      auto* c = dynamic_cast<Gtk::ComboBox*>(combo);
      return replace (c);
    }

  static LwCombo * replace (Gtk::ComboBox * combo)
  {
    if (!combo)
      return nullptr;

    auto *parent = combo->get_parent ();

    if (!parent)
      return nullptr;

    auto *lw = Gtk::make_managed <LwCombo> ();
    lw->set_hexpand (combo->get_hexpand ());
    lw->set_vexpand (combo->get_vexpand ());
    lw->set_halign (combo->get_halign ());
    lw->set_valign (combo->get_valign ());

    //
    // Copy items
    //
    auto model = combo->get_model ();

    if (model)
      {
        auto children = model->children ();

        for (auto it = children.begin (); it != children.end (); ++it)
          {
            Glib::ustring text;

            it->get_value (0, text);

            lw->append (text);
          }
      }

    //
    // Copy active item
    //
    if (combo->get_model () && combo->get_active ())
      {
        auto path = combo->get_model ()->get_path (combo->get_active ());
        if (path.empty () == false)
          lw->set_selected (path.front ());
      }


    //
    // Replace in parent
    //
    if (auto * box = dynamic_cast <Gtk::Box *>(parent))
      {
        combo->unparent ();

	box->append (*lw);
      }
    else if (auto * grid = dynamic_cast <Gtk::Grid *>(parent))
      {
	int column = 0;
	int row = 0;
	int width = 1;
	int height = 1;

	grid->query_child (*combo, column, row, width, height);

        combo->unparent ();

	grid->attach (*lw, column, row, width, height);
      }
    else
      {
	return nullptr;
      }

    return lw;
  }

  int get_active_row_number ()
    {
      return get_selected ();
    }

  sigc::signal<void ()> signal_changed ()
  {
    return m_signal_changed;
  }

  bool is_popped ()
    {
      return m_popover->is_visible ();
    }
  void popdown ()
    {
      m_popover->popdown ();
    }

  void popup (Gtk::Widget *parent)
    {
      if (!m_popover->get_parent ())
        m_popover->set_parent (*parent);
      m_popover->set_pointing_to
        (Gdk::Rectangle (0, 0, get_width (), get_height () + 6));

      m_popover->popup ();
    }
private:

  void on_item_activated (guint position)
  {
    set_selected (position);

    m_popover->popdown ();
  }

  int calculate_button_width ()
    {
      if (!m_model)
        return 0;

      int max_width = 0;

      auto context = get_pango_context ();

      for (guint i = 0; i < m_model->get_n_items (); ++i)
        {
          auto text = m_model->get_string (i);

          auto layout = Pango::Layout::create (context);

          layout->set_text(text);

          int w = 0, h = 0;
          layout->get_pixel_size (w, h);

          if (w > max_width)
            max_width = w;
        }

      // add padding + arrow space + label margins
      int fudge = 24;
      max_width += 6 + 6 + m_box.get_spacing () + m_arrow.get_width () + fudge;
      return max_width;
    }

  void update_button_width ()
    {
      int w = calculate_button_width ();
      set_size_request (w, -1);
    }
private:
  Gtk::Popover *m_popover;
  Gtk::Box m_box;
  Gtk::Label m_label;
  Gtk::Image m_arrow;
  Gtk::ScrolledWindow m_scrolled;
  Gtk::ListView m_list;
  Glib::RefPtr <Gtk::StringList> m_model;
  Glib::RefPtr <Gtk::SingleSelection> m_selection;

  int m_selected = -1;

  sigc::signal <void ()> m_signal_changed;
};
#endif
