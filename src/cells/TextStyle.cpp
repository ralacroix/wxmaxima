// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode:
// nil -*-
//
//  Copyright (C) 2004-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//            (C) 2014-2018 Gunter Königsmann <wxMaxima@physikbuch.de>
//            (C) 2020 Kuba Ober <kuba@mareimbrium.org>
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

/*!
 * \file
 * This file implements the text style system.
 */

#include "TextStyle.h"
#include <array>
#include <list>
#include <memory>
#include <iostream>
#include <vector>
#include <wx/fontenum.h>
#include <wx/log.h>
#include <wx/thread.h>
#include <wx/translation.h>

/*
 * Style
 */

constexpr wxFontFamily Style::Default_Family;
constexpr wxFontEncoding Style::Default_Encoding;
constexpr wxFontWeight Style::Default_Weight;
constexpr wxFontStyle Style::Default_FontStyle;
constexpr bool Style::Default_Underlined;
constexpr bool Style::Default_Strikethrough;
constexpr AFontSize Style::Default_FontSize;
constexpr uint32_t Style::Default_ColorRGB;

Style::Style()
{
  SetFontName(wxNORMAL_FONT->GetFaceName());
  wxASSERT(m.fontCache != NULL);
}

Style::Style(AFontSize fontSize)
{
  SetFontSize(fontSize);
  SetFontName(wxNORMAL_FONT->GetFaceName());
  wxASSERT(m.fontCache != NULL);
}

Style::Style(const Style &o) : m(o.m) {
  wxASSERT(m.fontCache != NULL);
}

Style &Style::operator=(const Style &o) {
  if (&o != this)
    {
      m = o.m;
      SetFontName(o.GetFontName());
    }
  wxASSERT(m.fontCache != NULL);

  return *this;
}

wxFontFamily Style::GetFamily() const { return m.family; }

wxFontEncoding Style::GetEncoding() const { return m.encoding; }

wxFontWeight Style::GetWeight() const { return m.weight; }

wxFontStyle Style::GetFontStyle() const { return m.fontStyle; }

bool Style::IsUnderlined() const { return m.underlined; }

bool Style::IsStrikethrough() const { return m.strikethrough; }

const wxString &Style::GetFontName() const {
  if(m.fontCache)
    return m.fontCache->GetFaceName();
  else
    return m_emptyString;
}

//! The size of this style's font, asserted to be valid.
AFontSize Style::GetFontSize() const {
  wxASSERT(m.fontSize.IsValid());
  return m.fontSize;
}

uint32_t Style::GetRGBColor() const { return m.rgbColor; }

using did_change = Style::did_change;

did_change Style::SetFamily(wxFontFamily family) {
  if (m.family == family)
    return false;
  m.family = family;
  return true;
}

did_change Style::SetEncoding(wxFontEncoding encoding) {
  if (m.encoding == encoding)
    return false;
  m.encoding = encoding;
  return true;
}

did_change Style::SetFontStyle(wxFontStyle style) {
  if (m.fontStyle == style)
    return false;
  m.fontStyle = style;
  return true;
}

did_change Style::SetWeight(wxFontWeight weight) {
  if (m.weight == weight)
    return false;
  m.weight = weight;
  return true;
}

did_change Style::SetBold(bool bold) {
  return SetWeight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
}

did_change Style::SetLight(bool light) {
  return SetWeight(light ? wxFONTWEIGHT_LIGHT : wxFONTWEIGHT_NORMAL);
}

did_change Style::SetItalic(bool italic) {
  return SetFontStyle(italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
}

did_change Style::SetSlant(bool slant) {
  return SetFontStyle(slant ? wxFONTSTYLE_SLANT : wxFONTSTYLE_NORMAL);
}

did_change Style::SetUnderlined(bool underlined) {
  if (m.underlined == underlined)
    return false;
  m.underlined = underlined;
  return true;
}

did_change Style::SetStrikethrough(bool strikethrough) {
  if (m.strikethrough == strikethrough)
    return false;
  m.strikethrough = strikethrough;
  return true;
}

did_change Style::SetFontName(wxString fontName) {
  if ((m.fontCache != NULL) && (GetFontName() == fontName))
    {
      return false;
    }
  auto fontCache = m_fontCaches.find(fontName);
  if(fontCache == m_fontCaches.end())
    {
      auto newfontCache = std::shared_ptr<FontVariantCache>(new FontVariantCache(fontName));
      m_fontCaches[std::move(fontName)] = newfontCache;
      m.fontCache = std::move(newfontCache);
    }
  else
    m.fontCache = fontCache->second;
  wxASSERT(m.fontCache != NULL);

  return true;
}

did_change Style::SetFontSize(AFontSize fontSize) {
  if (m.fontSize == fontSize)
    return false;
  m.fontSize = fontSize;
  return true;
}

did_change Style::SetRGBColor(uint32_t rgb) {
  if (m.rgbColor == rgb)
    return false;
  m.rgbColor = rgb;
  return true;
}

did_change Style::SetColor(const wxColor &color) {
  return SetRGBColor(color.GetRGB());
}

did_change Style::SetColor(wxSystemColour sysColour) {
  return SetColor(wxSystemSettings::GetColour(sysColour));
}

did_change Style::SetFontFrom(const Style &o) {
  return SetFontFaceAndSizeFrom(o) ||
    SetFontStyle(o.GetFontStyle()) ||
    SetWeight(o.GetWeight()) ||
    SetUnderlined(o.IsUnderlined()) ||
    SetStrikethrough(o.IsStrikethrough());
}

did_change Style::SetFontFaceFrom(const Style &o) {
  return SetFontName(o.GetFontName()) ||
    SetEncoding(o.GetEncoding()) ||
    SetFamily(o.GetFamily());
}

did_change Style::SetFontFaceAndSizeFrom(const Style &o) {
  return SetFontFaceFrom(o) || SetFontSize(o.m.fontSize);
}

bool Style::IsFontOk() const { return GetFont().IsOk(); }

did_change Style::SetFromFont(const wxFont &font) {
  if (font.IsOk()) {
    m.encoding = font.GetEncoding();
    m.family = font.GetFamily();
    m.fontStyle = font.GetStyle();
    m.underlined = font.GetUnderlined();
    m.strikethrough = font.GetStrikethrough();
    m.weight = font.GetWeight();
    m.fontSize = GetFontSize(font);
    SetFontName(wxString(font.GetFaceName()));
  } else
    {
      SetFontName(wxNORMAL_FONT->GetFaceName());
    }
  return true;
}

AFontSize Style::GetFontSize(const wxFont &font) {
#if wxCHECK_VERSION(3, 1, 2)
  return AFontSize(font.GetFractionalPointSize());
#endif
  return AFontSize(font.GetPointSize());
}

void Style::SetFontSize(wxFont &font, AFontSize fontSize) {
#if wxCHECK_VERSION(3, 1, 2)
  return font.SetFractionalPointSize(fontSize.Get());
#endif
  return font.SetPointSize(fontSize.GetAsLong());
}

wxFontInfo Style::GetAsFontInfo() const {
  wxFontInfo result(GetFontSize().GetForWX());

  result.Family(GetFamily())
    .FaceName(GetFontName())
    .Underlined(IsUnderlined())
    .Strikethrough(IsStrikethrough())
    .Encoding(GetEncoding());

  // This pattern is used to ensure that the legacy variant
  // still compiles (doesn't bitrot).
#if wxCHECK_VERSION(3, 1, 2)
  return result.Style(GetFontStyle()).Weight(GetWeight());
#endif
  return result.Slant(IsSlant())
    .Italic(IsItalic())
    .Bold(IsBold())
    .Light(IsLight());
}

const wxColor &Style::Default_Color() {
  using colorULong = unsigned long;
  static const wxColor color{colorULong(Default_ColorRGB)};
  return color;
}

static const wxString k_color = wxS("/color");
static const wxString k_bold = wxS("/bold");
static const wxString k_light = wxS("/light");
static const wxString k_italic = wxS("/italic");
static const wxString k_slant = wxS("/slant");
static const wxString k_underlined = wxS("/underlined");
static const wxString k_strikethrough = wxS("/strikethrough");
static const wxString k_fontsize_float = wxS("/Style/Text/fontsize_float");
static const wxString k_fontsize_legacy = wxS("/Style/Text/fontsize");
static const wxString k_fontname = wxS("/Style/Text/fontname");

Style &Style::Read(wxConfigBase *config, const wxString &where) {
  wxString tmpStr;
  bool tmpBool;
  long tmpLong;
  double tmpDouble;

  if (config->Read(where+k_color, &tmpStr)) {
    wxColor color = wxColor(tmpStr);
    if (color.IsOk())
      SetColor(color);
  }
  if (config->Read(where + k_bold, &tmpBool))
    SetBold(tmpBool);
  else if (config->Read( where + k_light, &tmpBool))
    SetLight(tmpBool);
  if (config->Read(where + k_italic, &tmpBool))
    SetItalic(tmpBool);
  else if (config->Read(where + k_slant, &tmpBool))
    SetSlant(tmpBool);
  if (config->Read(where + k_underlined, &tmpBool))
    SetUnderlined(tmpBool);
  if (config->Read(where + k_strikethrough, &tmpBool))
    SetStrikethrough(tmpBool);
  if (config->Read(where + k_fontsize_float, &tmpDouble))
    SetFontSize(AFontSize(tmpDouble));
  else if (config->Read(where + k_fontsize_legacy, &tmpLong))
    SetFontSize(AFontSize(tmpLong));
  if (config->Read(where + k_fontname, &tmpStr) &&
      !tmpStr.empty())
    SetFontName(tmpStr);
  else
    SetFontName(wxNORMAL_FONT->GetFaceName());

  // Validation is deferred to the point of first use, etc.
  return *this;
}

void Style::Write(wxConfigBase *config, const wxString &where) const {
  config->Write(where + k_color, GetColor().GetAsString());
  config->Write(where + k_bold, IsBold());
  config->Write(where + k_italic, IsItalic());
  config->Write(where + k_slant, IsSlant());
  config->Write(where + k_light, IsLight());
  config->Write(where + k_strikethrough, IsStrikethrough());
  config->Write(where + k_underlined, IsUnderlined());
  config->Write(where + k_fontsize_float, GetFontSize().Get());
  config->Write(where + k_fontname, GetFontName());
}

const wxFont &Style::GetFont(AFontSize fontSize) const {
  return *(m.fontCache->GetFont(fontSize.Get(), IsItalic(), IsBold(), IsUnderlined(),
                                IsSlant(), IsStrikethrough()));
}

Style Style::FromStockFont(wxStockGDI::Item font) {
  Style retval;
  switch (font) {
  case wxStockGDI::FONT_ITALIC: {
    retval.SetFromFont(*wxITALIC_FONT);
    return retval;
  }
  case wxStockGDI::FONT_NORMAL: {
    retval.SetFromFont(*wxNORMAL_FONT);
    return retval;
  }
  case wxStockGDI::FONT_SMALL: {
    retval.SetFromFont(*wxSMALL_FONT);
    return retval;
  }
  case wxStockGDI::FONT_SWISS: {
    retval.SetFromFont(*wxSWISS_FONT);
    return retval;
  }
  default: {
    retval.SetFromFont(*wxNORMAL_FONT);
    return retval;
  }
  }
}

Style::FontVariantCachesMap Style::m_fontCaches;
wxString Style::m_emptyString;

std::ostream& operator<<(std::ostream& out, const TextStyle textstyle){
  std::string result;
  switch(textstyle){
  case TS_CODE_DEFAULT: result = "TS_CODE_DEFAULT"; break;
  case TS_CODE_VARIABLE: result = "TS_CODE_VARIABLE"; break;
  case TS_CODE_FUNCTION: result = "TS_CODE_FUNCTION"; break;
  case TS_CODE_COMMENT: result = "TS_CODE_COMMENT"; break;
  case TS_CODE_NUMBER: result = "TS_CODE_NUMBER"; break;
  case TS_CODE_STRING: result = "TS_CODE_STRING"; break;
  case TS_CODE_OPERATOR: result = "TS_CODE_OPERATOR"; break;
  case TS_CODE_LISP: result = "TS_CODE_LISP"; break;
  case TS_CODE_ENDOFLINE: result = "TS_CODE_ENDOFLINE"; break;
  case TS_ASCIIMATHS: result = "TS_ASCIIMATHS"; break;
  case TS_MATH: result = "TS_MATH"; break;
  case TS_TEXT: result = "TS_TEXT"; break;
  case TS_VARIABLE: result = "TS_VARIABLE"; break;
  case TS_OPERATOR: result = "TS_OPERATOR"; break;
  case TS_NUMBER: result = "TS_NUMBER"; break;
  case TS_FUNCTION: result = "TS_FUNCTION"; break;
  case TS_SPECIAL_CONSTANT: result = "TS_SPECIAL_CONSTANT"; break;
  case TS_GREEK_CONSTANT: result = "TS_GREEK_CONSTANT"; break;
  case TS_STRING: result = "TS_STRING"; break;
  case TS_OUTDATED: result = "TS_OUTDATED"; break;
  case TS_MAIN_PROMPT: result = "TS_MAIN_PROMPT"; break;
  case TS_OTHER_PROMPT: result = "TS_OTHER_PROMPT"; break;
  case TS_LABEL: result = "TS_LABEL"; break;
  case TS_USERLABEL: result = "TS_USERLABEL"; break;
  case TS_HIGHLIGHT: result = "TS_HIGHLIGHT"; break;
  case TS_WARNING: result = "TS_WARNING"; break;
  case TS_ERROR: result = "TS_ERROR"; break;
  case TS_TITLE: result = "TS_TITLE"; break;
  case TS_SECTION: result = "TS_SECTION"; break;
  case TS_SUBSECTION: result = "TS_SUBSECTION"; break;
  case TS_SUBSUBSECTION: result = "TS_SUBSUBSECTION"; break;
  case TS_HEADING5: result = "TS_HEADING5"; break;
  case TS_HEADING6: result = "TS_HEADING6"; break;
  case TS_TEXT_BACKGROUND: result = "TS_TEXT_BACKGROUND"; break;
  case TS_DOCUMENT_BACKGROUND: result = "TS_DOCUMENT_BACKGROUND"; break;
  case TS_CELL_BRACKET: result = "TS_CELL_BRACKET"; break;
  case TS_ACTIVE_CELL_BRACKET: result = "TS_ACTIVE_CELL_BRACKET"; break;
  case TS_CURSOR: result = "TS_CURSOR"; break;
  case TS_SELECTION: result = "TS_SELECTION"; break;
  case TS_EQUALSSELECTION: result = "TS_EQUALSSELECTION"; break;
  default: result = "!!!Bug: Unknown text style!!!";
  }
  return out << result;
}
