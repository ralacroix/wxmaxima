// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2020 Kuba Ober <kuba@bertec.com>
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

#ifndef FONTVARIANTCACHE_H
#define FONTVARIANTCACHE_H

#include "precomp.h"
#include "TextStyle.h"
#include <wx/font.h>
#include <functional>
#include <list>
#include <utility>
#include <unordered_map>

/*! \file
 * This file implements the wxFont cache system.
 */

class FontVariantCache final
{
  FontVariantCache(const FontVariantCache &) = delete;
  FontVariantCache &operator=(const FontVariantCache &) = delete;
public:
  FontVariantCache(wxString fontName);
  wxFont GetFont (double size,
                  bool isItalic,
                  bool isBold,
                  bool isUnderlined);
private:
  int GetIndex (
    bool isItalic,
    bool isBold,
    bool isUnderlined) const
    {
      int result =0;
      if(isItalic)
        result++;
      if(isBold)
        result += 2;
      if(isUnderlined)
        result +=4;
      return result;
    }
  std::unordered_map<double, wxFont> m_fontCaches;
  
  ~FontVariantCache();
};

#endif  // FONTVARIANTCACHE_H
