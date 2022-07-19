// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2009-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//  Copyright (C) 2015-2019 Gunter Königsmann     <wxMaxima@physikbuch.de>
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
  This file defines the class HelpBrowser.

  HelpBrowser creates the list of maxima's manual anchors.
*/

#include "HelpBrowser.h"
#include <wx/sizer.h>

HelpBrowser::HelpBrowser(wxWindow *parent, Configuration *configuration):
  wxPanel(parent, wxID_ANY),
  m_configuration(configuration)
{
  wxSizer *vbox = new wxBoxSizer(wxVERTICAL);
  m_webView = wxWebView::New(this, wxID_ANY,
                             "https://maxima.sourceforge.io/docs/manual/maxima_singlepage.html");
  vbox->Add(m_webView, wxSizerFlags().Border(wxRIGHT, 10*GetContentScaleFactor()).Expand());
  SetSizer(vbox);
  FitInside();
}
