//  Copyright (C) 2010, 2012, 2014, 2015, 2020, 2021 Ben Asselstine
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
#include <sstream>

#include "tilestyle-organizer-dialog.h"

#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "Tile.h"
#include "ImageCache.h"
#include "timing.h"
#include "font-size.h"
#include "tilestyle-organizer-actions.h"

#define method(x) sigc::mem_fun(*this, &TileStyleOrganizerDialog::x)

TileStyleOrganizerDialog::TileStyleOrganizerDialog(Gtk::Window &parent, Tile *tile)
 : LwEditorDialog(parent, "tilestyle-organizer-dialog.ui")
{
    umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
    umgr->execute ().connect (method (executeAction));
    d_tile = tile;
    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("LordsawarTilestyleType", Gtk::TARGET_SAME_APP));
    
    xml->get_widget("categories_iconview", categories_iconview);
    xml->get_widget("category_iconview", category_iconview);
    xml->get_widget("unsorted_iconview", unsorted_iconview);
    xml->get_widget("category_label", category_label);
    xml->get_widget("unsorted_label", unsorted_label);
    xml->get_widget("undo_button", undo_button);
    undo_button->signal_activate ().connect (method (on_undo_activated));
    xml->get_widget("redo_button", redo_button);
    redo_button->signal_activate ().connect (method (on_redo_activated));

    unsorted_iconview->signal_button_press_event().connect
      (method(on_unsorted_mouse_button_event));
    category_iconview->signal_button_press_event().connect
      (method(on_category_mouse_button_event));
    categories_list = Gtk::ListStore::create (categories_columns);
    categories_iconview->set_model(categories_list); 
    categories_iconview->set_pixbuf_column(categories_columns.image);
    categories_iconview->enable_model_drag_dest(targets, Gdk::ACTION_MOVE);
    categories_iconview->signal_drag_data_received().connect
      (method(on_categories_drop_drag_data_received));
    fill_in_categories();
    category_list = Gtk::ListStore::create (tilestyle_columns);
    category_iconview->set_model(category_list); 
    category_iconview->set_pixbuf_column(tilestyle_columns.image);
    category_iconview->set_text_column(tilestyle_columns.name);
    category_iconview->enable_model_drag_dest(targets, Gdk::ACTION_MOVE);
    category_iconview->enable_model_drag_source(targets, Gdk::BUTTON1_MASK, Gdk::ACTION_MOVE);
    category_iconview->signal_drag_data_get().connect
      (sigc::hide(sigc::hide(method(on_category_drag_data_get))));
    category_iconview->signal_drag_data_received().connect
      (method(on_category_drop_drag_data_received));

    category_iconview->signal_drag_begin().connect
      (sigc::bind(sigc::hide<0>(method(on_drag_begin)), category_iconview));
    unsorted_list = Gtk::ListStore::create (tilestyle_columns);
    unsorted_iconview->set_model(unsorted_list); 
    unsorted_iconview->set_pixbuf_column(tilestyle_columns.image);
    unsorted_iconview->set_text_column(tilestyle_columns.name);
    unsorted_iconview->enable_model_drag_dest(targets, Gdk::ACTION_MOVE);
    unsorted_iconview->drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
    unsorted_iconview->enable_model_drag_source(targets, Gdk::BUTTON1_MASK, Gdk::ACTION_MOVE);
    unsorted_iconview->signal_drag_data_get().connect
      (sigc::hide(sigc::hide(method(on_unsorted_drag_data_get))));
    unsorted_iconview->signal_drag_data_received().connect
      (method(on_unsorted_drop_drag_data_received));
    unsorted_iconview->signal_drag_begin().connect
      (sigc::bind(sigc::hide<0>(method(on_drag_begin)), unsorted_iconview));

    if (d_tile->front())
      if (d_tile->front()->front())
        if (d_tile->front()->front()->getImage())
          {
          unsorted_iconview->property_item_width() =
            d_tile->front()->front()->getImage()->get_width();
          category_iconview->property_item_width() =
            d_tile->front()->front()->getImage()->get_width();
          }

    fill_category(TileStyle::UNKNOWN);
    categories_iconview->select_path(Gtk::TreeModel::Path("0"));
    inhibit_select = false;
    d_changed = false;
    update ();
}
      
void TileStyleOrganizerDialog::on_category_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &drag_context, Gtk::SelectionData &data)
{
  drag_context->get_source_window()->show();
  Glib::ustring s;
  std::list<TileStyle*> st = get_selected_category_tilestyles();
  if (st.empty() == true)
    return;
  for (std::list<TileStyle*>::iterator i = st.begin(); i != st.end(); ++i)
    s += String::ucompose("0x%1 ", TileStyle::idToString((*i)->getId()));
  data.set(data.get_target(), 8, (const guchar*)s.c_str(), strlen(s.c_str()));
}

void TileStyleOrganizerDialog::on_unsorted_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &drag_context, Gtk::SelectionData &data)
{
  drag_context->get_source_window()->show();
  Glib::ustring s;
  std::list<TileStyle*> st = get_selected_unsorted_tilestyles();
  if (st.empty() == true)
    return;
  for (std::list<TileStyle*>::iterator i = st.begin(); i != st.end(); ++i)
    s += String::ucompose("0x%1 ", TileStyle::idToString((*i)->getId()));
  data.set(data.get_target(), 8, (const guchar*)s.c_str(), strlen(s.c_str()));
}

int TileStyleOrganizerDialog::get_selected_category()
{
  typedef std::vector<Gtk::TreeModel::Path> type_list_paths;
  type_list_paths selected = categories_iconview->get_selected_items();
  if (!selected.empty())
    {
      const Gtk::TreeModel::Path &path = *selected.begin();
      Gtk::TreeModel::iterator iter = categories_list->get_iter(path);
      Gtk::TreeModel::Row row = *iter;
      return row[categories_columns.type];
    }
  return -1;
}

std::list<TileStyle*> TileStyleOrganizerDialog::get_selected_category_tilestyles ()
{
  std::list<TileStyle*> styles;
  typedef std::vector<Gtk::TreeModel::Path> paths;
  paths selected = category_iconview->get_selected_items();
  if (!selected.empty())
    {
      for (paths::iterator i = selected.begin(); i != selected.end(); ++i)
        {
          Gtk::TreeModel::iterator iter = category_list->get_iter(*i);
          Gtk::TreeModel::Row row = *iter;
          styles.push_back(row[tilestyle_columns.style]);
        }
    }
  return styles;
}

std::list<TileStyle*> TileStyleOrganizerDialog::get_selected_unsorted_tilestyles ()
{
  std::list<TileStyle*> styles;
  typedef std::vector<Gtk::TreeModel::Path> paths;
  paths selected = unsorted_iconview->get_selected_items();
  if (!selected.empty())
    {
      for (paths::iterator i = selected.begin(); i != selected.end(); ++i)
        {
          Gtk::TreeModel::iterator iter = unsorted_list->get_iter(*i);
          Gtk::TreeModel::Row row = *iter;
          styles.push_back(row[tilestyle_columns.style]);
        }
    }
  return styles;
}

void TileStyleOrganizerDialog::fill_in_categories()
{
    categories_list->clear();
    for (guint32 i = TileStyle::LONE; i < TileStyle::UNKNOWN; i++)
      add_category (i);
}

void TileStyleOrganizerDialog::add_category(guint32 type)
{
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeModel::Row row = *(categories_list->append());
  double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;
  PixMask *p = gc->getDefaultTileStylePic(type, 80)->copy ();
  int font_size = FontSize::getInstance()->get_height ();
  double new_height = font_size * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height
    (p, new_height);
  PixMask::scale (p, new_width, new_height);
  row[categories_columns.image] = p->to_pixbuf ();
  delete p;
  row[categories_columns.name] = TileStyle::getTypeName(TileStyle::Type(type));
  row[categories_columns.type] = type;
}

void TileStyleOrganizerDialog::empty_category()
{
  category_label->set_text("");
}

void TileStyleOrganizerDialog::add_tilestyle(Glib::RefPtr<Gtk::ListStore> list, TileStyle *tilestyle)
{
  Gtk::TreeModel::Row row = *(list->append());
  double ratio = EDITOR_DIALOG_TILE_PIC_FONTSIZE_MULTIPLE;

  PixMask *p = tilestyle->getImage ()->copy ();
  int font_size = FontSize::getInstance()->get_height ();
  double new_height = font_size * ratio;
  int new_width =
    ImageCache::calculate_width_from_adjusted_height
    (p, new_height);
  PixMask::scale (p, new_width, new_height);
  row[tilestyle_columns.image] = p->to_pixbuf();
  delete p;
  row[tilestyle_columns.name] = "0x" + 
    TileStyle::idToString(tilestyle->getId());
  row[tilestyle_columns.style] = tilestyle;
}

void TileStyleOrganizerDialog::fill_category(guint32 type)
{
  Gtk::Label *label;
  guint32 count = d_tile->countTileStyles(TileStyle::Type(type));
  Glib::ustring items = String::ucompose(_("(%1 items)"), count);
  Glib::ustring markup;
  Glib::RefPtr<Gtk::ListStore> list;
  if (type == TileStyle::UNKNOWN)
    {
      label = unsorted_label;
      Glib::ustring unsorted = _("Unsorted TileStyles");
      markup = "<b>" + unsorted + "</b> " + items;
      list = unsorted_list;
    }
  else
    {
      label = category_label;
      markup = "<b>" + TileStyle::getTypeName(TileStyle::Type(type)) + 
                        " TileStyles</b> " + items;
      list = category_list;
    }
  label->set_markup(markup);
  list->clear();
  std::list<TileStyle*> styles = d_tile->getTileStyles(TileStyle::Type(type));
  for (std::list<TileStyle*>::iterator i = styles.begin(); i != styles.end();
       ++i)
    add_tilestyle(list, *i);
}

void TileStyleOrganizerDialog::on_category_selected()
{
  int type = get_selected_category();
  if (type != -1)
    fill_category(TileStyle::Type(type));
  else
    empty_category();
}

void TileStyleOrganizerDialog::on_categories_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  (void) c;
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      std::string idstr;
      std::istringstream ids(selection_data.get_data_as_string());
      std::list<std::pair<guint, TileStyle::Type> > changes;
      while (1)
        {
          idstr = "";
          ids >> idstr;
          if (idstr.empty() == true)
            break;
          char *end = NULL;
          unsigned long int id = 0;
          id = strtoul (idstr.c_str(), &end, 0);
          //which category?
          int nx = 0, ny = 0;
          categories_iconview->convert_widget_to_bin_window_coords(x, y, nx, ny);
          const Gtk::TreeModel::Path &path = 
            categories_iconview->get_path_at_pos(nx, ny);
          Gtk::TreeModel::iterator iter = categories_list->get_iter(path);
          Gtk::TreeModel::Row row = *iter;
          TileStyle *style = d_tile->getTileStyle(id);
          if (style)
            {
              guint32 type = row[categories_columns.type];
              changes.push_back
                (std::pair<guint32, TileStyle::Type>(style->getId (),
                                                     style->getType ()));
              style->setType(TileStyle::Type(type));
              update ();
            }
        }
      if (changes.empty () == false)
        {
          TileStyleOrganizerAction_Move *action =
            new TileStyleOrganizerAction_Move (changes);
          umgr->add (action);
        }
    }

  context->drag_finish (false, false, time);
  d_changed = true;
}

void TileStyleOrganizerDialog::on_category_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  (void) a;
  (void) b;
  (void) c;
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      std::string idstr;
      std::istringstream ids(selection_data.get_data_as_string());
      std::list<std::pair<guint, TileStyle::Type> > changes;
      while (1)
        {
          idstr = "";
          ids >> idstr;
          if (idstr.empty() == true)
            break;
          char *end = NULL;
          unsigned long int id = 0;
          id = strtoul (idstr.c_str(), &end, 0);
          TileStyle *style = d_tile->getTileStyle(id);
          if (style)
            {
              int type = get_selected_category();
              if (type != -1)
                {
                  changes.push_back
                    (std::pair<guint32, TileStyle::Type>(id,
                                                         style->getType ()));
                  style->setType(TileStyle::Type(type));
                  update ();
                }
            }
        }
      if (changes.empty () == false)
        {
          TileStyleOrganizerAction_Move *action =
            new TileStyleOrganizerAction_Move (changes);
          umgr->add (action);
        }
    }
  context->drag_finish (false, false, time);
  d_changed = true;
}

void TileStyleOrganizerDialog::on_unsorted_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  (void) a;
  (void) b;
  (void) c;
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      std::string idstr;
      std::istringstream ids(selection_data.get_data_as_string());
      std::list<std::pair<guint, TileStyle::Type> > changes;
      while (1)
        {
          idstr = "";
          ids >> idstr;
          if (idstr.empty() == true)
            break;
          char *end = NULL;
          unsigned long int id = 0;
          id = strtoul (idstr.c_str(), &end, 0);
          TileStyle *style = d_tile->getTileStyle(id);
          if (style)
            {
              changes.push_back
                (std::pair<guint32, TileStyle::Type>(id, style->getType ()));
              style->setType(TileStyle::UNKNOWN);
              update ();
            }
        }
      if (changes.empty () == false)
        {
          TileStyleOrganizerAction_Move *action =
            new TileStyleOrganizerAction_Move (changes);
          umgr->add (action);
        }
    }
  context->drag_finish (false, false, time);
  d_changed = true;
}
    
/**
 * This is how we're getting multiple drag to work.
 * 1. we remember the last multiple selection. (last_multiple_selection)
 * 2. we remember the time we made it. (time_of_last_selection)
 * 3. when we begin a drag, the icon we are dragging gets selected, thereby
 * nullifying the multiple selection.  it's a good thing we already have it
 * remembered!  so we check to see if that happened really recently, and if it
 * did, we select what we had selected before the drag started.
 * 4. we take special care to forget the multiple selections later on.
 * 5. we deselect the other iconview when we make a selection, and then take
 * special care not to let a zero selection mess up our state.
 */
void TileStyleOrganizerDialog::on_drag_begin(Gtk::IconView *iconview)
{
  Glib::TimeVal now;
  now.assign_current_time();
  now.subtract(time_of_last_selection);
  double secs = now.as_double();
  if (secs < 0.5)
    {
      inhibit_select = true;
      for (unsigned int i = 0; i < last_multiple_selection.size(); i++)
        iconview->select_path(last_multiple_selection[i]);
      inhibit_select = false;
    }
  last_multiple_selection.clear();
}

void TileStyleOrganizerDialog::on_selection_made(Gtk::IconView *iconview)
{
  if (inhibit_select)
    return;
  if (iconview->get_selected_items().size() == 0)
    return;
  if (iconview->get_selected_items().size() > 0)
    time_of_last_selection.assign_current_time();
  if (iconview->get_selected_items().size() > 1)
    {
      selection_timeout_handler.disconnect();
      last_multiple_selection = iconview->get_selected_items();
    }
  else
    {
      selection_timeout_handler.disconnect();
      selection_timeout_handler = Timing::instance().register_timer (method(expire_selection), 1000);
    }
  if (iconview == unsorted_iconview)
    category_iconview->unselect_all();
  else
    unsorted_iconview->unselect_all();
}

bool TileStyleOrganizerDialog::expire_selection()
{
  last_multiple_selection.clear();
  return Timing::STOP;
}

bool TileStyleOrganizerDialog::run()
{
  dialog->run();
  return d_changed;
}

void TileStyleOrganizerDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void TileStyleOrganizerDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void TileStyleOrganizerDialog::update ()
{
  disconnect_signals ();
  on_category_selected();
  fill_category(TileStyle::UNKNOWN);
  connect_signals ();
}

void TileStyleOrganizerDialog::connect_signals ()
{
  connections.push_back
    (categories_iconview->signal_selection_changed().connect
     (method(on_category_selected)));
  connections.push_back
    (category_iconview->signal_selection_changed().connect
     (sigc::bind(method(on_selection_made), category_iconview)));
  connections.push_back
    (unsorted_iconview->signal_selection_changed().connect
      (sigc::bind(method(on_selection_made), unsorted_iconview)));
}

void TileStyleOrganizerDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

UndoAction *TileStyleOrganizerDialog::executeAction (UndoAction *action2)
{
  TileStyleOrganizerAction *action =
    dynamic_cast<TileStyleOrganizerAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
    case TileStyleOrganizerAction::MOVE:
        {
          TileStyleOrganizerAction_Move *a =
            dynamic_cast<TileStyleOrganizerAction_Move*>(action);
          out = new TileStyleOrganizerAction_Move
            (d_tile->getAllTileStyleTypes ());
          for (auto j : a->getTileStyleTypes ())
            {
              TileStyle *ts = d_tile->getTileStyle (j.first);
              ts->setType (j.second);
            }
        }
      break;
    }
  return out;
}

void TileStyleOrganizerDialog::popup (GdkEventButton *e, Gtk::IconView *i)
{
  Gtk::Menu *menu = manage(new Gtk::Menu);
    {
      Gtk::MenuItem *item = manage(new Gtk::MenuItem(_("Undo")));
      item->signal_activate().connect (method(on_undo_activated));
      item->set_sensitive (umgr->undoEmpty () == false);
      item->show();
      menu->add(*item);
    }
    {
      Gtk::MenuItem *item = manage(new Gtk::MenuItem(_("Redo")));
      item->signal_activate().connect (method(on_redo_activated));
      item->set_sensitive (umgr->redoEmpty () == false);
      item->show();
      menu->add(*item);
    }
    {
      Gtk::MenuItem *item = manage(new Gtk::MenuItem(_("Select All")));
      item->signal_activate().connect
        (sigc::bind (method(on_select_all_activated), i));
      item->show();
      menu->add(*item);
    }
  menu->accelerate (*dialog);
  menu->popup_at_pointer (reinterpret_cast<const GdkEvent*>(e));
}

bool TileStyleOrganizerDialog::on_unsorted_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
    return true;	// useless event

  popup (e, unsorted_iconview);

  return true;
}

void TileStyleOrganizerDialog::on_select_all_activated (Gtk::IconView *iconview)
{
  iconview->select_all ();
}

bool TileStyleOrganizerDialog::on_category_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
    return true;	// useless event

  popup (e, category_iconview);

  return true;
}
