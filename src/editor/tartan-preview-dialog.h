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

#include <gtkmm.h>
#include "lw-dialog-base.h"
#ifndef TARTAN_PREVIEW_DIALOG_H
#define TARTAN_PREVIEW_DIALOG_H
#include "preview-tartan-progress-bar.h"
class TartanPreviewDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "tartan-preview.ui";
      }

    TartanPreviewDialog (BaseObjectType* o,
                         const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_tartan_box = load <Gtk::Box> ("tartan_box");
      }

    ~TartanPreviewDialog ()
      {
        m_tick.disconnect ();
      }

    void setup (Shieldset *shieldset, Shield::Color shield)
      {
        set_valign (Gtk::Align::CENTER);
        m_progress_bar =
          Gtk::make_managed<PreviewTartanProgressBar> (shieldset, shield);

        auto ss = shieldset->lookupShieldByColor (shield);
        int height = ss->get_tallest_tartan_component ();
        m_progress_bar->set_size_request (-1, height);
        m_tartan_box->append (*m_progress_bar);
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
    
        m_tick =
          Glib::signal_timeout ().connect
          ([this] () -> bool
           {
             m_progress_bar->pulse ();
             return true;
           }, 477);

      }
private:
    Gtk::Button *m_close_button;
    Gtk::Box *m_tartan_box;
    PreviewTartanProgressBar *m_progress_bar;
    sigc::connection m_tick;
};
#endif
