// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
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
 * \file This file implements the text style system.
 */

#define wxNO_UNSAFE_WXSTRING_CONV
#include "TextStyle.h"
#include "FontCache.h"
#include <wx/fontenum.h>
#include <wx/log.h>
#include <wx/thread.h>
#include <wx/translation.h>
#include <array>
#include <list>
#include <vector>

/*! \brief Mixes two hashes together.
 *
 * Used to obtain hashes of composite data types, such as structures.
*/
template <typename T>
static size_t MixHash(size_t seed, const T &value)
{
  std::hash<T> const hasher;
  seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  return seed;
}

//! Comparator for wxString*
static bool operator<(const wxString *l, const wxString &r) { return *l < r; }

/*! \brief An implementation of string interning arena.
 */
template <typename T>
class Interner
{
  struct Slice {
    size_t nextIndex = 0;
    std::array<T, 128> data;
    T* GetNext() {
      if (nextIndex >= data.size()) return nullptr;
      return &(data[nextIndex++]);
    }
  };

  std::list<Slice> m_storage;
  std::vector<const T*> m_index;
public:
  bool IsInterned(const T *value) const
  {
    for (auto const &slice : m_storage)
    {
      auto const &data = slice.data;
      auto const nextIndex = slice.nextIndex;
      if (nextIndex)
      {
        auto *const first = &(data[0]);
        auto *const last = &(data[nextIndex-1]);
        if (value >= first && value <= last)
        {
          wxLogDebug("IsInterned %p", value);
          return true;
        }
      }
    }
    return false;
  }
  const T* Intern(const T *value) {
    if (!value) return nullptr;
    return &Intern(*value);
  }
  const T* Intern(const T &value)
  {
    // pointer equality: is the value within one of the slices?
    if (IsInterned(&value))
      return &value;
    // value equality: is the value equal to one already interned?
    auto indexPos = std::lower_bound(m_index.begin(), m_index.end(), value);
    if (indexPos != m_index.end() && **indexPos == value)
      return *indexPos;
    // none of the above: we must intern
    if (m_storage.empty()) m_storage.emplace_back();
    T *loc = m_storage.back().GetNext();
    if (!loc)
    {
      m_storage.emplace_back();
      loc = m_storage.back().GetNext();
    }
    *loc = value;
    indexPos = m_index.insert(indexPos, loc);
    wxASSERT(*indexPos && *indexPos == loc);
    return *indexPos;
  }
};

/*! \brief Synchronizes access to the font face string interner.
 *
 * This is only active on Windows. The class is empty on non-windows systems, where
 * access to the font objects is only allowed from the main thread.
 */
struct InternerLock
{
#ifdef __WINDOWS__
  // Windows allows font access from multiple threads, as long as each font
  // is built separately. We must thus synchronize the access to the interner.
  static wxMutex mutex;
  InternerLock() { mutex.Lock(); }
  ~InternerLock() { mutex.Unlock(); }
  InternerLock(const InternerLock &) = delete;
  void operator=(const InternerLock &) = delete;
#endif
};

#ifdef __WINDOWS__
//! The mutex is only for Windows, where fonts can be accessed from multiple threads.
//! On other systems, this is not allowed.
wxMutex InternerLock::mutex;
#endif

static Interner<wxString>& GetFontNameInterner()
{
  static Interner<wxString> interner;
  return interner;
}

const wxString *AFontName::Intern(const wxString &str)
{
  // cppcheck-suppress unusedVariable
  InternerLock lock;
  return GetFontNameInterner().Intern(str);
}

const wxString *AFontName::GetInternedEmpty()
{
  static const wxString *internedEmpty = AFontName::Intern(wxEmptyString);
  return internedEmpty;
}

/*
 * Style
 */

Style::Style(const Style &o) : m(o.m) {}

Style &Style::operator=(const Style &o)
{
  if (&o != this)
    m = o.m;

  return *this;
}

wxFontFamily Style::GetFamily() const
{ return wxFontFamily(m.family); }

wxFontEncoding Style::GetEncoding() const
{ return wxFontEncoding(m.encoding); }

wxFontWeight Style::GetWeight() const
{ return wxFontWeight(m.weight); }

wxFontStyle Style::GetFontStyle() const
{ return wxFontStyle(m.fontStyle); }

bool Style::IsUnderlined() const
{ return m.underlined; }

bool Style::IsStrikethrough() const
{ return m.strikethrough; }

AFontName Style::GetFontName() const
{ return m.fontName; }

const wxString &Style::GetNameStr() const
{ return m.fontName.GetAsString(); }

float Style::GetFontSize() const
{
  auto fontSize = m.uFontSize * FontSize_Unit;
  wxASSERT(fontSize > 0);
  return fontSize;
}

uint32_t Style::GetRGBColor() const
{ return m.rgbColor; }

using did_change = Style::did_change;

did_change Style::SetFamily(wxFontFamily family)
{
  if (m.family == family) return false;
  m.family = family;
  m.fontHash = 0;
  return true;
}

did_change Style::SetEncoding(wxFontEncoding encoding)
{
  if (m.encoding == encoding) return false;
  m.encoding = encoding;
  m.fontHash = 0;
  return true;
}

did_change Style::SetFontStyle(wxFontStyle fontStyle)
{
  if (m.fontStyle == fontStyle) return false;
  m.fontStyle = fontStyle;
  m.fontHash = 0;
  return true;
}

did_change Style::SetWeight(int weight)
{
  if (m.weight == weight) return false;
  m.weight = weight;
  m.fontHash = 0;
  return true;
}

did_change Style::SetBold(bool bold)
{
  return SetWeight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
}

did_change Style::SetLight(bool light)
{
  return SetWeight(light ? wxFONTWEIGHT_LIGHT : wxFONTWEIGHT_NORMAL);
}

did_change Style::SetItalic(bool italic)
{
  return SetFontStyle(italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
}

did_change Style::SetSlant(bool slant)
{
  return SetFontStyle(slant ? wxFONTSTYLE_SLANT : wxFONTSTYLE_NORMAL);
}

did_change Style::SetUnderlined(bool underlined)
{
  if (m.underlined == underlined) return false;
  m.underlined = underlined;
  m.fontHash = 0;
  return true;
}

did_change Style::SetStrikethrough(bool strikethrough)
{
  if (m.strikethrough == strikethrough) return false;
  m.strikethrough = strikethrough;
  m.fontHash = 0;
  return true;
}

did_change Style::SetFontName(AFontName faceName)
{
  if (m.fontName == faceName) return false;
  m.fontName = faceName;
  m.fontHash = 0;
  return true;
}

did_change Style::SetFontSize(float size)
{
  auto const uSize = lround(size / FontSize_Unit);
  return SetUFontSize(uSize);
}

did_change Style::SetUFontSize(int16_t uSize)
{
  if (m.uFontSize == uSize) return false;
  m.uFontSize = uSize;
  m.fontHash = 0;
  return true;
}

did_change Style::SetRGBColor(uint32_t rgb)
{
  if (m.rgbColor == rgb) return false;
  m.rgbColor = rgb;
  return true;
}

did_change Style::SetColor(const wxColor &color)
{
  return SetRGBColor(color.GetRGB());
}

did_change Style::SetColor(wxSystemColour sysColour)
{
  return SetColor(wxSystemSettings::GetColour(sysColour));
}

did_change Style::ResolveToFont()
{
  Style fromFont = HasFontCached()
                     ? Style().FromFontNoCache(*m.font)
                     : FontCache::GetAStyleFont(*this).first;
  return SetFontFrom(fromFont);
}

did_change Style::SetFromFont(const wxFont &font)
{ return SetFontFrom(Style::FromFont(font)); }

did_change Style::SetFontFrom(const Style &o)
{
  bool changed =
    SetFontFaceAndSizeFrom(o) |
    SetFontStyle(o.GetFontStyle()) |
    SetWeight(o.GetWeight()) |
    SetUnderlined(o.IsUnderlined()) |
    SetStrikethrough(o.IsStrikethrough());
  if (GetFontHash() == o.GetFontHash())
    m.font = o.m.font;
  return changed;
}

did_change Style::SetFontFaceFrom(const Style &o)
{
  return
    SetFontName(o.GetFontName()) |
    SetEncoding(o.GetEncoding()) |
    SetFamily(o.GetFamily());
}

did_change Style::SetFontFaceAndSizeFrom(const Style &o)
{
  return SetFontFaceFrom(o) | SetUFontSize(o.m.uFontSize);
}

size_t Style::GetFontHash() const
{
  size_t hash_ = m.fontHash;
  if (!hash_)
  {
    hash_ = MixHash(hash_, m.family);
    hash_ = MixHash(hash_, m.encoding);
    hash_ = MixHash(hash_, m.weight);
    hash_ = MixHash(hash_, m.fontStyle);
    hash_ = MixHash(hash_, m.underlined << 1 | m.strikethrough << 3 | m.isNotOK << 5);
    hash_ = MixHash(hash_, m.fontName);
    hash_ = MixHash(hash_, m.uFontSize);
    if (!hash_) hash_++;
    m.fontHash = hash_;
    m.font = 0;
  }
  return hash_;
}

bool Style::IsFontEqualTo(const Style &o_) const
{
  const Data &o = o_.m;
  if (m.font && m.font == o.font) return true;
  return
    (!m.fontHash || !o.fontHash || m.fontHash == o.fontHash) &&
    m.uFontSize == o.uFontSize &&
    m.family == o.family &&
    m.encoding == o.encoding &&
    m.weight == o.weight &&
    m.fontStyle == o.fontStyle &&
    m.fontName == o.fontName &&
    m.underlined == o.underlined &&
    m.strikethrough == o.strikethrough &&
    m.isNotOK == o.isNotOK;
}

bool Style::IsStyleEqualTo(const Style &o) const
{
  return
    this->IsFontEqualTo(o) &&
    m.rgbColor == o.m.rgbColor;
}

const wxFont& Style::LookupFont() const
{
  GetFontHash();
  auto &styleFont = FontCache::GetAStyleFont(*this);
  m.font = &styleFont.second;
  wxASSERT(m.font);
  return *m.font;
}

bool Style::IsFontOk() const
{
  return m.isNotOK ? false : GetFont().IsOk();
}

const wxFont& Style::GetFontAt(float fontSize) const
{
  auto const uFontSize = lround(fontSize / FontSize_Unit);
  if (uFontSize == m.uFontSize) return GetFont();
  return Style(*this).FontSize(fontSize).GetFont();
}

Style &Style::FromFontNoCache(const wxFont &font)
{
  this->SetFromFontNoCache(font);
  return *this;
}

void Style::SetFromFontNoCache(const wxFont &font)
{
  m.fontHash = 0;
  if (font.IsOk())
  {
    m.encoding = font.GetEncoding();
    m.family = font.GetFamily();
    m.fontStyle = font.GetStyle();
    m.underlined = font.GetUnderlined();
    m.strikethrough = font.GetStrikethrough();
#if wxCHECK_VERSION(3,1,2)
    m.weight = font.GetNumericWeight();
#else
    m.weight = font.GetWeight();
#endif
    m.fontName = AFontName(font.GetFaceName());
    m.uFontSize = lround(GetFontSize(font) / FontSize_Unit);
    GetFontHash();
  }
  else
    m.isNotOK = true;
}

bool Style::IsFractionalFontSizeSupported()
{
  return wxCHECK_VERSION(3,1,2);
}

float Style::GetFontSize(const wxFont &font)
{
#if wxCHECK_VERSION(3,1,2)
  return font.GetFractionalPointSize();
#endif
  return font.GetPointSize();
}

void Style::SetFontSize(wxFont &font, float fontSize)
{
#if wxCHECK_VERSION(3,1,2)
  return font.SetFractionalPointSize(fontSize);
#endif
  return font.SetPointSize(fontSize);
}

wxFontInfo Style::GetAsFontInfo() const
{
  wxFontInfo result(GetFontSize());

  result
    .Family(GetFamily())
    .FaceName(GetFontName())
    .Underlined(IsUnderlined())
    .Strikethrough(IsStrikethrough())
    .Encoding(GetEncoding())
    ;

  // This pattern is used to ensure that the legacy variant
  // still compiles (doesn't bitrot).
#if wxCHECK_VERSION(3,1,2)
  return result
    .Style(GetFontStyle())
    .Weight(GetWeight())
    ;
#endif
  return result
    .Slant(IsSlant())
    .Italic(IsItalic())
    .Bold(IsBold())
    .Light(IsLight())
    ;
}

AFontName Style::Default_FontName()
{
#if defined(__WXOSX_MAC__)
  static auto fontName = AFontName::Monaco();
#elif defined(__WINDOWS__)
  static auto fontName = AFontName::Linux_Libertine_O();
#else
  static auto fontName = AFontName::Arial();
#endif
  return fontName;
}

static const wxString k_color = wxT("%s/color");
static const wxString k_bold = wxT("%s/bold");
static const wxString k_light = wxT("%s/light");
static const wxString k_italic = wxT("%s/italic");
static const wxString k_slant = wxT("%s/slant");
static const wxString k_underlined = wxT("%s/underlined");
static const wxString k_strikethrough = wxT("%s/strikethrough");
static const wxString k_fontsize = wxT("%s/Style/Text/fontsize");
static const wxString k_fontname = wxT("%s/Style/Text/fontname");

Style &Style::Read(wxConfigBase *config, const wxString &where)
{
  wxString tmpStr;
  bool tmpBool;
  long tmpLong;

  if (config->Read(wxString::Format(k_color, where), &tmpStr))
  {
    wxColor color = wxColor(tmpStr);
    if (color.IsOk()) SetColor(color);
  }
  if (config->Read(wxString::Format(k_bold, where), &tmpBool) && tmpBool) SetBold(true);
  else if (config->Read(wxString::Format(k_light, where), &tmpBool) && tmpBool) SetLight(true);
  if (config->Read(wxString::Format(k_italic, where), &tmpBool) && tmpBool) SetItalic(true);
  else if (config->Read(wxString::Format(k_slant, where), &tmpBool) && tmpBool) SetSlant(true);
  if (config->Read(wxString::Format(k_underlined, where), &tmpBool)) SetUnderlined(tmpBool);
  if (config->Read(wxString::Format(k_strikethrough, where), &tmpBool)) SetStrikethrough(tmpBool);
  if (config->Read(wxString::Format(k_fontsize, where), &tmpLong))
    SetFontSize(tmpLong);
  if (config->Read(wxString::Format(k_fontname, where), &tmpStr) && !tmpStr.empty())
    SetFontName(AFontName(tmpStr));

  // Validation is deferred to the point of first use, etc.
  return *this;
}

void Style::Write(wxConfigBase *config, const wxString &where) const
{
  config->Write(wxString::Format(k_color, where), GetColor().GetAsString());
  config->Write(wxString::Format(k_bold, where), IsBold());
  config->Write(wxString::Format(k_italic, where), IsItalic());
  config->Write(wxString::Format(k_underlined, where), IsUnderlined());
  config->Write(wxString::Format(k_fontsize, where), long(GetFontSize()));
  config->Write(wxString::Format(k_fontname, where), GetNameStr());

  // We don't write the slant, light nor strikethrough attributes so as not to grow the
  // configuration compared to the previous releases. The slant and strikethrough are only
  // emitted when set or when previously set.
  auto const optWrite = [](wxConfigBase *config, const wxString &keyFormat,
                           const wxString &where, auto condition)
  {
    decltype(condition) tempVal;
    auto path = wxString::Format(keyFormat, where);
    if (condition != decltype(condition){} || config->Read(path, &tempVal))
      config->Write(path, condition);
  };

  optWrite(config, k_slant, where, IsSlant());
  optWrite(config, k_light, where, IsLight());
  optWrite(config, k_strikethrough, where, IsStrikethrough());
}

const Style &Style::FromFont(const wxFont &font)
{
  wxASSERT_MSG(
    &font != wxITALIC_FONT
      && &font != wxNORMAL_FONT
      && &font != wxSMALL_FONT
      && &font != wxSWISS_FONT, "Use Style::FromStockFont to get stock fonts!");

  return FontCache::AddAFont(font);
}

const Style &Style::FromStockFont(wxStockGDI::Item font)
{
  static const auto getStyleFor = [](const wxFont *font){
    auto style = FontCache::AddAFont(*font);
    style.GetFontHash();
    style.m.font = font; // Pre-cache the stock font in the style itself
    return style;
  };

  switch (font)
  {
  case wxStockGDI::FONT_ITALIC:
  {
    static Style italic = getStyleFor(wxITALIC_FONT);
    return italic;
  }
  case wxStockGDI::FONT_NORMAL:
  {
    static Style normal = getStyleFor(wxNORMAL_FONT);
    return normal;
  }
  case wxStockGDI::FONT_SMALL:
  {
    static Style small_ = getStyleFor(wxSMALL_FONT);
    return small_;
  }
  case wxStockGDI::FONT_SWISS:
  {
    static Style swiss = getStyleFor(wxSWISS_FONT);
    return swiss;
  }
  default:
  {
    static Style defaultStyle;
    return defaultStyle;
  }
  }
}

static bool TrySetFontName(Style &style, AFontName name)
{
  Style newStyle = style;
  if (!wxFontEnumerator::IsValidFacename(name.GetAsString()))
    return false;
  newStyle.SetFontName(name);
  if (!newStyle.IsFontOk())
    return false;
  style = newStyle;
  return true;
}

wxString Style::GetDump() const
{
  return wxString::Format("%5.2fpt %c%c%c%c%c \"%s\" fam:%d enc:%d",
             GetFontSize(),
             m.isNotOK ? '!' : ' ',
             IsBold() ? 'B' : IsLight() ? 'L' : '-',
             IsItalic() ? 'I' : IsSlant() ? 'S' : '-',
             IsUnderlined() ? 'U' : '-',
             IsStrikethrough() ? 'T' : '-',
             GetNameStr(),
             GetFamily(),
             GetEncoding());
}
