// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2004-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//  Copyright (C) 2017-2018 Gunter Königsmann <wxMaxima@physikbuch.de>
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

#ifndef GEN1WIZ_H
#define GEN1WIZ_H

#include "precomp.h"
#include <wx/wx.h>
#include <wx/statline.h>
#include <cstdarg>
#include <vector>
#include "BTextCtrl.h"

class GenWiz : public wxDialog
{
public:
  GenWiz(wxWindow *parent, Configuration *cfg,
         const wxString &title,
         const wxString &description,
         const wxString &commandRule,
         int numberOfParameters,
         ...);

  wxString operator[](int i) const { return m_textctrl[i]->GetValue(); }

protected:
  std::vector<BTextCtrl*> m_textctrl;
  std::vector<wxStaticText *> m_label;
  wxButton *button_1;
  wxButton *button_2;
  wxStaticText *m_description;
};

#endif // GEN1WIZ_H
