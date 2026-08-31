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
#ifndef MASKED_IMAGE_EDITOR_DIALOG_H
#define MASKED_IMAGE_EDITOR_DIALOG_H
#include "file-filter.h"
#include "masked-image-undo-actions.h"
#include "file-label.h"

class MaskedImageEditorDialog: public LwDialogBase
{
public:
    static const int MAX_IMAGES_WIDTH = 1000;
    static std::string get_resource_name ()
      {
        return "masked-image-editor.ui";
      }

    MaskedImageEditorDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_clear_button = load <Gtk::Button> ("clear_button");
        m_magnify_button = load <Gtk::ToggleButton> ("magnify_button");
        m_image_button = load <Gtk::Button> ("imagebutton");
        m_white_image = load <Gtk::Picture> ("white_image");
        m_green_image = load <Gtk::Picture> ("green_image");
        m_yellow_image = load <Gtk::Picture> ("yellow_image");
        m_light_blue_image = load <Gtk::Picture> ("light_blue_image");
        m_red_image = load <Gtk::Picture> ("red_image");
        m_dark_blue_image = load <Gtk::Picture> ("dark_blue_image");
        m_orange_image = load <Gtk::Picture> ("orange_image");
        m_black_image = load <Gtk::Picture> ("black_image");
        m_neutral_image = load <Gtk::Picture> ("neutral_image");
        m_shieldset_box = load <Gtk::Box> ("shieldset_box");
        m_shield_box = load <Gtk::Box> ("shield_box");
        m_frame_label = load <Gtk::Label> ("frame_label");
        m_one_color = false;
        m_color = Shield::WHITE;
        m_shieldset = NULL;
        m_override_default = false;
      }

    ~MaskedImageEditorDialog ()
      {
        delete m_umgr;
        delete m_mim;
        disconnect_signals ();
        m_tick.disconnect ();
      }

    Glib::ustring get_filename ()
      {
        return m_target_filename;
      }

    bool is_changed ()
      {
        return m_target_filename != m_orig_target_filename;
      }

    bool install_file (TarFile *t, TarFileMaskedImage *im,
                       Glib::ustring filename, Glib::ustring &err)
      {
        std::string newname;
        bool success = false;
        if (m_orig_target_filename.empty () == true)
          success = t->addFileInCfgFile (filename, newname, err);
        else
          success =
            t->replaceFileInCfgFile (m_orig_target_filename, filename, newname,
                                     err);
        im->setName(newname);
        im->load (t, newname);
        im->instantiateImages();
        return success;
      }

    bool uninstall_file (TarFile *t, TarFileMaskedImage *im, Glib::ustring &err)
      {
        im->clear ();
        return t->removeFileInCfgFile(m_orig_target_filename, err);
      }

    void set_override ()
      {
        m_override_default = true;
      }

    void set_color (Shieldset *s, Shield::Color color)
      {
        m_shieldset = s;
        m_one_color = true;
        m_color = color;
      }

    void set_shieldset (Shieldset *s)
      {
        m_shieldset = s;
      }

    void setup (TarFileMaskedImage *im)
      {
        m_active_frame = 0;
        setup_shield_theme_combobox ();

        m_target_filename = im->getName ();
        m_orig_target_filename  = m_target_filename;
        m_mim = new TarFileMaskedImage (*im);

        if (m_magnify_button)
          {
            auto width = 0;
            if (im->getImage ())
              width = im->getImage ()->get_width ();

            m_magnify_button->set_visible (width <= 40);
          }

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        set_response (m_clear_button, Gtk::ResponseType::REJECT);

        m_clear_button->set_visible (!im->getName ().empty ());

        m_image_file_label = Gtk::make_managed<FileLabel> (m_image_button);
        m_image_button->set_tooltip_text (_("Select a new image"));

        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &MaskedImageEditorDialog::execute_action));

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

        m_magnify_button->signal_toggled ().connect 
          ([this, im] ()
           {
             const int scale_factor = 8;
             auto width = 0, height = 0;
             if (im->getImage ())
               {
                 width = im->getImage ()->get_width ();
                 height = im->getImage ()->get_height ();

                 bool magnify = m_magnify_button->get_active ();
                 if (magnify)
                   {
                     m_white_image->set_can_shrink (false);
                     m_white_image->set_content_fit (Gtk::ContentFit::CONTAIN);
                     m_white_image->set_size_request (width * scale_factor,
                                                      height * scale_factor);
                     m_green_image->set_can_shrink (false);
                     m_green_image->set_content_fit (Gtk::ContentFit::CONTAIN);
                     m_green_image->set_size_request (width * scale_factor,
                                                      height * scale_factor);
                     m_yellow_image->set_can_shrink (false);
                     m_yellow_image->set_content_fit (Gtk::ContentFit::CONTAIN);
                     m_yellow_image->set_size_request (width * scale_factor,
                                                       height * scale_factor);
                     m_dark_blue_image->set_can_shrink (false);
                     m_dark_blue_image->set_content_fit
                       (Gtk::ContentFit::CONTAIN);
                     m_dark_blue_image->set_size_request
                       (width * scale_factor, height * scale_factor);
                     m_orange_image->set_can_shrink (false);
                     m_orange_image->set_content_fit (Gtk::ContentFit::CONTAIN);
                     m_orange_image->set_size_request (width * scale_factor,
                                                       height * scale_factor);
                     m_light_blue_image->set_can_shrink (false);
                     m_light_blue_image->set_content_fit
                       (Gtk::ContentFit::CONTAIN);
                     m_light_blue_image->set_size_request
                       (width * scale_factor, height * scale_factor);
                     m_red_image->set_can_shrink (false);
                     m_red_image->set_content_fit (Gtk::ContentFit::CONTAIN);
                     m_red_image->set_size_request (width * scale_factor,
                                                    height * scale_factor);
                     m_black_image->set_can_shrink (false);
                     m_black_image->set_content_fit (Gtk::ContentFit::CONTAIN);
                     m_black_image->set_size_request (width * scale_factor,
                                                      height * scale_factor);
                     m_neutral_image->set_can_shrink (false);
                     m_neutral_image->set_content_fit
                       (Gtk::ContentFit::CONTAIN);
                     m_neutral_image->set_size_request (width * scale_factor,
                                                      height * scale_factor);
                     m_magnify_button->set_sensitive (false);
                   }
               }
           });

        update ();

        if (im->getNumberOfFrames () > 1)
          m_tick = Glib::signal_timeout ().connect
            (sigc::bind_return
             (sigc::bind
              (sigc::mem_fun (*this, &MaskedImageEditorDialog::on_tick),
               true), true), 500);
        else
          m_frame_label->set_visible (false);
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_clear_button;
    Gtk::Button *m_image_button;
    Gtk::ToggleButton *m_magnify_button;
    FileLabel *m_image_file_label;
    Gtk::Picture *m_white_image;
    Gtk::Picture *m_green_image;
    Gtk::Picture *m_yellow_image;
    Gtk::Picture *m_light_blue_image;
    Gtk::Picture *m_red_image;
    Gtk::Picture *m_dark_blue_image;
    Gtk::Picture *m_orange_image;
    Gtk::Picture *m_black_image;
    Gtk::Picture *m_neutral_image;
    Gtk::Box *m_shieldset_box; //container for combobox
    LwCombo *m_shield_theme_combobox;
    Gtk::Box *m_shield_box; //container for whole shieldset section
    Gtk::Label *m_frame_label; 
    guint32 m_shield_row;
    bool m_one_color;
    Shield::Color m_color;

    UndoMgr *m_umgr;
    Glib::ustring m_target_filename;
    Glib::ustring m_orig_target_filename;
    bool m_override_default = false;
    TarFileMaskedImage *m_mim;
    Shieldset *m_shieldset;
    guint32 m_active_frame;
    std::list<sigc::connection> m_connections;
    sigc::connection m_tick;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
        add_connection
          (m_image_button->signal_clicked ().connect
           ([this] ()
            {
              LwDialog::open
                (*this, get_title (), FileFilter::IMAGE,
                 [this] (std::string path)
                 {
                   open_file (path);
                 });
            }));
  
        add_connection
          (m_shield_theme_combobox->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new MaskedImageUndoAction_Shield (m_shield_row));
              show_image ();
              m_shield_row = m_shield_theme_combobox->get_active_row_number ();
            }));
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        auto action =
          dynamic_cast<MaskedImageUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case MaskedImageUndoAction::SET:
              {
                auto a =
                  dynamic_cast<MaskedImageUndoAction_Set*>(action);
                out = new MaskedImageUndoAction_Set (m_mim, m_target_filename);
                a->get_image ()->copyFrames (m_mim);
                m_target_filename = a->get_file_name ();
              } 
            break;

          case MaskedImageUndoAction::SHIELD:
              {
                auto a = dynamic_cast<MaskedImageUndoAction_Shield*>(action);
                out = new MaskedImageUndoAction_Shield
                  (m_shield_theme_combobox->get_active_row_number ());
                m_shield_row = a->get_shield ();
              }
            break;
          }
        return out;
      }

    void update_imagebutton_label (Glib::ustring filename)
      {
        Glib::ustring f = File::get_basename (filename, true);
        if (f.empty () == false)
          m_image_file_label->set_label (f);
        else
          {
            if (m_override_default)
              m_image_file_label->set_override ();
            m_image_file_label->set_label ("");
          }
      }

    void update ()
      {
        disconnect_signals ();
        update_imagebutton_label (m_target_filename);
        show_image ();
        connect_signals ();
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    void setup_shield_theme_combobox ()
      {
        Gtk::Box *box = m_shieldset_box;
        // fill in shield themes combobox
        m_shield_theme_combobox = Gtk::make_managed<LwCombo> ();

        int counter = 0;
        int default_id = 0;
        for (auto shield_theme : Shieldsetlist::instance ()->getValidIds ())
          {
            auto s = Shieldsetlist::instance ()->get (shield_theme);
            if (s->getId () == 0)
              default_id = counter;
            m_shield_theme_combobox->append
              (Glib::filename_to_utf8 (s->getName ()));
            counter++;
          }

        m_shield_theme_combobox->set_active (default_id);
        m_shield_row = default_id;

        box->append (*m_shield_theme_combobox);
        if (m_shieldset)
          m_shield_box->set_visible (false);
      }

    bool load_image ()
      {
        m_active_frame = 0;
        bool broken = m_mim->loadFromFile (m_target_filename);

        if (!broken)
          m_mim->instantiateImages ();

        return broken;
      }

    void on_image_chosen (std::string path)
      {
        m_umgr->add (new MaskedImageUndoAction_Set (m_mim, m_target_filename));
        m_target_filename = path;
        load_image ();
        update_panel ();

        m_tick.disconnect ();
        show_image ();
        if (m_mim->getNumberOfFrames () > 1)
          {
            m_frame_label->set_visible (true);
            m_tick = Glib::signal_timeout ().connect
              (sigc::bind_return
               (sigc::bind
                (sigc::mem_fun (*this, &MaskedImageEditorDialog::on_tick),
                 true), true), 500);
          }
        else
          m_frame_label->set_visible (false);

      }

    void on_tick (bool incr)
      {
        guint32 num_frames = m_mim->getNumberOfFrames ();

        show_image ();
        if (num_frames > 1)
          m_frame_label->set_text
            (String::ucompose ("Frame %1", m_active_frame + 1));
        else
          m_frame_label->set_text ("");

        if (incr)
          {
            m_active_frame++;
            if (m_active_frame >= num_frames)
              m_active_frame = 0;
          }
      }

    void update_panel()
      {
        update_imagebutton_label (m_target_filename);
        show_image ();
        m_clear_button->set_visible (m_mim->getImage () != NULL);
      }

    void show_image ()
      {
        m_white_image->set_paintable (nullptr);
        m_green_image->set_paintable (nullptr);
        m_yellow_image->set_paintable (nullptr);
        m_light_blue_image->set_paintable (nullptr);
        m_red_image->set_paintable (nullptr);
        m_dark_blue_image->set_paintable (nullptr);
        m_orange_image->set_paintable (nullptr);
        m_black_image->set_paintable (nullptr);
        m_neutral_image->set_paintable (nullptr);
        m_white_image->set_visible (false);
        m_green_image->set_visible (false);
        m_yellow_image->set_visible (false);
        m_light_blue_image->set_visible (false);
        m_red_image->set_visible (false);
        m_dark_blue_image->set_visible (false);
        m_orange_image->set_visible (false);
        m_black_image->set_visible (false);
        m_neutral_image->set_visible (false);

        if (m_mim->getImage () == NULL)
          return;

        Vector<int> dim = m_mim->getImageDimensions ();
        if (dim.x * MAX_PLAYERS  > MAX_IMAGES_WIDTH)
          {
            dim.x = MAX_IMAGES_WIDTH / MAX_PLAYERS;
            dim.y = m_mim->getImage ()->get_height() *
              (double)((double)dim.x / (double)m_mim->getImage ()->get_width());
          }

        for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
          {
            if (m_one_color)
              {
                if (Shield::Color (i) != m_color)
                  continue;
              }
            Gtk::Picture *image = NULL;
            switch (i)
              {
              case Shield::WHITE:
                image = m_white_image;
                break;

              case Shield::GREEN:
                image = m_green_image;
                break;

              case Shield::YELLOW:
                image = m_yellow_image;
                break;

              case Shield::LIGHT_BLUE:
                image = m_light_blue_image;
                break;

              case Shield::RED:
                image = m_red_image;
                break;

              case Shield::DARK_BLUE:
                image = m_dark_blue_image;
                break;

              case Shield::ORANGE:
                image = m_orange_image;
                break;

              case Shield::BLACK:
                image = m_black_image;
                break;

              case Shield::NEUTRAL: 
                image = m_neutral_image;
                break;

              default:
                break;
              }

            if (m_shieldset == NULL)
              {
                Glib::ustring n = m_shield_theme_combobox->get_active_text ();
                m_shieldset = Shieldsetlist::instance ()->get (n, 0);
              }

            PixMask *p = NULL;
            switch (m_mim->getMaskOrientation ())
              {
              case TarFileMaskedImage::VERTICAL_MASK:
                p = m_mim->applyMask (m_active_frame,
                                      m_shieldset->getColors (i));
                break;
              }
            if (p)
              {
                image->set_css_classes ({"border-image"});
                image->set_paintable (p->to_texture ());
                image->set_visible (true);
              }
            delete p;
          }
      }

    void open_file (std::string p)
      {
        if (FileFilter ({".png", ".svg"}).has_invalid_ext (p))
          {
            Glib::ustring msg =
              _("Bad file extension, "
                "only PNG and SVG are supported");
            auto dialog = LwDialog::alert (msg, p);
            dialog->choose
              (*this,
               [dialog] (auto result2)
               {
                 dialog->choose_finish (result2);
                 return;
               });
          }
        else
          {
            if (PixMask::checkFormat (p) == false)
              {
                Glib::ustring msg = _("Couldn't make sense of the image");
                auto dialog = LwDialog::alert (msg, p);
                dialog->choose
                  (*this,
                   [dialog] (auto result2)
                   {
                     dialog->choose_finish (result2);
                     return;
                   });
              }
            else
              {
                if (m_mim->checkDimension (p) == false)
                  {
                    Glib::ustring msg = _("Bad dimensions in image");
                    auto dialog = LwDialog::alert (msg, p);
                    dialog->choose
                      (*this,
                       [dialog] (auto result2)
                       {
                         dialog->choose_finish (result2);
                         return;
                       });
                  }
                else
                  {
                    Glib::ustring filename = p;
                    if (filename.empty ())
                      return;
                    on_image_chosen (filename);
                  }
              }
          }
      }
};
#endif
