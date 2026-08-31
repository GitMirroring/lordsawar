#include "editor-map-widget.h"
#include <random>
#include "game-map.h"
#include "pixmask.h"
#include "bridge.h"
#include "stone.h"
#include "port.h"
#include "signpost.h"
#include "road.h"
#include "bridge-list.h"
#include "stone-list.h"
#include "port-list.h"
#include "signpost-list.h"
#include "road-list.h"
#include "temple-list.h"
#include "temple.h"
#include "ruin-list.h"
#include "ruin.h"
#include "city-list.h"
#include "city.h"
#include "stack.h"
#include "stack-list.h"
#include "player.h"
#include "player-list.h"
#include "tile-set.h"
#include "fog-map.h"
#include "item.h"
#include "stack-tile.h"
#include "tar-file-image.h"
#include "create-scenario.h"
#include "army-set-list.h"

using namespace std;
EditorMapWidget::EditorMapWidget ()
  : m_offset_x (0.0), m_offset_y (0.0),
    m_drag_start_x (0.0), m_drag_start_y (0.0),
    m_scale (1.0), m_grid_toggled (false),
    m_cursor (ImageCache::CursorType::POINTER),
    m_mouse_state (NONE), m_left_shift_down (false),
    m_right_shift_down (false), m_left_control_down (false),
    m_right_control_down (false),
    m_current_tile (Vector<int> (-1,-1))
{
  set_name ("editor-map-widget");
  set_hexpand (true);
  set_vexpand (true);

  auto drag = Gtk::GestureDrag::create ();
  drag->signal_drag_begin ().connect
    ([this](double start_x, double start_y)
     {
       if (m_mouse_state != NONE)
         return;
       (void) start_x;
       (void) start_y;
       m_drag_start_x = m_offset_x;
       m_drag_start_y = m_offset_y;
     });
  drag->signal_drag_update ().connect
    ([this] (double offset_x_delta, double offset_y_delta)
     {
       if (m_mouse_state != NONE)
         return;
       m_offset_x = m_drag_start_x + offset_x_delta;
       m_offset_y = m_drag_start_y + offset_y_delta;
       clamp_offset ();
       m_view_changed.emit (to_logical_extents ());
       queue_draw ();
     });
  add_controller (drag);

  auto motion = Gtk::EventControllerMotion::create ();
  motion->signal_motion ().connect
    ([this] (double x, double y)
     {
       static Vector<int> prev_tile = Vector<int>(-1, -1);
       auto tile = to_logical_coords (x, y);
       if (prev_tile != tile)
         {
           m_current_tile = tile;
           struct MouseMotionEvent *ev = &m_mouse_motion;
           ev->pos = Vector<int> (std::floor (x), std::floor (y));
           mouse_motion_event (*ev);
           m_pointing_at_new_tile.emit (tile);
         }
     });

  motion->signal_leave ().connect
    ([this] ()
     {
       m_redraw_without_cursor = true;
       m_mouse_motion.pressed[0] = false; //a hack
       queue_draw ();
     });
  add_controller (motion);

  auto xscroll = Gtk::EventControllerScroll::create ();
  xscroll->set_flags (Gtk::EventControllerScroll::Flags::BOTH_AXES);
  xscroll->signal_scroll ().connect
    ([this] (double dx, double dy)
     {
       m_offset_x -= dx;
       m_offset_y -= dy;

       clamp_offset ();
       m_view_changed.emit (to_logical_extents ());
       queue_draw ();

       return true;
     }, false);
  add_controller (xscroll);

  auto key_controller = Gtk::EventControllerKey::create ();
  key_controller->signal_key_pressed ().connect
    ([this](guint keyval, guint, Gdk::ModifierType) -> bool
     {
       update_key_state (keyval, true);
       return false;
     }, false);
  add_controller (key_controller);

  key_controller = Gtk::EventControllerKey::create ();
  key_controller->signal_key_pressed ().connect
    ([this](guint keyval, guint, Gdk::ModifierType) -> bool
     {
       update_key_state (keyval, false);
       return false;
     }, false);

  auto click = Gtk::GestureClick::create ();
  click->signal_pressed ().connect
    ([this] (int n, double x, double y)
     {
       struct MouseButtonEvent ev = to_input_event (n, x, y, true);
       struct MouseMotionEvent *evmotion = &m_mouse_motion;
       evmotion->pressed[ev.button] = true;
       mouse_button_event (ev);
     });

  click->signal_released ().connect
    ([this] (int n, double x, double y)
     {
       (void) n;
       struct MouseButtonEvent ev = to_input_event (n, x, y, false);
       struct MouseMotionEvent *evmotion = &m_mouse_motion;
       evmotion->pressed[ev.button] = false;
       mouse_button_event (ev);
     });
  add_controller (click);

  m_pointer = UNKNOWN;
}

EditorMapWidget::~EditorMapWidget ()
{
}

void EditorMapWidget::size_allocate_vfunc (int width, int height, int baseline)
{
  Gtk::Widget::size_allocate_vfunc (width, height, baseline);
  m_size_allocated.emit (width, height);
}

void EditorMapWidget::center_on_smallmap_pos (int x, int y)
{
  int view_width = get_width ();
  int view_height = get_height ();
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  m_offset_x = (view_width / 2.0) - (x * scaled_tile) - (scaled_tile / 2.0);
  m_offset_y = (view_height / 2.0) - (y * scaled_tile) - (scaled_tile / 2.0);

  clamp_offset ();
  m_view_changed.emit (to_logical_extents ());
  queue_draw ();
}

LwRectangle
EditorMapWidget::to_logical_extents ()
{
  if (!GameMap::getTileset ())
    return LwRectangle (0, 0, 1, 1);
  int width = get_width ();
  int height = get_height ();

  double scaled_tile = GameMap::getTileset ()->getTileSize () * m_scale;
  int start_col = static_cast<int> (-m_offset_x / scaled_tile);
  int start_row = static_cast<int> (-m_offset_y / scaled_tile);
  //int end_col = start_col + (width / scaled_tile) + 2;
  //int end_row = start_row + (height / scaled_tile) + 2;
  return LwRectangle (start_col, start_row,
                      (width / scaled_tile) + 2,
                      (height / scaled_tile) + 2);
}

Vector<int>
EditorMapWidget::to_logical_coords (double x, double y)
{
  double scaled_tile = GameMap::instance ()->getTileSize () * m_scale;

  double nx = (-m_offset_x + x) / scaled_tile;
  double ny = (-m_offset_y + y) / scaled_tile;
  return Vector<int>(std::floor (nx), std::floor (ny));
}

void EditorMapWidget::clamp_offset ()
{
  if (!GameMap::getTileset ())
    return;
  double scaled_tile = GameMap::getTileset ()->getTileSize () * m_scale;

  double min_offset_x = get_width () - (GameMap::getWidth () * scaled_tile);
  double min_offset_y = get_height () - (GameMap::getHeight () * scaled_tile);

  //welcome to the big suckola, you are here
    {

      /*
       * the problem is that centering on a tile doesn't make it get to the edges.
       * here if we're close the edge we just clamp to it.
       */
      if (m_offset_x > -scaled_tile)
        m_offset_x = 0;
      if (m_offset_y > -scaled_tile)
        m_offset_y = 0;
      if (m_offset_x < min_offset_x + scaled_tile)
        m_offset_x = min_offset_x;
      if (m_offset_y < min_offset_y + scaled_tile)
        m_offset_y = min_offset_y;
    }

  m_offset_x = clamp (m_offset_x, min_offset_x, 0.0);
  m_offset_y = clamp (m_offset_y, min_offset_y, 0.0);
}

void EditorMapWidget::snapshot_vfunc (const Glib::RefPtr<Gtk::Snapshot>& snapshot)
{
  if (!GameMap::getTileset ())
    return;
  int width = get_width ();
  int height = get_height ();

  Gdk::RGBA bg_color;
  bg_color.set_rgba (0.05, 0.05, 0.1, 1.0);
  graphene_rect_t bounds;
  graphene_rect_init (&bounds, 0, 0, width, height);
  snapshot->append_color (bg_color, &bounds);

  graphene_rect_t clip_bounds;
  graphene_rect_init (&clip_bounds, 0, 0, width, height);
  snapshot->push_clip (&clip_bounds);

  double scaled_tile = GameMap::getTileset ()->getTileSize () * m_scale;
  int start_col = static_cast<int>(-m_offset_x / scaled_tile);
  int start_row = static_cast<int>(-m_offset_y / scaled_tile);
  int end_col = start_col + (width / scaled_tile) + 2;
  int end_row = start_row + (height / scaled_tile) + 2;

  start_col = std::max (0, start_col);
  start_row = std::max (0, start_row);
  end_col = std::min (GameMap::getWidth (), end_col);
  end_row = std::min (GameMap::getHeight (), end_row);

  for (int row = start_row; row < end_row; ++row)
    {
      for (int col = start_col; col < end_col; ++col)
        {
          double x = m_offset_x + col * scaled_tile;
          double y = m_offset_y + row * scaled_tile;

          x = std::floor (x * 256.0) / 256.0;
          y = std::floor (y * 256.0) / 256.0;

          graphene_rect_t dest_rect;
          graphene_rect_init (&dest_rect, x, y,
                              ceil (scaled_tile + 0.5),
                              ceil (scaled_tile + 0.5));

          auto texture =
            Gdk::Texture::create_for_pixbuf
            (lookup_tile_graphics (Vector<int> (col, row))->to_pixbuf ());

          snapshot->append_texture (texture, &dest_rect);
        }
    }

    auto cr = snapshot->append_cairo (&bounds);
    after_draw (cr);
    snapshot->pop ();
}

PixMask * EditorMapWidget::lookup_tile_graphics (Vector<int> tile)
{
  guint32 tilesize = GameMap::instance ()->getTileSize ();
  ImageCache *gc = ImageCache::instance ();
  int tile_style_id = GameMap::instance ()->getTile (tile)->getTileStyleId ();
  int fog_type_id = FogMap::NONE;

  bool has_bag = false;
  guint32 bag_player_id = 0;
  bool has_standard = false;
  guint32 player_standard_id = 0;
  int stack_size = -1;
  int stack_player_id = -1;
  int army_type_id = -1;
  bool has_ship = false;
  bool has_tower = false;
  auto building_type = GameMap::instance ()->getTile (tile)->getBuilding ();
  Vector<int> building_tile = Vector<int>(-1,-1);
  int building_subtype = -1;
  int building_player_id = -1;
  int stone_type = -1;

  if (fog_type_id == FogMap::ALL)
    {
      //short circuit.  the tile is completely fogged.
      return gc->getTilePic (tile_style_id, fog_type_id, has_bag, bag_player_id,
                             has_standard, player_standard_id, stack_size,
                             stack_player_id, army_type_id, has_tower, has_ship,
                             building_type, building_subtype, building_tile,
                             building_player_id, tilesize, m_grid_toggled,
                             stone_type);
    }
  MapBackpack *backpack = GameMap::instance ()->getTile (tile)->getBackpack ();
  if (backpack && backpack->empty () == false)
    {
      bool standard_planted = false;
      Item *flag = backpack->getFirstPlantedItem ();
      if (flag)
	standard_planted = true;

      if (standard_planted && flag)
	{
	  has_standard = true;
	  player_standard_id = flag->getPlantableOriginalOwner ()->getId ();
          has_bag = backpack->size () > 1;
	}
      else
        {
          has_bag = true;
          bag_player_id = backpack->getOwnerId ();
        }
    }

  Stack *stack = GameMap::getStrongestStack (tile);
  if (stack)
    {
      //selected stack gets drawn in gamebigmap
      if (Playerlist::getActiveplayer ()->getActivestack () != stack)
        {
          stack_player_id = stack->getOwner ()->getId ();
          Maptile *m = GameMap::instance ()->getTile (tile);
          if (stack->getFortified () == true &&
              m->getBuilding () != Maptile::CITY &&
              m->getBuilding () != Maptile::RUIN &&
              m->getBuilding () != Maptile::TEMPLE)
            has_tower = true;
          else if (stack->hasShip () == true)
            {
              has_ship = true;
              stack_size =
                GameMap::getStacks (stack->getPos ())->countNumberOfArmies
                (Playerlist::instance ()->get (stack_player_id));
              if (stack_size > 0 && (guint)stack_size > MAX_STACK_SIZE)
                stack_size = stack->size ();
              //here we show the number of armies on the tile.
              //instead of the number of armies in the stack.
              //so that stacks appear whole before we click on them.
              //and that a stack of 1 can't hide a stack of 7.
            }
          else
            {
              army_type_id = (*stack->begin ())->getTypeId ();
              stack_size = GameMap::getStacks (stack->getPos ())->countNumberOfArmies
                (Playerlist::instance ()->get (stack_player_id));
              if (stack_size > 0 && (guint)stack_size > MAX_STACK_SIZE)
                stack_size = stack->size ();
            }
        }
    }

  if (building_type != Maptile::NONE)
    {
      switch (building_type)
	{
	case Maptile::CITY:
	    {
	      City *city = GameMap::getCity (tile);
	      building_player_id = city->getOwner ()->getId ();
	      building_tile = tile - city->getPos ();
	      if (city->isBurnt ())
		building_subtype = -1;
	      else
		building_subtype = 0;
	    }
	  break;

	case Maptile::RUIN:
            {
              Ruin *ruin = GameMap::getRuin (tile);
              building_tile = tile - ruin->getPos ();
              building_subtype = ruin->getType ();
            }
	  break;

	case Maptile::TEMPLE:
	    {
	      Temple *temple = GameMap::getTemple (tile);
	      building_tile = tile - temple->getPos ();
	      building_subtype = temple->getType ();
	    }
	  break;

	case Maptile::SIGNPOST:
	    {
	      Signpost *signpost = GameMap::getSignpost (tile);
	      building_tile = tile - signpost->getPos ();
	    }
	  break;

	case Maptile::ROAD:
	    {
	      Road *road = GameMap::getRoad (tile);
	      building_tile = tile - road->getPos ();
	      building_subtype = road->getType ();
              Stone *stone = GameMap::getStone (tile);
              if (stone)
                stone_type = stone->getType ();
	    }
	  break;

	case Maptile::STONE:
	    {
              Stone *stone = GameMap::getStone (tile);
	      building_tile = tile - stone->getPos ();
	      building_subtype = stone->getType ();
              stone_type = stone->getType ();
	    }
	  break;

	case Maptile::PORT:
	    {
	      Port *port = GameMap::getPort (tile);
	      building_tile = tile - port->getPos ();
	    }
	  break;

	case Maptile::BRIDGE:
	    {
	      Bridge *bridge = GameMap::getBridge (tile);
	      building_tile = tile - bridge->getPos ();
	      building_subtype = bridge->getType ();
	    }
	  break;

	case Maptile::NONE: default:
	  break;
	}
    }
  if (GameMap::getTileset ()->getStone ()->getName ().empty () == true)
    stone_type = -1;
  return
    gc->getTilePic (tile_style_id, fog_type_id, has_bag, bag_player_id,
                    has_standard, player_standard_id, stack_size,
                    stack_player_id, army_type_id, has_tower, has_ship,
                    building_type, building_subtype, building_tile,
                    building_player_id, tilesize, m_grid_toggled, stone_type);
}

void EditorMapWidget::update_key_state (guint keyval, bool pressed)
{
  switch (keyval)
    {
    case GDK_KEY_Shift_L:
      m_left_shift_down = pressed;
      break;
    case GDK_KEY_Shift_R:
      m_right_shift_down = pressed;
      break;
    case GDK_KEY_Control_L:
      m_left_control_down = pressed;
      break;
    case GDK_KEY_Control_R:
      m_right_control_down = pressed;
      break;
    }
}

void EditorMapWidget::mouse_button_event (MouseButtonEvent e)
{
  // coming back from dialogs our tile is -1, -1
  if (m_current_tile == Vector<int>(-1, -1))
    return;

  Vector<int> tile = mouse_pos_to_tile (e.pos);
  m_current_tile = tile;

  if (e.button == MouseButtonEvent::LEFT_BUTTON &&
      e.state == MouseButtonEvent::PRESSED && m_mouse_state == NONE)
    change_map_under_cursor (e.pos);
  else if (e.button == MouseButtonEvent::LEFT_BUTTON &&
           e.state == MouseButtonEvent::RELEASED &&
           m_mouse_state == MOVE_DRAGGING && m_pointer == MOVE)
    {
      m_mouse_state = NONE;
      change_map_under_cursor (e.pos);
    }
  else if (e.button == MouseButtonEvent::LEFT_BUTTON &&
           e.state == MouseButtonEvent::RELEASED &&
           (m_pointer == TERRAIN || m_pointer == ERASE || m_pointer == STONE ||
            m_pointer == ROAD))
    {
      //m_undo_map.emit (new EditorUndoAction_Blank ());
      m_mouse_state = NONE;
    }
  else if (e.button == MouseButtonEvent::RIGHT_BUTTON &&
           e.state == MouseButtonEvent::PRESSED)
    bring_up_details (e.pos);
  return;
}

void EditorMapWidget::bring_up_details (Vector<int> mouse_pos)
{
  map_selection_seq seq;

  if (Stack* s = GameMap::getStack (m_current_tile))
    seq.push_back (s);
  if (City* c = GameMap::getCity (m_current_tile))
    seq.push_back (c);
  if (Ruin* r = GameMap::getRuin (m_current_tile))
    seq.push_back (r);
  if (Signpost* s = GameMap::getSignpost (m_current_tile))
    seq.push_back (s);
  if (Temple* t = GameMap::getTemple (m_current_tile))
    seq.push_back (t);
  if (Road* rd = GameMap::getRoad (m_current_tile))
    seq.push_back (rd);
  auto b = GameMap::instance ()->getTile (m_current_tile)->getBackpack ();
  if (b->empty () == false)
    seq.push_back (b);
  if (Stone * st = GameMap::getStone (m_current_tile))
    seq.push_back (st);

  if (!seq.empty ())
    m_objects_selected.emit (seq, mouse_pos);
}

void EditorMapWidget::change_map_under_cursor (Vector<int> mouse_position)
{
  Player* active = Playerlist::instance ()->getActiveplayer ();

  std::vector<Vector<int> > tiles = get_cursor_tiles ();
  if (tiles.size () == 0)
    return;

  Vector<int> tile = tiles.front ();
  LwRectangle changed_tiles (tile, Vector<int>(-1, -1));
  Maptile* maptile = GameMap::instance ()->getTile (tile);
  switch (m_pointer)
    {
    case POINTER:
      bring_up_details (mouse_position);
      break;

    case TERRAIN:
      m_undo_map.emit
        (new EditorUndoAction_Terrain (m_pointer_terrain,
                                       get_cursor_rectangle ()));
      changed_tiles =
        GameMap::instance ()->putTerrain (get_cursor_rectangle (),
                                          m_pointer_terrain,
                                          m_pointer_tile_style_id, true);

      if ((*GameMap::getTileset ())[m_pointer_terrain]->getType () == Tile::WATER)
        m_map_water_changed.emit ();
      break;

    case MOVE:
      if (m_moving_objects_from == Vector<int>(-1,-1))
        {
          if (GameMap::instance ()->getBuilding (tile) != Maptile::NONE ||
              GameMap::getStack (tile) != NULL ||
              GameMap::getBackpack (tile)->empty () == false)
            {
              m_moving_objects_from = tile;
              if (GameMap::getBackpack (tile)->empty () == false)
                m_moving_bag = GameMap::getBackpack (tile);
            }
        }
      else
        {
          Vector<int> from = m_moving_objects_from;
          m_moving_objects_from = Vector<int>(-1,-1);
          if (m_mouse_state == MOVE_DRAGGING)
            break;
          //here we go with the move!
          GameMap *gm = GameMap::instance ();
          if (gm->getStack (from) != NULL)
            {
              Stack *s = gm->getStack (from);
              if (!s)
                s = gm->getStack (from);
              auto enemy_stacks = gm->getEnemyStacks (tile, s->getOwner ());
              if (gm->canPutStack (s->size (), s->getOwner (), tile) == true &&
                  enemy_stacks.empty () == true)
                {
                  auto friendly_stacks =
                    gm->getFriendlyStacks (tile, s->getOwner ());
                  if (s->getPos () != tile)
                    {
                      m_undo_map.emit
                        (new EditorUndoAction_Move (LwRectangle (s->getPos ()),
                                                    LwRectangle (tile)));
                    }
                  if (friendly_stacks.empty () == true)
                    gm->moveStack (s, tile);
                  else
                    {
                      gm->moveStack (s, tile);
                      gm->groupStacks (tile, s->getOwner ());
                      //big hack here.
                      //apparently the stacktile state is all messed up after
                      //we group a stack.
                      //the signals in the game make the game state work
                      //but we don't to do all that signalling, so we cheat.
                      gm->clearStackPositions ();
                      gm->updateStackPositions ();
                      //also we need to clear the active stack to have it show.
                      auto p = gm->getStack (tile)->getOwner ();
                      p->getStacklist ()->setActivestack (NULL);
                    }
                  changed_tiles = LwRectangle (s->getPos ());
                }
            }
          else if (gm->getBackpack (from)->empty () == false)
            {
              if (gm->canDropBag (tile))
                {
                  if (m_moving_bag->getPos () != tile)
                    {
                      m_undo_map.emit
                        (new EditorUndoAction_Move
                         (LwRectangle (m_moving_bag->getPos ()),
                          LwRectangle (tile)));
                      gm->moveBackpack (m_moving_bag, tile);
                      changed_tiles = LwRectangle (tile);
                    }
                  m_moving_bag = NULL;
                }
              else
                break;
            }
          else if (gm->getBuilding (from) != Maptile::NONE)
            {
              guint32 s = gm->getBuildingSize (from);
              if (gm->canPutBuilding
                  (gm->getBuilding (from), s, tile, false) == true)
                {
                  LwRectangle r1 = gm->getBoundingBox (from);
                  LwRectangle r2 = LwRectangle (tile);
                  r2.dim = r1.dim;
                  m_undo_map.emit
                    (new EditorUndoAction_Move (r1, r2));
                  gm->moveBuilding (from, tile);
                  changed_tiles = LwRectangle (tile);
                }
              else
                {
                  if (gm->getLocation (from)->contains (tile) ||
                      LocationBox (tile, s).contains (from))
                    {
                      LwRectangle r1 = gm->getBoundingBox (from);
                      LwRectangle r2 = LwRectangle (tile);
                      r2.dim = r1.dim;
                      m_undo_map.emit (new EditorUndoAction_Move (r1, r2));
                      gm->moveBuilding (from, tile);
                      changed_tiles = LwRectangle (tile);
                    }
                }
            }
        }
      break;

    case ERASE:
        {
          auto action = new EditorUndoAction_Erase
            (GameMap::instance ()->getBoundingBox (tile));
          // check if there is a building or a stack there and remove it
          if (GameMap::instance ()->eraseTile (tile))
            {
              m_undo_map.emit (action);
              changed_tiles = LwRectangle (tile);
            }
          else
            delete action;
        }
      break;

    case STACK:
      if (GameMap::instance ()->getStack (tile) != NULL)
        {
          map_selection_seq seq;
          Stack *s = GameMap::getStack (tile);
          if (s)
            seq.push_back (s);

          if (!seq.empty ())
            m_objects_selected.emit (seq, mouse_position);
        }
      else if (GameMap::instance ()->canPutStack (1, active, tile) == true)
        {
          m_create_new_stack.emit (tile);
        }

      break;

    case CITY:
      if (GameMap::instance ()->getBuilding (tile) == Maptile::CITY)
        {
          map_selection_seq seq;
          City *c = GameMap::getCity (tile);
          if (c)
            seq.push_back (c);

          if (!seq.empty ())
            m_objects_selected.emit (seq, mouse_position);
        }
      else
        {
          GameMap *gm = GameMap::instance ();
          Cityset *cs = GameMap::getCityset ();
          // check if we can place the city
          bool city_placeable =
            gm->canPutBuilding (Maptile::CITY, cs->getCityTileWidth (), tile);
          if (city_placeable)
            {
              LwRectangle rect = LwRectangle (tile);
              rect.dim =
                Vector<int>(cs->getCityTileWidth (), cs->getCityTileWidth ());
              m_undo_map.emit (new EditorUndoAction_City (rect));
              GameMap::instance ()->putNewCity (tile);
              changed_tiles = rect;
            }
        }
      break;

    case RUIN:
      if (GameMap::instance ()->getBuilding (tile) == Maptile::RUIN)
        {
          map_selection_seq seq;
          Ruin *r = GameMap::getRuin (tile);
          if (r)
            seq.push_back (r);

          if (!seq.empty ())
            m_objects_selected.emit (seq, mouse_position);
        }
      else
        {
          GameMap *gm = GameMap::instance ();
          Cityset *cs = GameMap::getCityset ();
          // check if we can place the ruin
          bool ruin_placeable =
            gm->canPutBuilding (Maptile::RUIN, cs->getRuinTileWidth (), tile);
          if (ruin_placeable)
            {
              LwRectangle rect = LwRectangle (tile);
              rect.dim =
                Vector<int>(cs->getRuinTileWidth (), cs->getRuinTileWidth ());
              m_undo_map.emit (new EditorUndoAction_Ruin (rect));
              GameMap::instance ()->putNewRuin (tile);
              changed_tiles = rect;
            }
        }
      break;

    case TEMPLE:
      if (GameMap::instance ()->getBuilding (tile) == Maptile::TEMPLE)
        {
          map_selection_seq seq;
          Temple *t = GameMap::getTemple (tile);
          if (t)
            seq.push_back (t);

          if (!seq.empty ())
            m_objects_selected.emit (seq, mouse_position);
        }
      else
        {
          GameMap *gm = GameMap::instance ();
          Cityset *cs = GameMap::getCityset ();
          // check if we can place the temple
          bool temple_placeable =
            gm->canPutBuilding (Maptile::TEMPLE, cs->getTempleTileWidth (),
                                tile);
          if (temple_placeable)
            {
              LwRectangle rect = LwRectangle (tile);
              rect.dim =
                Vector<int>(cs->getTempleTileWidth (),
                            cs->getTempleTileWidth ());
              m_undo_map.emit (new EditorUndoAction_Temple (rect));
              GameMap::instance ()->putNewTemple (tile);
              changed_tiles = rect;
            }
        }
      break;

    case SIGNPOST:
        {
          if (GameMap::instance ()->getBuilding (tile) == Maptile::SIGNPOST)
            {
              map_selection_seq seq;
              Signpost *s = GameMap::getSignpost (tile);
              if (s)
                seq.push_back (s);

              if (!seq.empty ())
                m_objects_selected.emit (seq, mouse_position);
            }
          else
            {
              bool signpost_placeable =
                GameMap::instance ()->canPutBuilding
                (Maptile::SIGNPOST, 1, tile);
              if (!signpost_placeable)
                break;
              LwRectangle rect = LwRectangle (tile);
              m_undo_map.emit (new EditorUndoAction_Signpost (rect));
              Signpost *s = new Signpost (tile);
              GameMap::instance ()->putSignpost (s);
              changed_tiles = LwRectangle (tile);
            }
          break;
        }

    case PORT:
        {
          bool port_placeable =
            GameMap::instance ()->canPutBuilding (Maptile::PORT, 1, tile);
          if (!port_placeable)
            break;
          LwRectangle rect = LwRectangle (tile);
          m_undo_map.emit (new EditorUndoAction_Port (rect));
          Port *p = new Port (tile);
          GameMap::instance ()->putPort (p);
          changed_tiles = LwRectangle (tile);
          break;
        }

    case BRIDGE:
        {
          if (GameMap::getBridge (tile))
            {
              GameMap::instance ()->removeBridge (tile);
              Bridge *b = new Bridge (tile, tile_to_bridge_type (tile));
              GameMap::instance ()->putBridge (b);
              changed_tiles = LwRectangle (tile);
              break;
            }
          bool bridge_placeable =
            GameMap::instance ()->canPutBuilding (Maptile::BRIDGE, 1, tile);
          if (!bridge_placeable)
            break;
          LwRectangle rect = LwRectangle (tile);
          m_undo_map.emit (new EditorUndoAction_Bridge (rect));
          Bridge *b = new Bridge (tile, tile_to_bridge_type (tile));
          GameMap::instance ()->putBridge (b);
          changed_tiles = LwRectangle (tile);
          break;
        }

    case ROAD:
        {
          Maptile::Building bldg =
            GameMap::instance ()->getTile (tile)->getBuilding ();
          switch (bldg)
            {
            case Maptile::ROAD:
            case Maptile::STONE:
            case Maptile::NONE:
                {
                  bool had_stone = GameMap::getStone (tile) != NULL;
                  if (GameMap::getRoad (tile) != NULL)
                    GameMap::instance ()->removeRoad (tile);

                  LwRectangle rect = LwRectangle (tile);
                  m_undo_map.emit (new EditorUndoAction_Road (rect));
                  int type = CreateScenario::calculateRoadType (tile);
                  Road *r = new Road (tile, type);
                  GameMap::instance ()->putRoad (r);
                  if (had_stone)
                    {
                      Stone *s = new Stone (tile, Road::Type (r->getType ()));
                      GameMap::instance ()->putStone (s);
                    }
                  //make sure we have a road building tile for path calculation
                  //purposes
                  GameMap::instance ()->getTile (tile)->setBuilding
                    (Maptile::ROAD);

                  changed_tiles.pos -= Vector<int>(1, 1);
                  changed_tiles.dim = Vector<int>(3, 3);
                }
              break;
            case Maptile::CITY:
            case Maptile::RUIN:
            case Maptile::TEMPLE:
            case Maptile::SIGNPOST:
            case Maptile::PORT:
            case Maptile::BRIDGE:
              break;
            }
          break;
        }

    case BAG:
      if (maptile->getType () != Tile::WATER)
        m_bag_selected.emit (tile);
      break;

    case FLAG:
      if (maptile->getType () != Tile::WATER)
        m_flag_selected.emit (tile);
      break;

    case TILESTYLE:
      m_tilestyle_tile_selected.emit (tile, mouse_position);
      break;

    case FIGHT:
        {
          Stack *s = GameMap::getStack (tile);
          if (s)
            {
              m_stack_selected_for_battle_calculator.emit (s, mouse_position);
            }
        }
      break;

    case STONE:
        {
          if (GameMap::getStone (tile) != NULL)
            {
              map_selection_seq seq;
              seq.push_back (GameMap::getStone (tile));
              m_objects_selected.emit (seq, mouse_position);
            }
          else
            {
              auto b = GameMap::instance ()->getBuilding (tile);
              if (GameMap::getStone (tile) == NULL &&
                  is_acceptable_building_for_stone (b))
                {
                  int type = Stone::ROAD_E_AND_W_STONE_N;
                  Road *r = GameMap::getRoad (tile);
                  if (r)
                    type = Stone::getRandomType (Road::Type (r->getType ()));
                  LwRectangle rect = LwRectangle (tile);
                  m_undo_map.emit (new EditorUndoAction_Stone (rect));
                  Stone *s = new Stone (tile, type);
                  GameMap::instance ()->putStone (s);
                  //we don't want to blast away the road building type bc
                  //we use it for path calculation
                  if (b == Maptile::ROAD)
                    GameMap::instance ()->getTile (tile)->setBuilding (b);
                  changed_tiles = LwRectangle (tile);
                }
            }
        }
      break;

    case UNKNOWN:
      break;
    }

  if (changed_tiles.w > 0 && changed_tiles.h > 0)
    m_map_tiles_changed.emit (changed_tiles);

  queue_draw ();
  return ;
}

std::vector<Vector<int> > EditorMapWidget::get_cursor_tiles ()
{
  // find out which cursor tiles are within bounds
  std::vector<Vector<int> > tiles;

  for (guint32 y = 0; y < m_pointer_size; ++y)
    for (guint32 x = 0; x < m_pointer_size; ++x)
      {
        int offset = - ((m_pointer_size - 1) / 2.0);
        Vector<int> tile (x + offset, y + offset);
        tile += m_current_tile;

        if (tile.x >= 0 && tile.x < GameMap::getWidth () &&
            tile.y >= 0 && tile.y < GameMap::getHeight ())
          tiles.push_back (tile);
      }

  return tiles;
}

int EditorMapWidget::tile_to_bridge_type (Vector<int> t)
{
    // examine neighbour tiles to discover whether there's a road on them
    bool u = Roadlist::instance ()->getObjectAt (t + Vector<int>(0, -1));
    bool b = Roadlist::instance ()->getObjectAt (t + Vector<int>(0, 1));
    bool l = Roadlist::instance ()->getObjectAt (t + Vector<int>(-1, 0));
    bool r = Roadlist::instance ()->getObjectAt (t + Vector<int>(1, 0));

    // then translate this to the type
    int type = 0;
    if (!u && !b && !l && !r)
	type = 0;
    else if (u && b && l && r)
	type = 0;
    else if (!u && b && l && r)
	type = 0;
    else if (u && !b && l && r)
	type = 0;
    else if (u && b && !l && r)
	type = 1;
    else if (u && b && l && !r)
	type = 1;
    else if (u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && r)
	type = 0;
    else if (u && !b && l && !r)
	type = 0;
    else if (u && !b && !l && r)
	type = 2;
    else if (!u && b && l && !r)
	type = 0;
    else if (!u && b && !l && r)
	type = 2;
    else if (u && !b && !l && !r)
	type = 3;
    else if (!u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && !r)
	type = 0;
    else if (!u && !b && !l && r)
	type = 2;
    return type;
}

LwRectangle EditorMapWidget::get_cursor_rectangle ()
{
  // find out which cursor tiles are within bounds
  std::vector<Vector<int> > tiles;

  int offset = (m_pointer_size - 1) / 2;
  Vector<int> tile = m_current_tile - Vector<int>(offset, offset);

  return LwRectangle (tile.x, tile.y, m_pointer_size, m_pointer_size);
}

void EditorMapWidget::after_draw (Cairo::RefPtr<Cairo::Context> &cr)
{
  if (m_redraw_without_cursor)
    {
      m_redraw_without_cursor = false;
      return;
    }

  int tilesize = GameMap::instance ()->getTileSize ();
  std::vector<Vector<int> > tiles;

  if (m_current_tile == Vector<int>(-1,-1))
    return;

  // we need to draw a drawing cursor on the map
  tiles = get_cursor_tiles ();
  // draw each tile

  Gdk::RGBA move_box_color =
    Gdk::RGBA (50.0 / 256.0, 200.0 / 256.0, 50.0 / 256.0);
  Gdk::RGBA moving_box_color =
    Gdk::RGBA (250.0 / 256.0, 250.0 / 256.0, 0.0 / 256.0);

  for (auto i = tiles.begin (), end = tiles.end (); i != end; ++i)
    {
      double scaled_tile = tilesize * m_scale;
      double nx = m_offset_x + (*i).x * scaled_tile;
      double ny = m_offset_y + (*i).y * scaled_tile;

      nx = std::floor (nx * 256.0) / 256.0;
      ny = std::floor (ny * 256.0) / 256.0;
      Vector<int> pos = Vector<int> ((int)nx, (int)ny);

      PixMask *pic;

      auto p = Playerlist::getActiveplayer ();

      switch (m_pointer)
        {
        case POINTER:
          break;

        case TERRAIN:
            {
              Gdk::RGBA c (200.0 / 256.0, 200.0 / 256.0, 200.0 / 256.0);
              pic =
                ImageCache::instance ()->getBoxPic (tilesize, c, true, false,
                                                    1, true);
              pic->blit (cr->get_target (), pos);
            }
          break;

        case ERASE:
          cr->stroke ();
            {
              Gdk::RGBA c (1.0, 1.0, 1.0);
              pic =
                ImageCache::instance ()->getBoxPic (tilesize, c, true, true,
                                                    1, true);
              pic->blit (cr->get_target (), pos);
            }
          break;

        case STACK:
          pic = ImageCache::instance ()->getArmyPic
            (p->getArmyset (), 0,
             p->get_shield (), NULL, true, false);

          pic->blit (cr->get_target (), pos);
          break;

        case CITY:
          pic = ImageCache::instance ()->getCityPic
            (0, p, GameMap::instance ()->getCitysetId ());

          pic->blit (cr->get_target (), pos);
          break;

        case RUIN:
          pic = ImageCache::instance ()->getRuinPic
            (0, GameMap::instance ()->getCitysetId ());

          pic->blit (cr->get_target (), pos);
          break;

        case TEMPLE:
          pic = ImageCache::instance ()->getTemplePic
            (0, GameMap::instance ()->getCitysetId ());

          pic->blit (cr->get_target (), pos);
          break;

        case SIGNPOST:
          pic = ImageCache::instance ()->getSignpostPic ();
          pic->blit (cr->get_target (), pos);
          break;

        case ROAD:
            {
              Road *r = GameMap::getRoad (*i);
              if (r)
                pic = ImageCache::instance ()->getRoadPic (r->getType ());
              else
                pic = ImageCache::instance ()->getRoadPic
                  (CreateScenario::calculateRoadType (*i));
              pic->blit (cr->get_target (), pos);
            }
          break;

        case STONE:
            {
              Tileset *t = GameMap::getTileset ();
              Stone *s = GameMap::getStone (*i);
              if (s)
                pic = t->getStone ()->getImage (s->getType ());
              else
                {
                  Road *r = GameMap::getRoad (*i);
                  if (r)
                    pic = t->getStone ()->getImage (Stone::getRandomType
                                                    (Road::Type (r->getType ())));
                  else
                    pic = t->getStone ()->getImage
                      (Stone::ROAD_ALL_DIRECTIONS_STONES_NW_NE_SW_SE);
                }
              if (pic)
                pic->blit (cr->get_target (), pos);
            }
          break;

        case PORT:
          pic = ImageCache::instance ()->getPortPic ();
          pic->blit (cr->get_target (), pos);
          break;

        case BRIDGE:
          pic = ImageCache::instance ()->getBridgePic
            (tile_to_bridge_type (*i));
          pic->blit (cr->get_target (), pos);
          break;

        case BAG:
          pic = ImageCache::instance ()->getBagPic ();
          pic->blit (cr->get_target (), pos);
          break;

        case FLAG:
          pic = ImageCache::instance ()->getPlantedStandardPic
            (p->getArmyset (), Playerlist::getNeutral ()->get_shield ());
          pic->blit (cr->get_target (), pos);
          break;

        case TILESTYLE:
          pic = ImageCache::instance ()->getBoxPic
            (tilesize, Gdk::RGBA ("white"), false, false, 1, false);
          pic->blit (cr->get_target (), pos);
          break;

        case FIGHT:
          pic = ImageCache::instance ()->getBoxPic
            (tilesize, Gdk::RGBA ("white"), false, false, 1, false);
          pic->blit (cr->get_target (), pos);
          break;

        case MOVE:
          cr->unset_dash ();
          if (m_moving_objects_from != Vector<int>(-1,-1))
            {
              Vector<int> tile = *i;

              GameMap *gm = GameMap::instance ();
              auto from = m_moving_objects_from;
              if (gm->getStack (from) != NULL)
                {
                  Stack *s = gm->getStack (from);
                  auto enemy_stacks = gm->getEnemyStacks (tile, s->getOwner ());
                  if (gm->canPutStack (s->size (), s->getOwner (),
                                       tile) == true &&
                      enemy_stacks.empty () == true)
                    {
                      auto plist = Playerlist::instance ();
                      pic = ImageCache::instance ()->getArmyPic
                        (s->getOwner ()->getArmyset (), 0,
                         s->getOwner ()->get_shield (), NULL, true, 0);
                      pic->blit (cr->get_target (), pos);

                      Player *o = plist->getActiveplayer ();
                      plist->setActiveplayer (s->getOwner ());
                      pic = ImageCache::instance ()->getFlagPic
                        (gm->countArmyUnits (s->getPos ()),
                         s->getOwner ()->get_shield ());
                      plist->setActiveplayer (o);
                      pic->blit (cr->get_target (), pos);
                    }
                }
              else if (gm->getBackpack (from)->empty () == false)
                {
                  pic = ImageCache::instance ()->getBagPic ();
                  pic->blit (cr->get_target (), pos);
                }
              else if (gm->getBuilding (from) != Maptile::NONE)
                {
                  guint32 s = gm->getBuildingSize (from);
                  bool same = false;
                  if (gm->getLocation (from)->contains (tile) ||
                      LocationBox (tile, s).contains (from))
                    same = true;
                  if (gm->canPutBuilding
                      (gm->getBuilding (from), s, tile, false) == true ||
                      same)
                    display_moving_building (cr, from, pos);
                }

              Gdk::RGBA color (250.0 / 256.0, 250.0 / 256.0, 0.0 / 256.0);
              pic = ImageCache::instance ()->getBoxPic (tilesize, color,
                                                        true, true, 4, false);
              pic->blit (cr->get_target (), pos);
            }
          else
            {
              Gdk::RGBA color (50.0 / 256.0, 200.0 / 256.0, 50.0 / 256.0);

              pic = ImageCache::instance ()->getBoxPic (tilesize, color,
                                                        true, true, 4, false);
              pic->blit (cr->get_target (), pos);
            }
          break;

        case UNKNOWN:
          break;
        }
    }
  return;
}

void EditorMapWidget::display_moving_building (Cairo::RefPtr<Cairo::Context> c,
                                               Vector<int> src,
                                               Vector<int> dest)
{
  PixMask *pic = NULL;
  switch (GameMap::instance ()->getBuilding (src))
    {
    case Maptile::CITY:
      pic = ImageCache::instance ()->getCityPic (GameMap::getCity (src));
      break;

    case Maptile::RUIN:
      pic = ImageCache::instance ()->getRuinPic (GameMap::getRuin (src));
      break;

    case Maptile::TEMPLE:
      pic = ImageCache::instance ()->getTemplePic (GameMap::getTemple (src));
      break;

    case Maptile::SIGNPOST:
      pic = ImageCache::instance ()->getSignpostPic ();
      break;

    case Maptile::ROAD:
      pic = ImageCache::instance ()->getRoadPic (GameMap::getRoad (src));
      break;

    case Maptile::STONE:
      pic = GameMap::getTileset ()->getStone ()->getImage
        (GameMap::getStone (src)->getType ());
      break;

    case Maptile::PORT:
      pic = ImageCache::instance ()->getPortPic ();
      break;

    case Maptile::BRIDGE:
      pic = ImageCache::instance ()->getBridgePic (GameMap::getBridge (src));
      break;

    default:
      break;
    }
  if (pic)
    pic->blit (c->get_target (), dest);

  if (GameMap::instance ()->getBuilding (src) == Maptile::ROAD)
    {
      if (GameMap::getStone (src))
        {
          auto stone = GameMap::getTileset ()->getStone ()->getImage
            (GameMap::getStone (src)->getType ());
          if (stone)
            stone->blit (c->get_target (), dest);
        }
    }
}

void EditorMapWidget::mouse_motion_event (MouseMotionEvent e)
{
  Vector<int> mouse_pos = e.pos;

  if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
      m_pointer != MOVE && m_pointer != POINTER && m_pointer != STACK &&
      m_pointer != CITY && m_pointer != RUIN && m_pointer != TEMPLE &&
      m_pointer != SIGNPOST && m_pointer != BAG && m_pointer != FLAG)
    change_map_under_cursor (e.pos);

  if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
           (m_mouse_state == NONE || m_mouse_state == MOVE_DRAGGING) &&
           m_pointer == MOVE)
    m_mouse_state = MOVE_DRAGGING;
  else if (e.pressed[MouseMotionEvent::LEFT_BUTTON] &&
           (m_mouse_state == NONE || m_mouse_state == TERRAIN_DRAGGING) &&
           (m_pointer == TERRAIN || m_pointer == ROAD || m_pointer == STONE ||
            m_pointer == ERASE))
    m_mouse_state = TERRAIN_DRAGGING;

  m_prev_mouse_pos = mouse_pos;
}
                
bool EditorMapWidget::is_acceptable_building_for_stone (Maptile::Building b)
{
  switch (b)
    {
    case Maptile::NONE:
    case Maptile::ROAD:
      return true;
    default:
      return false;
    }
    
  return false;
}
