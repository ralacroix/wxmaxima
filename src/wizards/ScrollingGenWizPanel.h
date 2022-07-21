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

#ifndef SCROLLINGGENWIZ_H
#define SCROLLINGGENWIZ_H

#include "precomp.h"
#include "GenWizPanel.h"
#include <wx/scrolwin.h>
#include <wx/panel.h>

class ScrollingGenWizPanel : public wxScrolled<wxPanel>
{
public:
  ScrollingGenWizPanel(wxWindow *parent, Configuration *cfg, MaximaManual *manual,
                       const wxString &description = {}, const wxString &description_tooltip = {},
                       const wxString &commandRule = {}, 
                       wxString label1 = {}, wxString defaultval1 = {}, wxString tooltip1 = {},
                       wxString label2 = {}, wxString defaultval2 = {}, wxString tooltip2 = {},
                       wxString label3 = {}, wxString defaultval3 = {}, wxString tooltip3 = {},
                       wxString label4 = {}, wxString defaultval4 = {}, wxString tooltip4 = {},
                       wxString label5 = {}, wxString defaultval5 = {}, wxString tooltip5 = {},
                       wxString label6 = {}, wxString defaultval6 = {}, wxString tooltip6 = {},
                       wxString label7 = {}, wxString defaultval7 = {}, wxString tooltip7 = {},
                       wxString label8 = {}, wxString defaultval8 = {}, wxString tooltip8 = {},
                       wxString label9 = {}, wxString defaultval9 = {}, wxString tooltip9 = {});
  
  wxString operator[](int i) const { return m_panel->operator[](i); }

  wxString GetOutput() const {return m_panel->GetOutput();}

  void NewWizard(wxString description, const wxString &description_tooltip,
                 const wxString &commandRule,
                 wxString label1, wxString defaultval1, wxString tooltip1,
                 wxString label2, wxString defaultval2, wxString tooltip2,
                 wxString label3, wxString defaultval3, wxString tooltip3,
                 wxString label4, wxString defaultval4, wxString tooltip4,
                 wxString label5, wxString defaultval5, wxString tooltip5,
                 wxString label6, wxString defaultval6, wxString tooltip6,
                 wxString label7, wxString defaultval7, wxString tooltip7,
                 wxString label8, wxString defaultval8, wxString tooltip8,
                 wxString label9, wxString defaultval9, wxString tooltip9)
    {
      m_panel->NewWizard(description, description_tooltip,
                         commandRule,
                         label1, defaultval1, tooltip1,
                         label2, defaultval2, tooltip2,
                         label3, defaultval3, tooltip3,
                         label4, defaultval4, tooltip4,
                         label5, defaultval5, tooltip5,
                         label6, defaultval6, tooltip6,
                         label7, defaultval7, tooltip7,
                         label8, defaultval8, tooltip8,
                         label9, defaultval9, tooltip9
         );
    }

  wxButton *GetOKButton() const {return m_panel->GetOKButton();}
  wxButton *GetAbortButton() const {return m_panel->GetAbortButton();}
  wxButton *GetInsertButton() const{return m_panel->GetInsertButton();}

  protected:
  GenWizPanel *m_panel;
  void OnSize(wxSizeEvent &event);
};

#endif // SCROLLINGGENWIZ_H
