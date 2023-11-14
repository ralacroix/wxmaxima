// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2004-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//            (C) 2014-2016 Gunter Königsmann <wxMaxima@physikbuch.de>
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

#ifndef NAMEDBOXCELL_H
#define NAMEDBOXCELL_H

#include <memory>
#include <utility>
#include "Cell.h"
#include "TextCell.h"

/*! \file

  This file defines the class for the cell type that represents a box(x) block with two arguments.
*/

/*! A cell that represents a box(x) block

  In the case that this cell is broken into two lines in the order of
  m_nextToDraw this cell is represented by the following individual
  cells:

  - The NamedBoxCell itself
  - The opening "box("
  - The contents
  - The closing ")".

  If it isn't broken into multiple cells m_nextToDraw points to the
  cell that follows this Cell.
*/
class NamedBoxCell final : public Cell
{
public:
  NamedBoxCell(GroupCell *group, Configuration *config, std::unique_ptr<Cell> &&inner,
               wxString name);
  NamedBoxCell(GroupCell *group, const NamedBoxCell &cell);
  std::unique_ptr<Cell> Copy(GroupCell *group) const override;
  const CellTypeInfo &GetInfo() override;

  size_t GetInnerCellCount() const override { return 5; }
  // cppcheck-suppress objectIndex
  Cell *GetInnerCell(size_t index) const override { return (&m_open)[index].get(); }

  bool BreakUp() override;

  void SetNextToDraw(Cell *next) override;

private:
  void MakeBreakupCells();

  // The pointers below point to inner cells and must be kept contiguous.
  // ** This is the draw list order. All pointers must be the same:
  // ** either Cell * or std::unique_ptr<Cell>. NO OTHER TYPES are allowed.
  std::unique_ptr<Cell> m_open;
  std::unique_ptr<Cell> m_innerCell;
  std::unique_ptr<TextCell> m_comma;
  std::unique_ptr<TextCell> m_boxname;
  std::unique_ptr<Cell> m_close;
  // The pointers above point to inner cells and must be kept contiguous.

//** Bitfield objects (0 bytes)
//**
  static void InitBitFields()
    { // Keep the initialization order below same as the order
      // of bit fields in this class!
    }

  void Recalculate(AFontSize fontsize) override;

  void Draw(wxPoint point, wxDC *dc, wxDC *antialiassingDC) override;

  wxString ToMathML() const override;
  wxString ToMatlab() const override;
  wxString ToOMML() const override;
  wxString ToString() const override;
  wxString ToTeX() const override;
  wxString ToXML() const override;
  int m_innerCellWidth = -1;
  int m_innerCellHeight = -1;
  int m_nameWidth = -1;
  int m_nameHeight = -1;
};

#endif // NAMEDBOXCELL_H
