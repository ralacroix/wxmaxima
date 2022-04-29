// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2019      Gunter Königsmann <wxMaxima@physikbuch.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//  SPDX-License-Identifier: GPL-2.0+

/*! \file
  This file declares the class Image

  Image stores compressed images and handles scaling and uncompressing them.
*/

#ifndef SVGBITMAP_H
#define SVGBITMAP_H

#include "Cell.h"

#include "precomp.h"
#include <wx/bitmap.h>
#define NANOSVG_ALL_COLOR_KEYWORDS
#include "nanoSVG/nanosvg.h"
#include "nanoSVG/nanosvgrast.h"

/*! A wxBitmap with a constructor that generates the image from SVG.
 */
class SvgBitmap: public wxBitmap
{
public:
//! A constructor that loads the data into a wxBitmap
  SvgBitmap(wxWindow *window, const unsigned char *data, size_t len, int width = 640, int height = 480, int scaleFactor = 1);
  SvgBitmap(wxWindow *windowm const unsigned char *data, size_t len, wxSize siz, int scaleFactor = 1);
  ~SvgBitmap() override;
  SvgBitmap(SvgBitmap &&) = delete;
  SvgBitmap &operator=(SvgBitmap &&o);

  //! Converts rgba data to a wxBitmap
  static wxBitmap RGBA2wxBitmap(const unsigned char imgdata[],const int &width, const int &height, const int &scaleFactor = 1);
  //! Sets the bitmap to a new size and renders the svg image at this size.
  const SvgBitmap& SetSize(int width, int height);
  //! Sets the bitmap to a new size and renders the svg image at this size.
  const SvgBitmap& SetSize(wxSize siz){return SetSize(siz.x, siz.y);}
  //! Gets the original size of the svg image
  wxSize GetOriginalSize() const
  { return m_svgImage ? wxSize(m_svgImage->width, m_svgImage->height) : wxDefaultSize; }
  /*! An "invalid bitmap" sign
    
    We should make the image static and generate it on start-up 
    so whenever we need it it already would be available as a 
    refcounted object and therefore be likely to 
    be available even if the problem was a failing malloc(). 
    But with wx3.0 gtk crashes in this case.
   */
  static wxBitmap GetInvalidBitmap(int targetSize);
private:
  //! The scale factor the bitmap is created with
  int m_scaleFactor;
  //! No idea what nanoSVG stores here. But can be shared between images.
  static struct NSVGrasterizer* m_svgRast;
  //! The renderable svg image after we have read it in
  std::unique_ptr<NSVGimage, decltype(std::free)*> m_svgImage{nullptr, std::free};
  //! The window this bitmap will be drawn on
  wxWindow m_window;
};

#endif // SVGBITMAP_H
