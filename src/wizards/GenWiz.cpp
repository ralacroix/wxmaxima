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

#include "GenWiz.h"
#include <wx/persist/toplevel.h>

GenWiz::GenWiz(wxWindow *parent, Configuration *cfg, const wxString &title,
               const wxString &description,
               const wxString &commandRule,
               int numberOfParameters,
               ...) :
  wxDialog(parent, wxID_ANY, title)
{
  SetName(title);

  wxASSERT(numberOfParameters > 0);

  // Automatically add parameters for any tuple of "..." args
  wxFlexGridSizer *grid_sizer = new wxFlexGridSizer(2, 5*GetContentScaleFactor(), 5*GetContentScaleFactor());
  va_list ap;
  va_start(ap, numberOfParameters);
  for(int i = 0; i< numberOfParameters; i++)
  {
    wxStaticText *label = new wxStaticText(this, -1, va_arg(ap, wxString));
    grid_sizer->Add(label, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
    m_label.push_back(label);
    BTextCtrl *textbox = new BTextCtrl(this, -1, cfg, va_arg(ap, wxString), wxDefaultPosition,
                                       wxSize(300, -1));
    grid_sizer->Add(textbox, wxSizerFlags(1).Expand().Border(wxALL, 5*GetContentScaleFactor()));    
    m_textctrl.push_back(textbox);
  }
  m_textctrl[0]->SetFocus();

#if defined __WXMSW__
  button_1 = new wxButton(this, wxID_OK, _("OK"));
  button_1->SetDefault();
  button_2 = new wxButton(this, wxID_CANCEL, _("Cancel"));
#else
  button_1 = new wxButton(this, wxID_CANCEL, _("Cancel"));
  button_2 = new wxButton(this, wxID_OK, _("OK"));
  button_2->SetDefault();
#endif

  SetName(title);
  
  grid_sizer->Add(new wxStaticLine(this, -1),
                  wxSizerFlags(1).Expand().Border(wxLEFT|wxRIGHT, 2*GetContentScaleFactor()));

//  if(m_warning != NULL)
//    grid_sizer->Add(m_warning, 0, wxALL, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(button_1, wxSizerFlags(1).Border(wxALL, 5*GetContentScaleFactor()));
  buttonSizer->Add(button_2, wxSizerFlags(1).Border(wxALL, 5*GetContentScaleFactor()));

  grid_sizer->Add(buttonSizer, wxSizerFlags(1).Border(wxALL, 2*GetContentScaleFactor()));
  SetSizer(grid_sizer);
  grid_sizer->Fit(this);
  grid_sizer->SetSizeHints(this);
  
  SetAutoLayout(true);
  wxPersistenceManager::Get().RegisterAndRestore(this);
  Layout();
}
