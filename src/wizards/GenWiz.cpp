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
               wxString label1, wxString defaultval1,
               wxString label2, wxString defaultval2,
               wxString label3, wxString defaultval3,
               wxString label4, wxString defaultval4,
               wxString label5, wxString defaultval5,
               wxString label6, wxString defaultval6,
               wxString label7, wxString defaultval7,
               wxString label8, wxString defaultval8,
               wxString label9, wxString defaultval9) :
  wxDialog(parent, wxID_ANY, title),
  m_commandRule(commandRule)
{
  SetName(title);
  wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
  m_description = new WrappingStaticText(this, wxID_ANY, description);
  if(description.IsEmpty())
    m_description->Show(false);
  vbox->Add(m_description, wxSizerFlags(1).Border(wxALL, 2*GetContentScaleFactor()));
  
  m_label.push_back(new wxStaticText(this, -1, label1));
  m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval1, wxDefaultPosition,
                                     wxSize(300*GetContentScaleFactor(), -1)));
  if(!label2.IsEmpty())
  {
    m_label.push_back(new wxStaticText(this, -1, label2));
    m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval2, wxDefaultPosition,
                                       wxSize(300*GetContentScaleFactor(), -1)));
  }
  if(!label3.IsEmpty())
  {
    m_label.push_back(new wxStaticText(this, -1, label3));
    m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval3, wxDefaultPosition,
                                       wxSize(300*GetContentScaleFactor(), -1)));
  }
  if(!label4.IsEmpty())
  {
    m_label.push_back(new wxStaticText(this, -1, label4));
    m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval4, wxDefaultPosition,
                                       wxSize(300*GetContentScaleFactor(), -1)));
  }
  if(!label5.IsEmpty())
  {
    m_label.push_back(new wxStaticText(this, -1, label5));
    m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval5, wxDefaultPosition,
                                       wxSize(300*GetContentScaleFactor(), -1)));
  }
  if(!label6.IsEmpty())
  {
    m_label.push_back(new wxStaticText(this, -1, label6));
    m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval6, wxDefaultPosition,
                                       wxSize(300*GetContentScaleFactor(), -1)));
  }
  if(!label7.IsEmpty())
  {
    m_label.push_back(new wxStaticText(this, -1, label7));
    m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval7, wxDefaultPosition,
                                       wxSize(300*GetContentScaleFactor(), -1)));
  }
  if(!label8.IsEmpty())
  {
    m_label.push_back(new wxStaticText(this, -1, label8));
    m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval8, wxDefaultPosition,
                                       wxSize(300*GetContentScaleFactor(), -1)));
  }
  if(!label9.IsEmpty())
  {
    m_label.push_back(new wxStaticText(this, -1, label9));
    m_textctrl.push_back(new BTextCtrl(this, -1, cfg, defaultval9, wxDefaultPosition,
                                       wxSize(300*GetContentScaleFactor(), -1)));
  }
    
  wxFlexGridSizer *grid_sizer = new wxFlexGridSizer(2, 5*GetContentScaleFactor(), 5*GetContentScaleFactor());
  for(int i = 0; i< m_textctrl.size(); i++)
  {
    grid_sizer->Add(m_label[i], 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
    grid_sizer->Add(m_textctrl[i], wxSizerFlags(1).Expand().Border(wxALL, 5*GetContentScaleFactor()));
    m_textctrl[i]->Connect(wxEVT_TEXT,
                           wxCommandEventHandler(GenWiz::OnParamChange),
                           NULL, this);
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

  vbox->Add(grid_sizer, wxSizerFlags(1).Border(wxALL, 2*GetContentScaleFactor()).Expand());
//  if(m_warning != NULL)
//    grid_sizer->Add(m_warning, 0, wxALL, 5);
  m_output = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                            wxSize(10*GetContentScaleFactor(),10*GetContentScaleFactor()),
                            wxTE_READONLY|wxTE_MULTILINE|wxTE_CHARWRAP);
  vbox->Add(m_output, wxSizerFlags(10).Border(wxALL, 2*GetContentScaleFactor()).Expand());
  if(commandRule.IsEmpty())
    m_output->Show(false);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(button_1, wxSizerFlags(1).Border(wxALL, 5*GetContentScaleFactor()));
  buttonSizer->Add(button_2, wxSizerFlags(1).Border(wxALL, 5*GetContentScaleFactor()));

  vbox->Add(buttonSizer, wxSizerFlags(1).Border(wxALL, 2*GetContentScaleFactor()));
  
  SetSizer(vbox);
  
  UpdateOutput();
  SetAutoLayout(true);
  wxPersistenceManager::Get().RegisterAndRestore(this);
  Layout();
}

void GenWiz::UpdateOutput()
{
  wxString output(m_commandRule);
  for(int i=0;i<m_textctrl.size();i++)
    output.Replace(wxString::Format("#%i#",i+1), m_textctrl[i]->GetValue());
  m_output->SetValue(output);
  Layout();
}

void GenWiz::OnParamChange(wxCommandEvent& event)
{
  UpdateOutput();
}
