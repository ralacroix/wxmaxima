// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode:
// nil -*-
//
//  Copyright (C) 2007-2015 Andrej Vodopivec <andrej.vodopivec@gmail.com>
//            (C) 2014-2018 Gunter Königsmann <wxMaxima@physikbuch.de>
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
  This file defines the class AnimationCell

  AnimationCell is the Cell type that represents animations.
*/

// 72 points per inch / 96 pixels per inch
#define PRINT_SIZE_MULTIPLIER (72.0 / 96.0)

#include "AnimationCell.h"
#include "CellImpl.h"
#include "CellPointers.h"
#include "ImgCell.h"
#include "StringUtils.h"

#include <memory>
#include <wx/anidecod.h>
#include <wx/clipbrd.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/fs_mem.h>
#include <wx/imaggif.h>
#include <wx/mstream.h>
#include <wx/quantize.h>
#include <wx/utils.h>
#include <wx/wfstream.h>

// filesystem cannot be passed by const reference as we want to keep the
// pointer to the file system alive in a background task
// cppcheck-suppress performance symbolName=filesystem
AnimationCell::AnimationCell(GroupCell *group, Configuration *config,
                             std::shared_ptr<wxFileSystem> filesystem,
                             int framerate)
  : ImgCellBase(group, config),
    m_timer(m_cellPointers->GetWorksheet(), wxNewId()),
    m_fileSystem(filesystem), m_framerate(framerate), m_displayed(0),
    m_imageBorderWidth(Scale_Px(1)) {
  InitBitFields();
  m_type = MC_TYPE_SLIDE;
  ReloadTimer();
}

AnimationCell::AnimationCell(GroupCell *group, Configuration *config,
                             int framerate)
  : ImgCellBase(group, config),
    m_timer(m_cellPointers->GetWorksheet(), wxNewId()),
    m_framerate(framerate), m_displayed(0), m_imageBorderWidth(Scale_Px(1)) {
  InitBitFields();
  m_type = MC_TYPE_SLIDE;
  ReloadTimer();
}

AnimationCell::AnimationCell(GroupCell *group, Configuration *config,
                             const wxMemoryBuffer &image,
                             const wxString &WXUNUSED(type))
  : AnimationCell(group, config) {
  LoadImages(image);
}

AnimationCell::AnimationCell(GroupCell *group, Configuration *config,
                             const wxString &image, bool remove)
  : AnimationCell(group, config) {
  LoadImages(image);
  if (remove)
    wxRemoveFile(image);
}

AnimationCell::AnimationCell(GroupCell *group, const AnimationCell &cell)
  : AnimationCell(group, cell.m_configuration) {
  CopyCommonData(cell);

  m_images.reserve(cell.Length());
  for (std::vector<std::shared_ptr<Image>>::const_iterator i =
	 cell.m_images.begin();
       i != cell.m_images.end(); ++i)
    m_images.push_back(std::make_shared<Image>(cell.m_configuration, **i));

  m_framerate = cell.m_framerate;
  m_displayed = cell.m_displayed;
  m_animationRunning = cell.m_animationRunning;
  m_drawBoundingBox = cell.m_drawBoundingBox;
}

void AnimationCell::SetConfiguration(Configuration *config) {
  m_configuration = config;
  for (std::vector<std::shared_ptr<Image>>::const_iterator i = m_images.begin();
       i != m_images.end(); ++i)
    (*i)->SetConfiguration(config);
}

AnimationCell::~AnimationCell() {
  StopTimer();
  AnimationCell::ClearCache();
}

DEFINE_CELL(AnimationCell)

int AnimationCell::GetFrameRate() const {
  int framerate = 2;

  if (m_framerate > -1)
    framerate = m_framerate;
  else {
    framerate = m_configuration->DefaultFramerate();
  }
  if (framerate > 30)
    framerate = 30;
  if (framerate < 1)
    framerate = 1;
  return (framerate);
}

void AnimationCell::ReloadTimer() {
  if (!m_timer.IsRunning()) {
    // Tell MathCtrl about our timer.
    m_cellPointers->SetTimerIdForCell(this, m_timer.GetId());
    m_timer.StartOnce(1000 / GetFrameRate());
  }
}

void AnimationCell::StopTimer() {
  m_timer.Stop();
  m_cellPointers->RemoveTimerIdForCell(this);
}

void AnimationCell::AnimationRunning(bool run) {
  if (run)
    ReloadTimer();
  else
    StopTimer();
  m_animationRunning = run;
}

int AnimationCell::SetFrameRate(int Freq) {
  m_framerate = Freq;

  if (Freq < 0)
    m_framerate = -1;
  else {
    if (Freq < 1)
      m_framerate = 1;
    if (Freq > 200)
      m_framerate = 200;
  }

  return m_framerate;
}

void AnimationCell::LoadImages(wxMemoryBuffer imageData) {
  wxImage images;
  wxMemoryInputStream istream(imageData.GetData(), imageData.GetDataLen());
  size_t count = wxImage::GetImageCount(istream);

  for (size_t i = 0; i < count; i++) {
    wxMemoryInputStream istream2(imageData.GetData(), imageData.GetDataLen());
    wxImage image;
    image.LoadFile(istream2, wxBITMAP_TYPE_ANY, i);
    m_images.push_back(
		       std::make_shared<Image>(m_configuration, wxBitmap(image)));
  }
}

void AnimationCell::LoadImages(wxString imageFile) {
  wxImage images;
  size_t count = wxImage::GetImageCount(imageFile);

  for (size_t i = 0; i < count; i++) {
    wxImage image;
    image.LoadFile(imageFile, wxBITMAP_TYPE_ANY, i);
    m_images.push_back(
		       std::make_shared<Image>(m_configuration, wxBitmap(image)));
  }
}

void AnimationCell::LoadImages(wxArrayString images, bool deleteRead) {
  wxString gnuplotFilename;
  wxString dataFilename;

  if (images.GetCount() == 1) {
    LoadImages(images[0]);
  } else
    for (size_t i = 0; i < images.GetCount(); i++) {
      if (images[i].EndsWith(".gnuplot"))
        gnuplotFilename = images[i];
      else {
        if (images[i].EndsWith(".data"))
          dataFilename = images[i];
        else {
          m_images.push_back(std::make_shared<Image>(m_configuration, images[i],
                                                     m_fileSystem, deleteRead));
          if (gnuplotFilename != wxEmptyString) {
            if (m_images.back())
              m_images.back()->GnuplotSource(gnuplotFilename, dataFilename);
          }
        }
      }
    }
  m_fileSystem = NULL;
  m_displayed = 0;
}

void AnimationCell::SetDisplayedIndex(int ind) {
  m_displayed = ind;
  if (m_displayed > Length())
    m_displayed = Length() - 1;
  if (m_displayed < 0)
    m_displayed = 0;
}

double AnimationCell::GetMaxWidth() const {
  if (!IsOk())
    return -1;
  else
    return m_images[m_displayed]->GetMaxWidth();
}

double AnimationCell::GetHeightList() const {
  if (!IsOk())
    return -1;
  else
    return m_images[m_displayed]->GetHeightList();
}

void AnimationCell::SetMaxWidth(double width) {
  for (std::vector<std::shared_ptr<Image>>::iterator i = m_images.begin();
       i != m_images.end(); i++)
    (*i)->SetMaxWidth(width);
}

void AnimationCell::SetMaxHeight(double height) {
  for (std::vector<std::shared_ptr<Image>>::iterator i = m_images.begin();
       i != m_images.end(); i++)
    (*i)->SetMaxHeight(height);
}

void AnimationCell::Recalculate(AFontSize fontsize) {
  if (!IsOk()) {
    m_height = m_width = 10;
    m_center = 0;
    return;
  }

  // Assuming a minimum size maybe isn't that bad.
  m_height = m_width = 10;

  // Make the cell as big as the biggest image plus its border.
  for (int i = 0; i < Length(); i++) {
    if (m_images[i] != NULL) {
      if (m_configuration->GetPrinting()) {
        m_images[i]->Recalculate(m_configuration->GetZoomFactor() *
                                 PRINT_SIZE_MULTIPLIER);
      } else {
        m_images[i]->Recalculate();
      }
      if (m_width < m_images[i]->m_width + 2 * m_imageBorderWidth)
        m_width = m_images[i]->m_width + 2 * m_imageBorderWidth;
      if (m_height < m_images[i]->m_height + 2 * m_imageBorderWidth)
        m_height = m_images[i]->m_height + 2 * m_imageBorderWidth;
    }
  }
  m_center = m_height / 2;
  Cell::Recalculate(fontsize);
}

void AnimationCell::Draw(wxPoint point) {
  Cell::Draw(point);
  if (!IsOk())
    return;
  // If the animation leaves the screen the timer is stopped automatically.
  if (m_animationRunning)
    ReloadTimer();

  if (DrawThisCell(point) && (m_images[m_displayed] != NULL)) {
    // Start the timer once the animation appears on the screen.
    // But start it only once: Else the animation could be refreshed
    // more frequent than it can be drawn. Each update of the animation
    // will trigger this function and will trigger the animation to be
    // restarted anyway.
    //
    if (m_configuration->GetPrinting()) {
      m_images[m_displayed]->Recalculate(m_configuration->GetZoomFactor() *
                                         PRINT_SIZE_MULTIPLIER);
    } else {
      m_images[m_displayed]->Recalculate();
    }

    if (!InUpdateRegion())
      return;

    wxDC *dc = m_configuration->GetDC();
    wxMemoryDC bitmapDC;

    // Slide show cells have a red border except if they are selected
    if (m_drawBoundingBox)
      dc->SetPen(*(wxThePenList->FindOrCreatePen(
						 m_configuration->GetColor(TS_SELECTION))));
    else
      dc->SetPen(*wxRED_PEN);

    dc->DrawRectangle(wxRect(point.x, point.y - m_center, m_width, m_height));

    wxBitmap bitmap =
      (m_configuration->GetPrinting()
       ? m_images[m_displayed]->GetBitmap(
					  m_configuration->GetZoomFactor() * PRINT_SIZE_MULTIPLIER)
       : m_images[m_displayed]->GetBitmap());
    bitmapDC.SelectObject(bitmap);

    int imageBorderWidth = m_imageBorderWidth;
    if (m_drawBoundingBox) {
      imageBorderWidth = Scale_Px(3);
      dc->SetBrush(*(wxTheBrushList->FindOrCreateBrush(
						       m_configuration->GetColor(TS_SELECTION))));
      dc->DrawRectangle(wxRect(point.x, point.y - m_center, m_width, m_height));
    }

    dc->Blit(point.x + imageBorderWidth, point.y - m_center + imageBorderWidth,
             m_width - 2 * imageBorderWidth, m_height - 2 * imageBorderWidth,
             &bitmapDC, imageBorderWidth - m_imageBorderWidth,
             imageBorderWidth - m_imageBorderWidth);
  } else
    // The cell isn't drawn => No need to keep it's image cache for now.
    ClearCache();

  // If we need a selection border on another redraw we will be informed by
  // OnPaint() again.
  m_drawBoundingBox = false;
}

wxString AnimationCell::ToString() const { return wxT(" << Animation >> "); }

wxString AnimationCell::ToMatlab() const { return wxT(" << Animation >> "); }

wxString AnimationCell::ToTeX() const { return wxT(" << Graphics >> "); }

wxString AnimationCell::ToXML() const {
  if (!IsOk())
    return wxEmptyString;
  wxString images;
  wxString gnuplotSourceFiles;
  wxString gnuplotDataFiles;

  for (int i = 0; i < Length(); i++) {
    wxString basename = m_cellPointers->WXMXGetNewFileName();
    // add the file to memory
    if (m_images[i]) {
      // Anonymize the name of our temp directory for saving
      wxString gnuplotSource;
      wxString gnuplotData;
      if (m_images[i]->GnuplotData() != wxEmptyString) {
        wxFileName gnuplotDataFile(m_images[i]->GnuplotData());
        gnuplotData = gnuplotDataFile.GetFullName();
      }
      if (m_images[i]->GnuplotSource() != wxEmptyString) {
        wxFileName gnuplotSourceFile(m_images[i]->GnuplotSource());
        gnuplotSource = gnuplotSourceFile.GetFullName();
      }

      // Save the gnuplot source, if necessary.
      if (gnuplotSource != wxEmptyString) {
        gnuplotSourceFiles += gnuplotSource + ".gz;";
        const wxMemoryBuffer data = m_images[i]->GetCompressedGnuplotSource();
        if (data.GetDataLen() > 0)
	  m_configuration->PushFileToSave(gnuplotSource + ".gz", data);
      }
      if (gnuplotData != wxEmptyString) {
        gnuplotDataFiles += gnuplotData + ".gz;";
        const wxMemoryBuffer data = m_images[i]->GetCompressedGnuplotData();
        if (data.GetDataLen() > 0)
	  m_configuration->PushFileToSave(gnuplotData + ".gz", data);
      }

      if (m_images[i]->GetCompressedImage())
	m_configuration->PushFileToSave(basename + m_images[i]->GetExtension(),
					m_images[i]->GetCompressedImage());
    }

    images += basename + m_images[i]->GetExtension() + wxT(";");
  }

  wxString flags;
  flags = " gnuplotSources_gz=\"" + gnuplotSourceFiles + "\"";
  flags += " gnuplotData_gz=\"" + gnuplotDataFiles + "\"";
  if (Length() > 0)
    flags += wxString::Format(wxT(" ppi=\"%i\""), m_images[1]->GetPPI());
  if (HasHardLineBreak())
    flags += wxT(" breakline=\"true\"");
  if (m_animationRunning)
    flags += wxT(" running=\"true\"");
  else
    flags += wxT(" running=\"false\"");
  flags += wxString::Format(wxT(" frame=\"%i\""), m_displayed);

  if (m_framerate > 0)
    flags += wxString::Format(wxT(" fr=\"%i\""), GetFrameRate());
  return wxT("\n<slide") + flags + wxT(">") + images + wxT("</slide>");
}

void AnimationCell::SetPPI(int ppi) {
  for (std::vector<std::shared_ptr<Image>>::const_iterator i = m_images.begin();
       i != m_images.end(); ++i)
    (*i)->SetPPI(ppi);
}

wxSize AnimationCell::ToImageFile(wxString file) {
  return m_images[m_displayed]->ToImageFile(file);
}

wxString AnimationCell::ToRTF() const {
  if (!IsOk())
    return wxEmptyString;

  // Animations aren't supported by RTF so we just export the currently shown
  // image.

  // Lines that are common to all types of images
  wxString header = wxT("{\\pict");
  wxString footer = wxT("}\n");

  // Extract the description of the image data
  wxString image;
  wxMemoryBuffer imgdata;
  if (m_images[m_displayed]->GetExtension().Lower() == wxT("png")) {
    imgdata = m_images[m_displayed]->GetCompressedImage();
    image = wxT("\\pngblip\n");
  } else if ((m_images[m_displayed]->GetExtension().Lower() == wxT("jpg")) ||
             (m_images[m_displayed]->GetExtension().Lower() == wxT("jpeg"))) {
    imgdata = m_images[m_displayed]->GetCompressedImage();
    image = wxT("\\jpegblip\n");
  } else {
    // Convert any non-rtf-enabled format to .png before adding it to the .rtf
    // file.
    image = wxT("\\pngblip\n");
    wxImage imagedata =
      m_images[m_displayed]->GetUnscaledBitmap().ConvertToImage();
    wxMemoryOutputStream stream;
    imagedata.SaveFile(stream, wxBITMAP_TYPE_PNG);
    imgdata.AppendData(stream.GetOutputStreamBuffer()->GetBufferStart(),
                       stream.GetOutputStreamBuffer()->GetBufferSize());
  }

  image += wxString::Format(
			    wxT("\\picw%lu\\pich%lu "),
			    (unsigned long)m_images[m_displayed]->GetOriginalWidth(),
			    (unsigned long)m_images[m_displayed]->GetOriginalHeight());

  // Convert the data into a hexadecimal string
  for (size_t i = 0; i <= imgdata.GetDataLen(); i++)
    image += wxString::Format("%02x", ((unsigned char *)imgdata.GetData())[i]);

  return header + image + footer;
}

const wxString &AnimationCell::GetToolTip(const wxPoint point) const {
  if (!ContainsPoint(point))
    return wxm::emptyString;

  m_cellPointers->m_cellUnderPointer = const_cast<AnimationCell *>(this);
  if (!IsOk())
    return Image::GetBadImageToolTip();

  return GetLocalToolTip();
}

wxSize AnimationCell::ToGif(wxString file) {
  if (!IsOk())
    return wxSize(1, 1);

  // Show a busy cursor as long as we export a .gif file (which might be a
  // lengthy action).
  wxBusyCursor crs;

  wxImageArray gifFrames;

  for (int i = 0; i < Length(); i++) {
    wxImage frame;
    // Reduce the frame to at most 256 colors
    wxQuantize::Quantize(m_images[i]->GetUnscaledBitmap().ConvertToImage(),
                         frame);
    // Gif supports only fully transparent or not transparent at all.
    frame.ConvertAlphaToMask();
    gifFrames.Add(frame);
  }

  wxFile fl(file, wxFile::write);
  if (fl.IsOpened()) {
    wxFileOutputStream outStream(fl);
    if (outStream.IsOk()) {
      wxGIFHandler gif;

      if (gif.SaveAnimation(gifFrames, &outStream, true, 1000 / GetFrameRate()))
        return wxSize(m_images[1]->GetOriginalWidth(),
                      m_images[1]->GetOriginalHeight());
    }
  }
  return wxSize(-1, -1);
}

void AnimationCell::ClearCache() {
  for (int i = 0; i < Length(); i++)
    if (m_images[i] != NULL)
      m_images[i]->ClearCache();
}

AnimationCell::GifDataObject::GifDataObject(const wxMemoryOutputStream &str)
  : wxCustomDataObject(m_gifFormat) {
  SetData(str.GetOutputStreamBuffer()->GetBufferSize(),
          str.GetOutputStreamBuffer()->GetBufferStart());
}

bool AnimationCell::CopyToClipboard() const {
  if (!IsOk())
    return false;
  wxASSERT_MSG(!wxTheClipboard->IsOpened(),
               _("Bug: The clipboard is already opened"));
  if (wxTheClipboard->Open()) {
    bool res = wxTheClipboard->SetData(
				       new wxBitmapDataObject(m_images[m_displayed]->GetUnscaledBitmap()));
    wxTheClipboard->Close();
    return res;
  }
  return false;
}

bool AnimationCell::IsOk() const {
  if (Length() < 1)
    return false;
  if (m_displayed >= Length())
    return false;
  if (m_displayed < 0)
    return false;
  if (!m_images[m_displayed])
    return false;
  if (!m_images[m_displayed]->IsOk())
    return false;
  return true;
}

bool AnimationCell::CopyAnimationToClipboard() {
  if (!IsOk())
    return false;

  if (wxTheClipboard->Open()) {
    // Show a busy cursor as long as we export a .gif file (which might be a
    // lengthy action).
    wxBusyCursor crs;

    wxImageArray gifFrames;

    for (int i = 0; i < Length(); i++) {
      wxImage frame;
      // Reduce the frame to at most 256 colors
      wxQuantize::Quantize(m_images[i]->GetUnscaledBitmap().ConvertToImage(),
                           frame);
      // Gif supports only fully transparent or not transparent at all.
      frame.ConvertAlphaToMask();
      gifFrames.Add(frame);
    }

    wxMemoryOutputStream stream;
    wxGIFHandler gif;
    if (!gif.SaveAnimation(gifFrames, &stream, true, 1000 / GetFrameRate()))
      return false;

    GifDataObject *clpbrdObj = new GifDataObject(stream);
    bool res = wxTheClipboard->SetData(clpbrdObj);
    wxTheClipboard->Close();

    return res;
  }
  return false;
}

wxDataFormat AnimationCell::m_gifFormat(wxT("image/gif"));
