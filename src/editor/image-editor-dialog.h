//  Copyright (C) 2009, 2010, 2011, 2014, 2015, 2020, 2021, 2026 Ben Asselstine
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
#ifndef IMAGE_EDITOR_DIALOG_H
#define IMAGE_EDITOR_DIALOG_H
#include "file-filter.h"
#include "image-undo-actions.h"
#include "file-label.h"
class ImageEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "image-editor.ui";
      }

    ImageEditorDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_magnify_button = load <Gtk::ToggleButton> ("magnify_button");
        m_close_button = load <Gtk::Button> ("close_button");
        m_clear_button = load <Gtk::Button> ("clear_button");
        m_image_button = load <Gtk::Button> ("imagebutton");
        m_picture = load <Gtk::Picture> ("picture");
        m_frame_label = load <Gtk::Label> ("frame_label");
        m_override_default = false;
      }

    ~ImageEditorDialog ()
      {
        delete m_umgr;
        for (auto f : m_frames)
          delete f;
        delete m_im;
        m_tick.disconnect ();
        disconnect_signals ();
      }

    Glib::ustring get_filename ()
      {
        return m_target_filename;
      }

    bool is_changed ()
      {
        return m_target_filename != m_orig_target_filename;
      }

    bool install_file (TarFile *t, TarFileImage *im, Glib::ustring filename,
                       Glib::ustring &err)
      {
        std::string newname;
        bool success = false;
        if (m_orig_target_filename.empty () == true)
          success = t->addFileInCfgFile (filename, newname, err);
        else
          success =
            t->replaceFileInCfgFile (m_orig_target_filename, filename, newname,
                                     err);
        im->setName (newname);
        im->load (t, newname);
        im->instantiateImages ();
        return success;
      }

    bool uninstall_file (TarFile *t, TarFileImage *im, Glib::ustring &err)
      {
        im->clear ();
        return t->removeFileInCfgFile (m_orig_target_filename, err);
      }

    void set_override ()
      {
        m_override_default = true;
      }

    void setup (TarFileImage *im)
      {
        m_num_frames = im->getNumberOfFrames ();
        if (m_num_frames <= 1)
          m_frame_label->set_visible (false);

        if (m_magnify_button)
          {
            auto width = 0;
            if (im->getImage ())
              width = im->getImage ()->get_width ();

            m_magnify_button->set_visible (width <= 40);
          }

        m_active_frame = 0;
        m_target_filename = im->getName ();
        m_orig_target_filename  = m_target_filename;
        m_im = new TarFileImage (*im);

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        set_response (m_clear_button, Gtk::ResponseType::REJECT);

        m_clear_button->set_visible (!im->getName ().empty ());

        m_image_filelabel = Gtk::make_managed<FileLabel> (m_image_button);
        m_image_button->set_tooltip_text (_("Select a new image"));

        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &ImageEditorDialog::execute_action));

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
                     m_picture->set_can_shrink (false);
                     m_picture->set_content_fit (Gtk::ContentFit::CONTAIN);
                     m_picture->set_size_request (width * scale_factor,
                                                  height * scale_factor);
                     m_magnify_button->set_sensitive (false);
                   }
               }
           });

        if (im->getBackingImage () != NULL)
          for (guint32 i = 0; i < m_num_frames; i++)
            m_frames.push_back (im->getImage (i)->copy ());
        update ();
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_clear_button;
    Gtk::ToggleButton *m_magnify_button;
    Gtk::Button *m_image_button;
    FileLabel *m_image_filelabel;
    Gtk::Picture *m_picture;
    Gtk::Label *m_frame_label;

    UndoMgr *m_umgr;
    guint32 m_num_frames;
    guint32 m_active_frame;
    Glib::ustring m_target_filename;
    Glib::ustring m_orig_target_filename;
    bool m_override_default;
    TarFileImage *m_im;
    std::vector<PixMask*> m_frames;
    sigc::connection m_tick;
    std::list<sigc::connection> m_connections;

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
                if (m_im->checkDimension (p) == false)
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

                    m_umgr->add
                      (new ImageUndoAction_Set
                       (m_target_filename, m_frames));
                    m_target_filename = filename;

                    update_imagebutton_label (m_target_filename);
                    load_frames (m_target_filename);

                    show_image ();
                  }
              }
          }
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        auto action = dynamic_cast<ImageUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case ImageUndoAction::SET:
              {
                auto a = dynamic_cast<ImageUndoAction_Set*>(action);
                out = new ImageUndoAction_Set (m_target_filename, m_frames);
                m_target_filename = a->get_file ();
                m_tick.disconnect ();
                for (auto f : m_frames)
                  delete f;
                m_frames.clear ();
                for (auto f : a->get_frames ())
                  m_frames.push_back (f->copy ());
              } 
            break;
          }
        return out;
      }

    void update_imagebutton_label (Glib::ustring filename)
      {
        Glib::ustring f = File::get_basename (filename, true);
        if (f.empty () == false)
          m_image_filelabel->set_label (f);
        else
          {
            if (m_override_default)
              m_image_filelabel->set_override ();
            m_image_filelabel->set_label ("");
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

    void show_image ()
      {
        if (m_tick.connected ())
          m_tick.disconnect ();

        m_picture->set_css_classes ({});
        m_picture->set_paintable (nullptr);

        if (m_frames.empty () == false)
          {
            m_picture->set_css_classes ({"border-image"});
            guint32 w = m_frames.front ()->get_width ();
            guint32 h = m_frames.front ()->get_height ();
            m_picture->set_size_request (w, h);
            on_tick (false);
            m_tick = Glib::signal_timeout ().connect
              (sigc::bind_return
               (sigc::bind
                (sigc::mem_fun (*this, &ImageEditorDialog::on_tick), true),
                true), 500);
          }
      }

    void on_tick (bool incr)
      {
        if (m_num_frames > 1)
          m_frame_label->set_text
            (String::ucompose ("Frame %1", m_active_frame + 1));
        else
          m_frame_label->set_text ("");
        m_picture->set_paintable (m_frames[m_active_frame]->to_texture ());
        if (incr)
          {
            m_active_frame++;
            if (m_active_frame >= m_num_frames)
              m_active_frame = 0;
          }
      }

    bool load_frames (Glib::ustring filename)
      {
        bool broken = false;
        for (auto f : m_frames)
          delete f;
        if (filename == "")
          return true;
        m_frames = disassemble_row (filename, m_num_frames, broken);
        if (!broken)
          {
            m_clear_button->set_visible (true);
          }
        return broken;
      }

};
#endif
