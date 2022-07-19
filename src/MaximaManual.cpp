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
#include <wx/txtstrm.h>
#include <wx/uri.h>

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
wxString MaximaManual::GetHelpfileUrl_Singlepage(wxString keyword)
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
wxString MaximaManual::GetHelpfileUrl_FilePerChapter(wxString keyword)
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

void MaximaManual::AnchorAliasses(HelpFileAnchors &anchors)
{
  HelpFileAnchors aliasses;
  aliasses["%solve"] = "to_poly_solve";
  aliasses["find_root_error"] = "find_root";
  aliasses["wxbarsplot"] = "barsplot";
  aliasses["wxboxplot"] = "boxplot";
  aliasses["wxhistogram"] = "histogram";
  aliasses["wxpiechart"] = "piechart";
  aliasses["wxscatterplot"] = "scatterplot";
  aliasses["wxstarplot"] = "starplot";
  aliasses["wxdrawdf"] = "drawdf";
  aliasses["wxdraw"] = "draw";
  aliasses["wxdraw2d"] = "draw2d";
  aliasses["wxdraw3d"] = "draw3d";
  aliasses["with_slider_draw"] = "draw";
  aliasses["with_slider_draw2d"] = "draw2d";
  aliasses["with_slider_draw3d"] = "draw3d";
  
  for (auto it = aliasses.begin(); it !=aliasses.end(); ++it)
  {
    wxString cmdName = it->first;
    
    if((anchors.find(it->first) == anchors.end()) &&
       (anchors.find(it->second) != anchors.end()))
      anchors[it->first] = anchors[it->second];
  }
}


void MaximaManual::CompileHelpFileAnchors()
{
  SuppressErrorDialogs suppressor;

  int foundAnchors = 0;
  if(m_helpFileAnchors_singlePage.empty() && (!(m_maximaHtmlDir.IsEmpty())))
  {
    wxArrayString helpFiles;
    GetHTMLFiles htmlFilesTraverser(helpFiles, m_maximaHtmlDir);
    wxDir dir(m_maximaHtmlDir);
    dir.Traverse(htmlFilesTraverser);
    
    for (auto file: helpFiles)
    {
      wxString fileURI = wxURI(wxT("file://") + file).BuildURI();
      // wxWidgets cannot automatically replace a # as it doesn't know if it is a anchor
      // separator
      fileURI.Replace("#", "%23");
#ifdef __WINDOWS__
      fileURI.Replace("\\", "/");
#endif
#ifdef  __WXMSW__
      // Fixes a missing "///" after the "file:". This works because we always get absolute
      // file names.
      wxRegEx uriCorector1("^file:([a-zA-Z]):");
      wxRegEx uriCorector2("^file:([a-zA-Z][a-zA-Z]):");
      
      uriCorector1.ReplaceFirst(&fileURI, wxT("file:///\\1:"));
      uriCorector2.ReplaceFirst(&fileURI, wxT("file:///\\1:"));
#endif

      wxLogMessage(wxString::Format(_("Scanning help file %s for anchors"),
                                    file.c_str()));
      
      wxRegEx idExtractor(".*<span id=\\\"([a-zAZ0-9_-]*)\\\"");
      wxRegEx idExtractor2("<dt id=\\\"(index-[a-zAZ0-9_-]*)\\\"");
      wxRegEx idExtractor_oldManual(".*<a name=\\\"([a-zAZ0-9_-]*)\\\"");
      wxString escapeChars = "<=>[]`%?;\\$%&+-*/.!\'@#:^_";
      wxFileInputStream input(file);
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
                {
                  if(!file.EndsWith(wxT("_singlepage.html")))
                    m_helpFileAnchors_singlePage[token] = fileURI + "#" + id;
                  else
                    m_helpFileAnchors_FilePerChapter[token] = fileURI + "#" + id;
                  m_helpFileAnchors_generic[token] = id;
                  foundAnchors++;
                }
              }
            }
          }
        }
      }
      AnchorAliasses(m_helpFileAnchors_generic);
      AnchorAliasses(m_helpFileAnchors_FilePerChapter);
      AnchorAliasses(m_helpFileAnchors_singlePage);
      wxLogMessage(wxString::Format(_("Found %i anchors."), foundAnchors));
    }
    if(foundAnchors > 50)
      SaveManualAnchorsToCache();
    else
      LoadBuiltInManualAnchors();
  }
}

wxDirTraverseResult MaximaManual::GetHTMLFiles::OnFile(const wxString& filename)
{
  wxFileName newItemName(filename);
  wxString newItem = m_prefix + wxFileName::GetPathSeparator() + newItemName.GetFullName();
  newItem.Replace(wxFileName::GetPathSeparator(),"/");
  if(newItem.EndsWith(".html") && (m_files.Index(newItem) == wxNOT_FOUND))
    m_files.Add(newItem);
  return wxDIR_CONTINUE;
}

wxDirTraverseResult MaximaManual::GetHTMLFiles::OnDir(const wxString& dirname)
{
  return wxDIR_IGNORE;
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
  for (it = m_helpFileAnchors_generic.begin();
       it != m_helpFileAnchors_generic.end();
       ++it)
  {
    wxXmlNode *manualEntry =
      new wxXmlNode(
        headNode,
          wxXML_ELEMENT_NODE,
        "entry");
    {
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
    {
      wxXmlNode *anchorNode = new wxXmlNode(
        manualEntry,
        wxXML_ELEMENT_NODE,
        "anchor");
      new wxXmlNode(
        anchorNode,
        wxXML_TEXT_NODE,
        wxEmptyString,
        it->second);
    }
    if(m_helpFileAnchors_singlePage.find(it->first) != m_helpFileAnchors_singlePage.end())
    {
      wxXmlNode *keyNode = new wxXmlNode(
        manualEntry,
        wxXML_ELEMENT_NODE,
        "url_singlepage");
      new wxXmlNode(
        keyNode,
        wxXML_TEXT_NODE,
        wxEmptyString,
        m_helpFileAnchors_singlePage[it->first]);
    }
    if(m_helpFileAnchors_FilePerChapter.find(it->first) != m_helpFileAnchors_FilePerChapter.end())
    {
      wxXmlNode *keyNode = new wxXmlNode(
        manualEntry,
        wxXML_ELEMENT_NODE,
        "url_fileperchapter");
      new wxXmlNode(
        keyNode,
        wxXML_TEXT_NODE,
        wxEmptyString,
        m_helpFileAnchors_singlePage[it->first]);
    }
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
      wxString url_singlepage;
      wxString url_filePerChapter;
      wxString anchor;
      wxXmlNode *node = entry->GetChildren();
      while(node)
      {
        if((node->GetName() == wxT("anchor")) && (node->GetChildren()))
          anchor = node->GetChildren()->GetContent();
        if((node->GetName() == wxT("key")) && (node->GetChildren()))
          key = node->GetChildren()->GetContent();
        if((node->GetName() == wxT("url_singlepage")) && (node->GetChildren()))
          url_singlepage = node->GetChildren()->GetContent();
        if((node->GetName() == wxT("url_fileperchapter")) && (node->GetChildren()))
          url_filePerChapter = node->GetChildren()->GetContent();
        node = node->GetNext();
      }
      if((!key.IsEmpty()) && (!anchor.IsEmpty()))
        m_helpFileAnchors_singlePage[key] = anchor;
      if((!key.IsEmpty()) && (!url_filePerChapter.IsEmpty()))
        m_helpFileAnchors_FilePerChapter[key] = url_filePerChapter;
      if((!key.IsEmpty()) && (!url_singlepage.IsEmpty()))
        m_helpFileAnchors_singlePage[key] = url_singlepage;
    }
    entry = entry->GetNext();
  }
  return !m_helpFileAnchors_singlePage.empty();
}

wxString MaximaManual::GetHelpfileURL(wxString keyword)
{
  return GetHelpfileUrl_Singlepage(keyword);
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
    return;
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
  helpfilepaths.Add(docDir);
  helpfilepaths.Add(docDir+"/info");
  helpfilepaths.Add(docDir+"/info/html");
  helpfilepaths.Add(docDir+"/html");
  helpfilepaths.Add(docDir+"/../html");
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

void MaximaManual::LoadHelpFileAnchors(wxString docdir, wxString maximaVersion)
{
  FindMaximaHtmlDir(docdir);
  m_maximaVersion = maximaVersion;
  if(m_helpfileanchorsThread)
  {
    m_helpfileanchorsThread->join();
    m_helpfileanchorsThread.reset();
  }
  if(m_helpFileAnchors_singlePage.empty())
  {
    if(!LoadManualAnchorsFromCache())
    {
      if(!m_maximaHtmlDir.IsEmpty())
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
        wxLogMessage(_("Maxima help file not found!"));
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
