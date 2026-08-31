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
#ifndef TILESTYLES_DIALOG_H
#define TILESTYLES_DIALOG_H
#include "tile-set.h"
#include "tile.h"
#include "tile-style-set.h"
#include "tile-style.h"
#include "tilestyles-undo.h"
#include "lw-column.h"
class TileStyleSetRow: public Glib::Object
{
public:
    TileStyleSet *m_tilestyleset;

    static Glib::RefPtr<TileStyleSetRow> create (TileStyleSet *s)
      {
        return
          Glib::make_refptr_for_instance<TileStyleSetRow> (new TileStyleSetRow (s));
      }

protected:
    TileStyleSetRow (TileStyleSet *s)
      : m_tilestyleset (s)
      {
      }
};

class TileStylesDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "tilestyles.ui";
      }

    TileStylesDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        set_size_request (650, 450);
        m_close_button = load <Gtk::Button> ("close_button");
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
        m_category_gridview = load <Gtk::GridView> ("category_gridview");
        m_unsorted_gridview = load <Gtk::GridView> ("unsorted_gridview");
        m_combobox_box = load <Gtk::Box> ("combobox_box");
      }

    ~TileStylesDialog ()
      {
        delete m_umgr;
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

    void setup (Tileset *tileset, Tile *tile)
      {
        auto idx = get_index (tileset, tile);
        if (idx < 0)
          return;
        m_index = idx;

        m_tileset = tileset;
        set_title
          (String::ucompose (_("Tile Styles for %1"), get_tile ()->getName ()));

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_file_store = Gio::ListStore<TileStyleSetRow>::create ();
        m_file_selection_model = Gtk::SingleSelection::create (m_file_store);
        m_treeview->set_model (m_file_selection_model);

        m_category_combo = Gtk::make_managed<LwCombo> ();
        for (guint32 i = TileStyle::LONE; i < TileStyle::UNKNOWN; i++)
          m_category_combo->append
            (TileStyle::getTypeName (TileStyle::Type (i)));
        m_combobox_box->append (*m_category_combo);

        m_category_combo->signal_changed ().connect
          ([this] ()
           {
             fill_category_gridview ();
           });

        m_add_button->set_icon_name ("list-add-symbolic");
        m_remove_button->set_icon_name ("list-remove-symbolic");

        m_add_button->signal_clicked ().connect
          ([this] ()
           {
             LwDialog::open
               (*this,
                String::ucompose (_("Select a %1 tile image"),
                                  get_tile ()->getName ()), FileFilter::PNG,
                [this] (std::string path)
                {

                  if (!m_tileset->addTileStyleSet (get_tile (), path))
                    {
                      Glib::ustring msg = _("Couldn't load image");
                      auto dialog = LwDialog::alert (msg);
                      dialog->choose
                        (*this,
                         [this, dialog] (auto result)
                         {
                           dialog->choose_finish (result);
                           return;
                         });
                    }
                  else
                    {
                      auto a = new TileStylesUndoAction_AddSet (m_tileset);
                      Glib::ustring err;
                      std::string outfile;
                      if (m_tileset->addFileInCfgFile (path, outfile, err))
                        {
                          m_umgr->add (a);
                          auto set = get_tile ()->back ();
                          set->setName (outfile);
                          m_file_store->append (TileStyleSetRow::create (set));
                          scroll_treeview_to_bottom ();
                          update ();
                        }
                      else
                        {
                          delete a;
                          Glib::ustring msg = _("Couldn't add image");
                          auto dialog = LwDialog::alert (msg, err);
                          dialog->choose
                            (*this,
                             [this, dialog] (auto result)
                             {
                               dialog->choose_finish (result);
                               return;
                             });
                        }
                    }
                });
           });

        m_remove_button->signal_clicked ().connect
          ([this] ()
           {
             Glib::ustring err;
             auto a = new TileStylesUndoAction_RemoveSet (m_tileset);
             auto set = get_selected_tilestyleset ();
             if (!m_tileset->removeFileInCfgFile (set->getName (), err))
               {
                 delete a;
                 Glib::ustring msg = _("Couldn't remove image");
                 auto dialog = LwDialog::alert (msg, err);
                 dialog->choose
                   (*this,
                    [this, dialog] (auto result)
                    {
                      dialog->choose_finish (result);
                      return;
                    });
                 return;
               }
             m_umgr->add (a);
             get_tile ()->remove (set);
             m_file_store->remove (m_file_selection_model->get_selected ());
             delete set;
             fill_gridviews ();
             update_buttons ();
           });

        m_file_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             scroll_gridviews_to_top ();
             fill_gridviews ();
           });

        setup_tilestyleset_column ();

        m_unsorted_model = Gtk::StringList::create ();
        m_unsorted_selection_model =
          Gtk::MultiSelection::create (m_unsorted_model);
        m_unsorted_gridview->set_model (m_unsorted_selection_model);
        setup_gridview (m_unsorted_gridview, m_unsorted_model,
                        m_unsorted_selection_model);

        m_category_model = Gtk::StringList::create ();
        m_category_selection_model =
          Gtk::MultiSelection::create (m_category_model);
        m_category_gridview->set_model (m_category_selection_model);
        setup_gridview (m_category_gridview, m_category_model,
                        m_category_selection_model);

        fill_treeview ();
        fill_gridviews ();
        
        setup_undo ();
      }

private:
    Gtk::Button *m_close_button;
    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_file_selection_model;
    Glib::RefPtr<Gio::ListStore<TileStyleSetRow>> m_file_store;
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    Gtk::GridView *m_category_gridview;
    Gtk::GridView *m_unsorted_gridview;
    Gtk::Box *m_combobox_box;
    LwCombo *m_category_combo;

    size_t m_index;
    Tileset *m_tileset;

    Glib::RefPtr<Gtk::StringList> m_unsorted_model;
    Glib::RefPtr<Gtk::MultiSelection> m_unsorted_selection_model;

    Glib::RefPtr<Gtk::StringList> m_category_model;
    Glib::RefPtr<Gtk::MultiSelection> m_category_selection_model;

    UndoMgr *m_umgr;

    void setup_tilestyleset_column ()
      {
        LwColumn::setup_text_column<TileStyleSetRow>
          (m_treeview, "tilestyleset_label", true, Gtk::Justification::LEFT,
           _("Image Files"),
           [] (const auto& row)
           {
             return row->m_tilestyleset->getName ();
           });
      }

    void fill_treeview ()
      {
        m_file_store->remove_all ();
        for (auto set : *get_tile ())
          m_file_store->append (TileStyleSetRow::create (set));
      }

    TileStyleSet *get_selected_tilestyleset ()
      {
        auto item = m_file_selection_model->get_selected_item ();
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<TileStyleSetRow>(item);
        return row->m_tilestyleset;
      }

    void update_buttons ()
      {
        m_remove_button->set_sensitive (m_file_store->get_n_items () > 0);
      }

    void scroll_treeview_to_bottom ()
      {
        guint n = m_file_store->get_n_items ();
        Glib::signal_idle ().connect_once
          ([this, n] ()
           {
             m_file_selection_model->set_selected (n - 1);
             m_treeview->scroll_to (n - 1);
           });
      }

    void scroll_gridviews_to_top ()
      {
        if (m_category_model->get_n_items () > 0)
          m_category_gridview->scroll_to (0);
        if (m_unsorted_model->get_n_items () > 0)
          m_unsorted_gridview->scroll_to (0);
      }

    void fill_category_gridview ()
      {
        while (m_category_model->get_n_items () > 0)
          m_category_model->remove (0);

        auto set = get_selected_tilestyleset ();
        if (!set)
          return;

        auto t = get_selected_category ();
        for (auto ts : *set)
          if (ts->getType () == t)
            m_category_model->append (TileStyle::idToString (ts->getId ()));
      }

    void fill_unsorted_gridview ()
      {
        while (m_unsorted_model->get_n_items () > 0)
          m_unsorted_model->remove (0);

        auto set = get_selected_tilestyleset ();
        if (!set)
          return;

        for (auto ts : *set)
          if (ts->getType () == TileStyle::UNKNOWN)
            m_unsorted_model->append (TileStyle::idToString (ts->getId ()));
      }

    void fill_gridviews ()
      {
        fill_unsorted_gridview ();
        fill_category_gridview ();
      }

    void setup_gridview (Gtk::GridView *gridview, Glib::RefPtr<Gtk::StringList> model,
                         Glib::RefPtr<Gtk::MultiSelection> selection)
      {
        auto factory = Gtk::SignalListItemFactory::create ();

        factory->signal_setup ().connect
          ([] (const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto image = Gtk::make_managed<Gtk::Image>();
             image->set_pixel_size (64);
             image->set_margin (6);
             item->set_child (*image);
           }, false);

        factory->signal_bind ().connect
          ([this] (const Glib::RefPtr<Gtk::ListItem>& item)
           {
             auto str_obj =
               std::dynamic_pointer_cast<Gtk::StringObject>(item->get_item ());

             if (!str_obj)
               return;

             auto image = dynamic_cast<Gtk::Image*>(item->get_child ());
             if (!image)
               return;

             char *end = NULL;
             auto idstr = "0x" + str_obj->get_string ();
             auto id = strtoul (idstr.c_str (), &end, 0);
             auto tilestyle = get_tile ()->getTileStyle (id);

             image->set (tilestyle->getImage ()->to_pixbuf ());
             image->set_tooltip_text (idstr);
           }, false);

        gridview->set_factory (factory);

        auto drag_source = Gtk::DragSource::create ();
        drag_source->set_actions (Gdk::DragAction::MOVE);
        drag_source->signal_prepare ().connect
          ([this, model, selection, drag_source] (double, double) -> Glib::RefPtr<Gdk::ContentProvider>
           {
             auto bitset = selection->get_selection ();
             if (bitset->get_size () == 0)
               return Glib::RefPtr<Gdk::ContentProvider>();

             std::vector<std::string> items;
             std::vector<Glib::RefPtr<Gdk::Texture>> textures;

             for (auto it = bitset->begin (); it != bitset->end (); ++it)
               {
                 guint pos = *it;
                 auto str_obj =
                   std::dynamic_pointer_cast<Gtk::StringObject>(model->get_object (pos));
                 if (!str_obj)
                   continue;

                 std::string str = str_obj->get_string ();
                 items.push_back (str);

                 char *end = NULL;
                 auto idstr = "0x" + str;
                 auto id = strtoul (idstr.c_str (), &end, 0);
                 auto tilestyle = get_tile ()->getTileStyle (id);
                 textures.push_back (tilestyle->getImage ()->to_texture ());
               }

             if (items.empty ())
               return Glib::RefPtr<Gdk::ContentProvider>();

             auto icon = build_drag_icon (textures);
             if (icon)
               drag_source->set_icon (icon, 0, 0);

             std::string payload = join_strings (items, ",");

             Glib::Value<Glib::ustring> value;
             value.init (G_TYPE_STRING);
             value.set (Glib::ustring (payload));

             return Gdk::ContentProvider::create (value);
           }, false);

        gridview->add_controller (drag_source);

        auto drop_target = Gtk::DropTarget::create (G_TYPE_STRING,
                                                    Gdk::DragAction::MOVE);
        drop_target->signal_drop ().connect
          ([this, model] (const Glib::ValueBase& value, double, double) -> bool
           {
             const auto& str_val =
               static_cast<const Glib::Value<Glib::ustring>&>(value);
             std::string payload = str_val.get ();

             auto items = split_string (payload, ',');

             // Reject drops on the same grid
             for (const auto& str : items)
               {
                 for (guint i = 0; i < model->get_n_items (); ++i)
                   {
                     auto so =
                       std::dynamic_pointer_cast<Gtk::StringObject>(model->get_object (i));
                     if (so)
                       {
                         std::string model_str = so->get_string ();
                         if (model_str == str)
                           return false;
                       }
                   }
               }

             Glib::RefPtr<Gtk::StringList> other =
               (model == m_unsorted_model) ? m_category_model : m_unsorted_model;

             std::vector<guint> positions;
             for (const auto& str : items)
               {
                 for (guint i = 0; i < other->get_n_items (); ++i)
                   {
                     auto so =
                       std::dynamic_pointer_cast<Gtk::StringObject>(other->get_object (i));
                     if (so)
                       {
                         std::string other_str = so->get_string ();
                         if (other_str == str)
                           {
                             positions.push_back (i);
                             break;
                           }
                       }
                   }
               }

             std::sort (positions.begin (), positions.end (), std::greater<guint>());
             for (auto pos : positions)
               {
                 other->remove (pos);
               }

             std::vector<guint32> ids;
             for (const auto& str : items)
               {
                 char *end = NULL;
                 auto idstr = "0x" + str;
                 auto id = strtoul (idstr.c_str (), &end, 0);
                 ids.push_back (id);
               }

             TileStyle::Type target;
             if (model == m_unsorted_model)
               target = TileStyle::UNKNOWN;
             else
               target = get_selected_category ();

             m_umgr->add (new TileStylesUndoAction_Type (m_tileset));

             for (const auto& str : items)
                model->append (Glib::ustring (str));

             for (auto id : ids)
               {
                 auto tilestyle = get_tile ()->getTileStyle (id);
                 if (tilestyle)
                   tilestyle->setType (target);
               }

             return true;
           }, false);

        gridview->add_controller (drop_target);
      }

    static Glib::RefPtr<Gdk::Texture> build_drag_icon (const std::vector<Glib::RefPtr<Gdk::Texture>>& textures)
      {
        if (textures.empty ())
          return {};

        if (textures.size () == 1)
          return textures[0];

        int w = 64, h = 64;
        int cols = std::min (3, static_cast<int>(textures.size ()));
        int rows = (textures.size () + cols - 1) / cols;
        int total_w = w * cols + 4 * (cols - 1);
        int total_h = h * rows + 4 * (rows - 1);

        auto surface =
          Cairo::ImageSurface::create (Cairo::ImageSurface::Format::ARGB32,
                                       total_w, total_h);
        auto cr = Cairo::Context::create (surface);

        cr->set_source_rgba (0, 0, 0, 0);
        cr->paint ();

        for (size_t i = 0; i < textures.size (); ++i)
          {
            int col = i % cols;
            int row = i / cols;
            int x = col * (w + 4);
            int y = row * (h + 4);

            cr->set_source_rgba (0.5, 0.7, 0.9, 0.9);
            cr->rectangle (x, y, w, h);
            cr->fill ();

            cr->set_source_rgba (1, 1, 1, 1);
            cr->set_font_size (14);
            auto text = std::to_string (i + 1);
            Cairo::TextExtents extents;
            cr->get_text_extents (text, extents);
            cr->move_to (x + (w - extents.width) / 2,
                         y + (h + extents.height) / 2);
            cr->show_text (text);
          }

        surface->flush ();

        auto bytes =
          Glib::Bytes::create (surface->get_data (),
                               surface->get_stride () * total_h);
        return
          Gdk::MemoryTexture::create
          (total_w, total_h,
           Gdk::MemoryTexture::Format::B8G8R8A8_PREMULTIPLIED, bytes,
           surface->get_stride ());
      }

    static std::string join_strings (const std::vector<std::string>& strings,
                                     const std::string& delimiter)
      {
        std::string result;
        for (size_t i = 0; i < strings.size (); ++i)
          {
            if (i > 0)
              result += delimiter;
            result += strings[i];
          }
        return result;
      }

    static std::vector<std::string> split_string (const std::string& s,
                                                  char delimiter)
      {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream token_stream (s);
        while (std::getline (token_stream, token, delimiter))
          {
            tokens.push_back (token);
          }
        return tokens;
      }

    TileStyle::Type get_selected_category ()
      {
        return TileStyle::Type (m_category_combo->get_active_row_number ());
      }

    UndoAction* execute_action (UndoAction *action2)
      {
        auto action = dynamic_cast<TileStylesUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {

          case TileStylesUndoAction::ADD_TILESTYLESET:
              {
                auto a = dynamic_cast<TileStylesUndoAction_AddSet*>(action);
                out = new TileStylesUndoAction_AddSet (m_tileset);
                reload_tileset (a);
              }
            break;

          case TileStylesUndoAction::REMOVE_TILESTYLESET:
              {
                auto a = dynamic_cast<TileStylesUndoAction_RemoveSet*>(action);
                out = new TileStylesUndoAction_RemoveSet (m_tileset);
                reload_tileset (a);
              }
            break;

          case TileStylesUndoAction::TYPE:
              {
                auto a = dynamic_cast<TileStylesUndoAction_Type*>(action);
                out = new TileStylesUndoAction_Type (m_tileset);
                reload_tileset (a);
              }
            break;
          }

        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &TileStylesDialog::execute_action));

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

      }

    void update ()
      {
        fill_treeview ();
        fill_gridviews ();
      }

    void reload_tileset (TileStylesUndoAction_Save *action)
      {
        Glib::ustring olddir = m_tileset->getDirectory ();
        Glib::ustring oldname =
          File::get_basename (m_tileset->getConfigurationFile (true));
        Glib::ustring oldext = m_tileset->getExtension ();

        m_tileset->clean_tmp_dir ();
        delete m_tileset;
        m_tileset = new Tileset (*(action->get_tileset ()));
        m_tileset->setLoadTemporaryFile ();

        m_tileset->setDirectory (olddir);
        m_tileset->setBaseName (oldname);
        m_tileset->setExtension (oldext);
        fill_treeview ();
        update ();
      }

    Tile *get_tile ()
      {
        return (*m_tileset)[m_index];
      }

    ssize_t get_index (Tileset *tileset, Tile *tile)
      {
        auto it = std::find (tileset->begin (), tileset->end (), tile);
        if (it != tileset->end ())
          return std::distance (tileset->begin (), it);
        return -1;
      }
};
#endif

