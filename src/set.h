// Copyright (C) 2009, 2014 Ben Asselstine
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

#pragma once
#ifndef SET_H
#define SET_H
#include "File.h"
#include "defs.h"
#include "xmlhelper.h"
#include "tarfile.h"

//! Base class for Armyset, Tileset, Shieldset, and Cityset objects.
class Set: public TarFile
{
public:
    Set(Glib::ustring file_extension, guint32 id, Glib::ustring name, guint32 ts);
    Set(Glib::ustring file_extension, XML_Helper* helper, Glib::ustring dir);
    ~Set() {};
    Set(const Set &s);

    //! Returns the width and height in pixels of a square on the map.
    guint32 getTileSize() const {return d_tileSize * (double)d_scale;}
    guint32 getUnscaledTileSize() const {return d_tileSize;}

    void setTileSize(guint32 tile_size) {d_tileSize = tile_size;}

    //! Get the unique identifier for this set.
    /**
     * Analagous to the <d_id> XML entity in the set 
     * configuration file.
     */
    guint32 getId() const {return d_id;}

    //! Set the unique identifier for this set.
    void setId(guint32 id) {d_id = id;}

    //! Returns the name of the set.
    /** 
     * Analagous to the <d_name> XML entity in the set 
     * configuration file.
     *
     * @return The name or an empty string on error.
     */
    Glib::ustring getName() const {return _(d_name.c_str());}

    //! Set the name of the set.
    /**
     * @note This method is only used in the scenario editor.
     */
    void setName(Glib::ustring name) {d_name = name;}

    //! Get the copyright holders for this set.
    Glib::ustring getCopyright() const {return d_copyright;};

    //! Set the copyright holders on the set.
    void setCopyright(Glib::ustring copy) {d_copyright = copy;};

    //! Get the license of this set.
    Glib::ustring getLicense() const {return d_license;};

    //! Returns the description of the set.
    Glib::ustring getInfo() const {return _(d_info.c_str());}

    //! Set the license for this set.
    void setLicense(Glib::ustring license) {d_license = license;};

    //! Set the description of the set.
    /**
     * @note This method is only used in the scenario editor.
     */
    void setInfo(Glib::ustring info) {d_info = info;}

    bool save(XML_Helper *helper) const;

    //!Get the zoom level.
    double get_scale () const {return d_scale;};

    //!Set the zoom level.
    void set_scale (double d) {d_scale = d;};
private:

    //! The unique Id of this set.
    /**
     * This Id is unique among all other sets.
     * It is analgous to <d_id> in the set configuration files.
     */
    guint32 d_id;

    //! The name of the set.
    /**
     * This value appears in game configuration dialogs.
     * It is analgous to <d_name> in a set configuration file.
     */
    Glib::ustring d_name;

    //! The set has these copyright holders.
    Glib::ustring d_copyright;

    //! The license of the set.
    Glib::ustring d_license;

    //! The description of the set.
    /**
     * Equates to the <d_info> XML entity in the set
     * configuration file.
     * This value is not used.
     */
    Glib::ustring d_info;

    //! The size of the graphic tiles in the Tileset.
    /**
     * Equates to the tileset.d_tilesize XML entity in the tileset
     * configuration file.
     * It represents the size in pixels of the width and height of tile
     * imagery onscreen (but then it is multipled by scale).
     */
    guint32 d_tileSize;

    //! The zoom level of tiles.  A number between 0 and 1.
    double d_scale;
};

#endif
