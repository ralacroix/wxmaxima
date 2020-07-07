// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2020      Kuba Ober <kuba@bertec.com>
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

#define CATCH_CONFIG_RUNNER
#include "test_ImgCell.h"
#include "Cell.cpp"
#include "CellPtr.cpp"
#include "FontCache.h"
#include "Image.cpp"
#include "ImgCell.cpp"
#include <catch2/catch.hpp>

Cell::CellPointers pointers(nullptr);

Style &Style::operator=(const Style &) { return *this; }
float Style::GetFontSize() const { return {}; }
AFontName Style::Default_FontName() { return {}; }
bool Style::SetItalic(bool) { return {}; }
bool Style::SetFontName(AFontName) { return {}; }
bool Style::SetFontSize(float) { return {}; }
bool Style::IsFontOk() const { return {}; }
Style const &Style::FromStockFont(wxStockGDI::Item)
{
  static const Style s;
  return s;
}
wxFont const &Style::LookupFont() const
{
  static const wxFont font;
  return font;
}

Configuration::Configuration(wxDC *dc) : m_dc(dc) {}
Configuration::~Configuration() {}
long Configuration::Scale_Px(double) const { return 1; }
wxFontStyle Configuration::IsItalic(long) const { return {}; }
wxColour Configuration::GetColor(TextStyle) { return {}; }
Style Configuration::GetStyle(TextStyle, long) const { return {}; }
Cell::CellPointers *Cell::GetCellPointers() const { return &pointers; }

wxBitmap SvgBitmap::RGBA2wxBitmap(unsigned char const *, int const &, int const &) { return {}; }

int ErrorRedirector::m_messages_logPaneOnly;

template <typename C>
wxString HexEncoding(C &&bits)
{
  wxString output;
  for (auto ch : bits)
    output += wxString::Format("%02x", ch);
  return output;
}

SCENARIO("RTF Output represents the image") {
  wxMemoryBuffer image;
  image.AppendData(wxmaxima_art_wxmac_doc_png, wxmaxima_art_wxmac_doc_png_size);
  Configuration config;
  Configuration *pConfig = &config;
  GIVEN("An image with test data") {
    ImgCell cell(nullptr, &pConfig, image, "png");
    WHEN("we convert it to RTF") {
      auto rtf = cell.ToRTF();
      THEN("the RTF output ends in \"}\\n\"")
      REQUIRE(rtf.EndsWith("}\n"));
      THEN("the RTF output contains the hex encoding of the image")
      {
        rtf.Truncate(rtf.size() - 2);
        auto hex = HexEncoding(wxmaxima_art_wxmac_doc_png);
        rtf.erase(0, rtf.size() - hex.size());
        REQUIRE(rtf == hex);
      }
    }
  }
}

// If we don't provide our own main when compiling on MinGW
// we currently get an error message that WinMain@16 is missing
// (https://github.com/catchorg/Catch2/issues/1287)
int main(int argc, char *argv[])
{
  wxEntryStart(argc, argv);
  wxImage::AddHandler(new wxPNGHandler);
  auto rc = Catch::Session().run(argc, argv);
  wxEntryCleanup();
  return rc;
}
