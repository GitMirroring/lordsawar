// Copyright (C) 2020, 2021 Ben Asselstine
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
#ifndef TAR_FILE_IMAGE_H
#define TAR_FILE_IMAGE_H

#include <gtkmm.h>
#include "vector.h"
#include "PixMask.h"

class PixMask;
class Tar_Helper;
class TarFile;
class XML_Helper;

/**
 * A helper class to handle image files in tar files.
 *
 * This encompasses files that don't have a color mask like castles,
 * roads, fog, the next turn image, the bag image, and more.
 *
 * sometimes these images can have many frames, and we know exactly
 * how many to expect beforehand.
 *
 * also, sometimes there's just a single image.
 */
class TarFileImage
{
public:

  //! Default Constructor
  TarFileImage (guint32 number_of_frames, PixMask::DimensionType d);

  //! Copy Constructor
  TarFileImage (const TarFileImage &i);

  //! Destructor
  ~TarFileImage ();

  //! Return the number of images
  guint32 getNumberOfFrames () const {return number_of_frames;}

  //! Return the basename of the image (archive member in tar file)
  Glib::ustring getName () const {return name;}

  //! Get the whole image as loaded from the file
  PixMask *getBackingImage () {return image;}

  //! Return an image by index
  PixMask *getImage (guint32 i) const {return i < frames.size () ? frames[i] : NULL;}

  //! Return the first image
  PixMask *getImage () const {return frames.empty () ? NULL :frames[0];}

  //! Return all of the images
  std::vector<PixMask*> getImages () const {return frames;}

  //! Return the dimensions that the images are scaled to
  Vector<int> getScaledImageDimensions () const {return scale_dimension;}

  //! Return the dimensions of the images in the backing image
  Vector<int> getImageDimensions () const  {return dimension;}

  //! Does the image in file F have the correct dimensions for this?
  bool checkDimension (Glib::ustring f);

  //! Set the name of the archive member in the tar file that holds the image
  void setName (Glib::ustring n) {name = n;}

  //! Set the opened tar file
  void setTarFile (Tar_Helper *t) {tarfile = t;}

  //! Read the data tag from an opened xml file and put it in our name member
  void load_name (XML_Helper *helper, Glib::ustring data_tag);

  //! Load an image from a tar file, with bname already provided
  /**
   * @param t        the unopened tar file (shieldset file, etc.)
   * @param bname    the archive member of the image
   * @return         true if something went wrong
   */
  bool load (TarFile *t, Glib::ustring bname);

  //! Load an image from the tar file, with bname already set by setName
  /**
   * @return         true if something went wrong
   */
  bool load ();

  //! Load an image from the tar file t, already provided bname
  /**
   * @param t        the opened tar file
   * @return         true if something went wrong
   */
  bool load (Tar_Helper *t);

  //! Load an image named bname from the tar file t
  /**
   * @param t        the opened tar file
   * @param bname    archive member of the image
   * @return         true if something went wrong
   */
  bool load (Tar_Helper *t, Glib::ustring bname);

  //! Load an image named bname from the tar file
  /**
   * @param bname    archive member of the image
   * @return         true if something went wrong
   */
  bool load (Glib::ustring bname);

  //! Load an image named filename from disk
  /**
   * @param filename file on disk of the image
   * @return         true if something went wrong
   */
  bool loadFromFile (Glib::ustring filename);

  //! Process the backing image into a set of images
  void instantiateImages (Vector<int> scale_to_dimension = Vector<int>(-1,-1));

  //! Destroy the images
  void uninstantiateImages ();

  //! Destroy the images and optionally clear the basename
  void clear (bool clear_name = true);

  //! Destroy just the backing image
  void dropBackingImage ();

  //! Uninstantiate all images named NAME in images
  static void uninstantiate (Glib::ustring name, std::vector<TarFileImage*> images);

private:

  //! The opened tar file
  Tar_Helper *tarfile;

  //! The basename of the archive member holding the image
  Glib::ustring name;

  //! When we extract the file from the tar file, this is where it is
  Glib::ustring file_on_disk;

  //! What we want to scale the images to
  Vector<int> scale_dimension;

  //! The original dimensions of the images in the backing image
  Vector<int> dimension;

  //! What general size the image should be
  PixMask::DimensionType dimension_type;

  //! The backing image, the whole image as loaded from file_on_disk.
  PixMask *image;

  //! in vertical orientation, how many frames there are.
  guint32 number_of_frames;

  //! The parts of the backing image cut up into images,
  std::vector<PixMask *> frames;
};

#endif
