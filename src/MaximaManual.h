// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2009-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//  Copyright (C) 2015 Gunter Königsmann     <wxMaxima@physikbuch.de>
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
  This file declares the class MaximaManual.

  MaximaManual creates the list of autocompletions for a string and allows
  dynamically appending maxima commands to this list as soon as they are defined.
*/

#ifndef MAXIMAMANUAL_H
#define MAXIMAMANUAL_H

#include "precomp.h"
#include <thread>
#include <memory>
#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/arrstr.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include <wx/filename.h>
#include <vector>
#include "Configuration.h"

/* The autocompletion logic

   The wordlists for autocompletion for keywords come from several sources:

     - wxMaxima::ReadLoadSymbols receive the contents of maxima's variables
       "values" and "functions" after a package is loaded.
     - all words that appear in the worksheet
     - and a list of maxima's builtin commands.
 */
class MaximaManual
{

public:
  explicit MaximaManual(Configuration *configuration);
  WX_DECLARE_STRING_HASH_MAP(wxString, HelpFileAnchors);
  HelpFileAnchors GetHelpfileAnchors();
  bool FindMaximaHtmlDir(wxString docDir);
  wxString GetHelpfileAnchorName(wxString keyword);
  wxString GetHelpfileAnchor_Singlepage(wxString keyword);
  wxString GetHelpfileAnchor_FilePerChapter(wxString keyword);
  wxString GetHelpfileAnchor(wxString keyword){return GetHelpfileAnchor_Singlepage(keyword);}
  //! Search maxima's help file for command and variable names
  void LoadHelpFileAnchors(wxString directory);
  //! Collect all keyword anchors in the help file
  void CompileHelpFileAnchors();
  //! Load the result from the last CompileHelpFileAnchors from the disk cache
  bool LoadManualAnchorsFromCache();
  //! Load the help file anchors from an wxXmlDocument
  bool LoadManualAnchorsFromXML(wxXmlDocument xmlDocument, bool checkManualVersion = true);
  //! Load the help file anchors from the built-in list
  bool LoadBuiltInManualAnchors();
  //! Save the list of help file anchors to the cache.
  void SaveManualAnchorsToCache();

  ~MaximaManual();
private:
  //! The thread the help file anchors are compiled in
  std::unique_ptr<std::thread> m_helpfileanchorsThread;
  //! The configuration storage
  Configuration *m_configuration;
  //! All anchors for keywords maxima's helpfile contains (singlepage version)
  HelpFileAnchors m_helpFileAnchors_singlePage;
  //! All anchors for keywords maxima's helpfile contains (file-per-chapter version)
  HelpFileAnchors m_helpFileAnchors_FilePerChapter;
  //! All anchors for keywords maxima's helpfile contains (without the file name)
  HelpFileAnchors m_helpFileAnchors_generic;
  wxString m_maximaHtmlDir;
};

#endif // MAXIMAMANUAL_H
