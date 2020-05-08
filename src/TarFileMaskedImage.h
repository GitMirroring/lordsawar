// Copyright (C) 2020 Ben Asselstine
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
#ifndef TAR_FILE_MASKED_IMAGE_H
#define TAR_FILE_MASKED_IMAGE_H

#include <gtkmm.h>
#include "vector.h"

class PixMask;
class Tar_Helper;
class TarFile;
class XML_Helper;
class Player;

/**
 * A helper class to handle image files in tar files that have masks.
 *
 * A mask is a second half of an image that contains white with transparency
 * to be overlaid on the main image in the player's colour.
 *
 * This encompasses files like army images, selectors, ships, planted
 * standards and more.
 */
class TarFileMaskedImage
{
public:

    enum MaskOrientation
      {
        // The mask is to the right of the image
        HORIZONTAL_MASK = 0,

        // The mask is below the image
        VERTICAL_MASK,
      };

  //! Default Constructor
  /**
   * @param o the orientation of the mask
   * we get the tar file from the load method later on.
   *
   */
  TarFileMaskedImage (MaskOrientation o = VERTICAL_MASK);

  //! Copy Constructor
  TarFileMaskedImage (const TarFileMaskedImage &i);

  //! Destructor
  ~TarFileMaskedImage ();

  //! Return the orientation of the mask
  MaskOrientation getMaskOrientation () const {return orientation;}

  //! Return the number of images
  guint32 getNumberOfFrames () const {return frames.size ();}

  //! Return the basename of the image (archive member in tar file)
  Glib::ustring getName () const {return name;}

  //! Get the whole image as loaded from the file
  PixMask *getBackingImage () {return image;}

  //! Return an image by index
  PixMask *getImage (guint32 i) const {return i < frames.size () ? frames[i].first : NULL;}

  //! Return the first image
  PixMask *getImage () const {return frames.empty () ? NULL :frames[0].first;}

  //! Apply the mask onto the image in the player's colour
  /**
   * @return a pointer to a new PixMask that must be deleted.
   */
  PixMask *applyMask (Player *p) const;
  PixMask *applyMask (Gdk::RGBA colour) const;

  //! Apply the mask onto the image at the given index in the player's colour
  /**
   * @return a pointer to a new PixMask that must be deleted.
   */
  PixMask *applyMask (guint32 i, Player *p) const;
  PixMask *applyMask (guint32 i, Gdk::RGBA colour) const;

  //! Return all of the images
  std::vector<PixMask*> getImages () const
    { std::vector<PixMask*> o; for (auto f : frames) o.push_back (f.first); return o; }

  //! Return the dimensions that the images are scaled to
  Vector<int> getScaledImageDimensions () const {return scale_dimension;}

  //! Return the dimensions of the images in the backing image
  Vector<int> getImageDimensions () const  {return dimension;}

  //! Set the basename of the image (archive member in the tar file)
  void setImageName (Glib::ustring n) {name = n;}

  //! Set the name of the archive member in the tar file that holds the image
  void setName (Glib::ustring n) {name = n;}

  //! Set the opened tar file
  void setTarFile (Tar_Helper *t) {tarfile = t;}

  //! Read the data tag from an opened xml file and put it in our name member
  void load_name (XML_Helper *helper, Glib::ustring data_tag);

  //! Load an image from a tar file, with bname already provided
  /**
   * @param          t the unopened tar file
   * @param bname    the archive member containing the image
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
   * @param bname    the archive member containing the image
   * @return         true if something went wrong
   */
  bool load (Tar_Helper *t, Glib::ustring bname);

  //! Load an image named bname from the tar file
  /**
   * @param          bname the archive member containing the image
   * @return         true if something went wrong
   */
  bool load (Glib::ustring bname);

  //! Load an image named filename from disk
  /**
   * @param filename the file holding the image
   * @return         true if something went wrong
   */
  bool loadFromFile (Glib::ustring filename);

  //! Process the backing image into a set of images and masks
  void instantiateImages (Vector<int> scale_to_dimension = Vector<int>(-1,-1));

  //! Destroy the images
  void uninstantiateImages ();

  //! Destroy the images and optionally clear the basename
  void clear (bool clear_name = true);

  //! Destroy just the backing image
  void dropBackingImage ();

  //! copy this to DEST in T
  bool copy (TarFile *t, TarFileMaskedImage *dest);

  //! uninstantiate all images named NAME in IMAGES
  static void uninstantiate (Glib::ustring name, std::vector<TarFileMaskedImage*> images);
private:

  //! The orientation of the masked image
  MaskOrientation orientation;

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

  //! The backing image, the whole image as loaded from file_on_disk.
  PixMask *image;

  //! in vertical orientation, how many frames there are.
  guint32 calculated_number_of_frames;

  //! The parts of the backing image cut up into images and masks
  /**
   * the first member of the pair is the image, and the second is the mask.
   */
  std::vector<std::pair<PixMask*, PixMask *> > frames;

  //! Process a horizontally masked image
  void instantiateVertical  ();

  //! Process a vertically masked image
  void instantiateHorizontal ();

  //! Overlay the mask on the image in the given colour.
  /**
   * @return a new pixmask that must be deleted.
   */
  PixMask* applyMask(PixMask* image, PixMask* mask, Gdk::RGBA colour) const;
};

#endif
