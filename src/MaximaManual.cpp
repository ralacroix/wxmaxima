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
  This file defines the class MaximaManual.

  MaximaManual creates the list of maxima's manual anchors.
*/

#include "MaximaManual.h"
#include "Dirstructure.h"
#include "ErrorRedirector.h"
#include "wxm_manual_anchors_xml.h"
#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>

MaximaManual::MaximaManual(Configuration *configuration)
{
  m_configuration = configuration;
}

  MaximaManual::HelpFileAnchors MaximaManual::GetHelpfileAnchors()
  {
    if(m_helpfileanchorsThread)
    {
      if(m_helpfileanchorsThread->joinable())
        m_helpfileanchorsThread->join();
      m_helpfileanchorsThread.reset();
    }
    return m_helpFileAnchors_generic;
  }

wxString MaximaManual::GetHelpfileAnchorName(wxString keyword)
{
  if(m_helpfileanchorsThread)
  {
    if(m_helpfileanchorsThread->joinable())
      m_helpfileanchorsThread->join();
    m_helpfileanchorsThread.reset();
  }
  
  auto anchor = m_helpFileAnchors_generic.find(keyword);
  if(anchor == m_helpFileAnchors_generic.end())
    return wxEmptyString;
  else
    return anchor->second;
}
wxString MaximaManual::GetHelpfileAnchor_Singlepage(wxString keyword)
{
  if(m_helpfileanchorsThread)
  {
    if(m_helpfileanchorsThread->joinable())
      m_helpfileanchorsThread->join();
    m_helpfileanchorsThread.reset();
  }

  auto anchor = m_helpFileAnchors_singlePage.find(keyword);
  if(anchor == m_helpFileAnchors_singlePage.end())
    return wxEmptyString;
  else
    return anchor->second;
}
wxString MaximaManual::GetHelpfileAnchor_FilePerChapter(wxString keyword)
{
  if(m_helpfileanchorsThread)
  {
    if(m_helpfileanchorsThread->joinable())
      m_helpfileanchorsThread->join();
    m_helpfileanchorsThread.reset();
  }

  auto anchor = m_helpFileAnchors_FilePerChapter.find(keyword);
  if(anchor == m_helpFileAnchors_FilePerChapter.end())
    return wxEmptyString;
  else
    return anchor->second;
}

bool MaximaManual::LoadBuiltInManualAnchors()
{
  wxLogMessage(_("Using the built-in list of manual anchors."));
  wxMemoryInputStream istream(MANUAL_ANCHORS_XML, MANUAL_ANCHORS_XML_SIZE);
  wxXmlDocument xmlDoc;
  if(!xmlDoc.Load(istream, wxT("UTF-8")))
    return false;
  if(!LoadManualAnchorsFromXML(xmlDoc, false))
    return false;
  return true;
}

bool MaximaManual::LoadManualAnchorsFromCache()
{
  SuppressErrorDialogs suppressor;
  wxString anchorsFile = Dirstructure::Get()->AnchorsCacheFile();
  if(!wxFileExists(anchorsFile))
  {
    wxLogMessage(_("No file with the subjects the manual contained in the last wxMaxima run."));
    return false;
  }
  wxXmlDocument xmlDocument(anchorsFile);
  if(!xmlDocument.IsOk())
  {
    wxLogMessage(_("The cache for the subjects the manual contains cannot be read."));
    wxRemoveFile(anchorsFile);
    return false;
  }

  if(LoadManualAnchorsFromXML(xmlDocument))
  {
    wxLogMessage(wxString::Format(_("Read the entries the maxima manual offers from %s"), Dirstructure::Get()->AnchorsCacheFile().utf8_str()));
    return true;
  }
  return !m_helpFileAnchors_singlePage.empty();
}


void MaximaManual::CompileHelpFileAnchors()
{
  SuppressErrorDialogs suppressor;

  if(m_helpFileAnchors_singlePage.empty() && (!(m_maximaHtmlDir.IsEmpty())))
  {
    m_helpFileAnchors_singlePage["wxbarsplot"] = "barsplot";
    m_helpFileAnchors_singlePage["wxboxplot"] = "boxplot";
    m_helpFileAnchors_singlePage["wxhistogram"] = "histogram";
    m_helpFileAnchors_singlePage["wxpiechart"] = "piechart";
    m_helpFileAnchors_singlePage["wxscatterplot"] = "scatterplot";
    m_helpFileAnchors_singlePage["wxstarplot"] = "starplot";
    m_helpFileAnchors_singlePage["wxdrawdf"] = "drawdf";
    m_helpFileAnchors_singlePage["wxdraw"] = "draw";
    m_helpFileAnchors_singlePage["wxdraw2d"] = "draw2d";
    m_helpFileAnchors_singlePage["wxdraw3d"] = "draw3d";
    m_helpFileAnchors_singlePage["with_slider_draw"] = "draw";
    m_helpFileAnchors_singlePage["with_slider_draw2d"] = "draw2d";
    m_helpFileAnchors_singlePage["with_slider_draw3d"] = "draw3d";

    int foundAnchors = 0;
    wxLogMessage(_("Compiling the list of anchors the maxima manual provides"));
    wxRegEx idExtractor(".*<span id=\\\"([a-zAZ0-9_-]*)\\\"");
    wxRegEx idExtractor2("<dt id=\\\"(index-[a-zAZ0-9_-]*)\\\"");
    wxRegEx idExtractor_oldManual(".*<a name=\\\"([a-zAZ0-9_-]*)\\\"");
    wxString escapeChars = "<=>[]`%?;\\$%&+-*/.!\'@#:^_";
    if(wxFileExists(MaximaHelpFile))
    {
      wxFileInputStream input(MaximaHelpFile);
      if(input.IsOk())
      {
        wxTextInputStream text(input, wxT('\t'), wxConvAuto(wxFONTENCODING_UTF8));
        while(input.IsOk() && !input.Eof())
        {
          wxString line = text.ReadLine();
          wxStringTokenizer tokens(line, wxT(">"));
          while(tokens.HasMoreTokens())
          {
            wxString token = tokens.GetNextToken();
            wxString oldToken(token);
            wxString id;
            if(idExtractor.Replace(&token, "\\1")>0)
              id = token;
            else
              if(idExtractor2.Replace(&token, "\\1")>0)
                id = token;
              else
              {
                if(idExtractor_oldManual.Replace(&token, "\\1")>0)
                  id = token;
              }
            if(!id.IsEmpty())
            {
              // anchorless tokens begin with "index-"
              token.Replace("index-", "");
              // In anchors a space is represented by a hyphen
              token.Replace("-", " ");
              // Some other chars including the minus are represented by "_00xx"
              // where xx is being the ascii code of the char.
              for(wxString::const_iterator it = escapeChars.begin(); it != escapeChars.end(); ++it)
                token.Replace(wxString::Format("_00%x",(char)*it), *it);
              // What the g_t means I don't know. But we don't need it
              if(token.StartsWith("g_t"))
                token = token.Right(token.Length()-3);
              //! Tokens that end with "-1" aren't too useful, normally.
              if((!token.EndsWith("-1")) && (!token.Contains(" ")))              {
                m_helpFileAnchors_singlePage[token] = id;
                foundAnchors++;
              }
            }
          }
        }
      }
    }
    if(m_helpFileAnchors_singlePage["%solve"].IsEmpty())
      m_helpFileAnchors_singlePage["%solve"] = m_helpFileAnchors_singlePage["to_poly_solve"];

    if((m_helpFileAnchors_singlePage.find("find_root_error") == m_helpFileAnchors_singlePage.end()) &&
       (m_helpFileAnchors_singlePage.find("find_root") != m_helpFileAnchors_singlePage.end()))
      m_helpFileAnchors_singlePage["find_root_error"] = m_helpFileAnchors_singlePage["find_root"];

    wxLogMessage(wxString::Format(_("Found %i anchors."), foundAnchors));
    if(foundAnchors > 50)
      SaveManualAnchorsToCache();
    else
      LoadBuiltInManualAnchors();
  }
}

void MaximaManual::SaveManualAnchorsToCache()
{
  long num = m_helpFileAnchors_singlePage.size();
  if(num <= 50)
  {
    wxLogMessage(
      wxString::Format(
        _("Found only %li keywords in maxima's manual. Not caching them to disc."),
        num));
    return;
  }
  wxXmlAttribute *maximaVersion = new wxXmlAttribute(
    wxT("maxima_version"),
    m_maximaVersion);
  wxXmlNode *topNode = new wxXmlNode(
    NULL,
    wxXML_DOCUMENT_NODE, wxEmptyString,
    wxEmptyString
    );
  wxXmlNode *headNode = new wxXmlNode(
    topNode,
    wxXML_ELEMENT_NODE, wxT("maxima_toc"),
    wxEmptyString,
    maximaVersion
    );

  MaximaManual::HelpFileAnchors::const_iterator it;
  for (it = m_helpFileAnchors_singlePage.begin();
       it != m_helpFileAnchors_singlePage.end();
       ++it)
  {
    wxXmlNode *manualEntry =
      new wxXmlNode(
        headNode,
          wxXML_ELEMENT_NODE,
        "entry");
    wxXmlNode *anchorNode = new wxXmlNode(
      manualEntry,
      wxXML_ELEMENT_NODE,
      "anchor");
    new wxXmlNode(
      anchorNode,
      wxXML_TEXT_NODE,
      wxEmptyString,
      it->second);
    wxXmlNode *keyNode = new wxXmlNode(
      manualEntry,
      wxXML_ELEMENT_NODE,
        "key");
    new wxXmlNode(
        keyNode,
        wxXML_TEXT_NODE,
        wxEmptyString,
        it->first);
  }
  wxXmlDocument xmlDoc;
  xmlDoc.SetDocumentNode(topNode);
  wxXmlNode *commentNode = new wxXmlNode(
    NULL,
    wxXML_COMMENT_NODE,
    wxEmptyString,
    _("This file is generated by wxMaxima\n"
      "It caches the list of subjects maxima's manual offers and is automatically\n"
      "overwritten if maxima's version changes or the file cannot be read"));

  xmlDoc.AppendToProlog(commentNode);

  wxString saveName = Dirstructure::AnchorsCacheFile();
  wxLogMessage(wxString::Format(_("Trying to cache the list of subjects the manual contains in the file %s."),
                                saveName.utf8_str()));
  xmlDoc.Save(saveName);
}

bool MaximaManual::LoadManualAnchorsFromXML(wxXmlDocument xmlDocument, bool checkManualVersion)
{
  wxXmlNode *headNode = xmlDocument.GetDocumentNode();
  if(!headNode)
  {
    wxLogMessage(_("The cache for the subjects the manual contains has no head node."));
    return false;
  }
  headNode = headNode->GetChildren();
  while((headNode) && (headNode->GetName() != wxT("maxima_toc")))
    headNode = headNode->GetNext();
  if(!headNode)
  {
    wxLogMessage(_("Anchors file has no top node."));
    return false;
  }
  if(checkManualVersion && ((headNode->GetAttribute(wxT("maxima_version")) != m_maximaVersion)))
  {
    wxLogMessage(_("The cache for the subjects the manual contains is from a different Maxima version."));
    return false;
  }
  wxXmlNode *entry = headNode->GetChildren();
  if(entry == NULL)
  {
    wxLogMessage(_("No entries in the caches for the subjects the manual contains."));
    return false;
  }
  while(entry)
  {
    if(entry->GetName() == wxT("entry"))
    {
      wxString key;
      wxString anchor;
      wxXmlNode *node = entry->GetChildren();
      while(node)
      {
        if((node->GetName() == wxT("anchor")) && (node->GetChildren()))
          anchor = node->GetChildren()->GetContent();
        if((node->GetName() == wxT("key")) && (node->GetChildren()))
          key = node->GetChildren()->GetContent();
        node = node->GetNext();
      }
      if((!key.IsEmpty()) && (!anchor.IsEmpty()))
        m_helpFileAnchors_singlePage[key] = anchor;
    }
    entry = entry->GetNext();
  }
  return !m_helpFileAnchors_singlePage.empty();
}

void MaximaManual::FindMaximaHtmlDir(wxString docDir)
{
  // One may set the help file location in the wxMaxima configuration (on Unix: ~/.wxMaxima), e.g.
  // helpFile=/usr/local/share/maxima/5.44.0/doc/html/maxima_singlepage.html
  // Use that file, if the configuration option is used.
  wxString headerFile;
  wxConfig::Get()->Read(wxT("helpFile"), &headerFile);
  if (headerFile.Length() && wxFileExists(headerFile)) {
    wxLogMessage(_("Using Maxima help file from wxMaxima configuration file (helpFile=...))"));
    return headerFile;
  }
#ifdef __CYGWIN__
  // Cygwin uses /c/something instead of c:/something and passes this path to the
  // web browser - which doesn't support cygwin paths => convert the path to a
  // native windows pathname if needed.
  if(headerFile.Length()>1 && headerFile[1]==wxT('/'))
  {
    headerFile[1]=headerFile[2];
    headerFile[2]=wxT(':');
  }
#endif // __CYGWIN__
  wxPathList helpfilepaths;
  helpfilepaths.Add(m_maximaDocDir);
  helpfilepaths.Add(m_maximaDocDir+"/info");
  helpfilepaths.Add(m_maximaDocDir+"/info/html");
  helpfilepaths.Add(m_maximaDocDir+"/html");
  helpfilepaths.Add(m_maximaDocDir+"/../html");
  helpfilepaths.Add(m_configuration->MaximaShareDir() + "/../doc/html");
  helpfilepaths.Add(m_configuration->MaximaShareDir() + "/doc/html");
  wxString helpfile_location = helpfilepaths.FindAbsoluteValidPath("maxima_singlepage.html");
  wxFileName helpfile_cleanup(helpfile_location);
  helpfile_cleanup.Normalize(wxPATH_NORM_ENV_VARS | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT);

  if (helpfile_cleanup.IsFileReadable())
  {
    m_maximaHtmlDir = helpfile_cleanup.GetPath();
    wxLogMessage(wxString::Format(_("Found the maxima HTML manual in the folder %s."),
                                  m_maximaHtmlDir.c_str()));
  }
  else
  {
    m_maximaHtmlDir.clear();
    wxLogMessage(_("Didn't find the maxima HTML manual."));
    
  }
}

void MaximaManual::LoadHelpFileAnchors(wxString docdir)
{
  if(m_helpfileanchorsThread)
  {
    m_helpfileanchorsThread->join();
    m_helpfileanchorsThread.reset();
  }
  if(m_helpFileAnchors_singlePage.empty())
  {
    if(!LoadManualAnchorsFromCache())
    {
      if(wxFileExists(GetMaximaHelpFile(docdir)))
      {
        if(m_helpfileanchorsThread)
        {
          // Show a busy cursor as long as we finish the old background task
          wxBusyCursor crs;
          m_helpfileanchorsThread->join();
        }
        m_helpfileanchorsThread =
          std::unique_ptr<std::thread>(
            new std::thread(&MaximaManual::CompileHelpFileAnchors, this));
      }
      else
      {
        wxLogMessage(wxString::Format(_("Help file %s not found!"),
                                      GetMaximaHelpFile().c_str()));
        LoadBuiltInManualAnchors();
      }
    }
  }
}

MaximaManual::~MaximaManual()
{
  wxLogMessage(_("Waiting for the thread that parses the maxima manual to finish"));
  if(m_helpfileanchorsThread)
  {
    if(m_helpfileanchorsThread->joinable())
      m_helpfileanchorsThread->join();
    m_helpfileanchorsThread.reset();
  }
}
