// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CLIPBOARD_CLIPBOARD_WIN_H_
#define UI_BASE_CLIPBOARD_CLIPBOARD_WIN_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "crui/base/clipboard/clipboard.h"

namespace cr {
namespace win {
class MessageWindow;
}  // namespace win
}  // namespace cr

namespace crui {

namespace gfx {
class Size;
}  // namespace gfx 

class ClipboardWin : public Clipboard {
 private:
  friend class Clipboard;

  ClipboardWin(const ClipboardWin&) = delete;
  ClipboardWin& operator=(const ClipboardWin&) = delete;

  ClipboardWin();
  ~ClipboardWin() override;

  // Clipboard overrides:
  void OnPreShutdown() override;
  uint64_t GetSequenceNumber(ClipboardBuffer buffer) const override;
  bool IsFormatAvailable(const ClipboardFormatType& format,
                         ClipboardBuffer buffer) const override;
  void Clear(ClipboardBuffer buffer) override;
  void ReadAvailableTypes(ClipboardBuffer buffer,
                          std::vector<cr::string16>* types,
                          bool* contains_filenames) const override;
  std::vector<cr::string16> ReadAvailablePlatformSpecificFormatNames(
      ClipboardBuffer buffer) const override;
  void ReadText(ClipboardBuffer buffer, cr::string16* result) const override;
  void ReadAsciiText(ClipboardBuffer buffer,
                     std::string* result) const override;
  void ReadHTML(ClipboardBuffer buffer,
                cr::string16* markup,
                std::string* src_url,
                uint32_t* fragment_start,
                uint32_t* fragment_end) const override;
  void ReadRTF(ClipboardBuffer buffer, std::string* result) const override;
  ///SkBitmap ReadImage(ClipboardBuffer buffer) const override;
  void ReadCustomData(ClipboardBuffer buffer,
                      const cr::string16& type,
                      cr::string16* result) const override;
  void ReadBookmark(cr::string16* title, std::string* url) const override;
  void ReadData(const ClipboardFormatType& format,
                std::string* result) const override;
  void WritePortableRepresentations(ClipboardBuffer buffer,
                                    const ObjectMap& objects) override;
  void WritePlatformRepresentations(
      ClipboardBuffer buffer,
      std::vector<Clipboard::PlatformRepresentation> platform_representations)
      override;
  void WriteText(const char* text_data, size_t text_len) override;
  void WriteHTML(const char* markup_data,
                 size_t markup_len,
                 const char* url_data,
                 size_t url_len) override;
  void WriteRTF(const char* rtf_data, size_t data_len) override;
  void WriteBookmark(const char* title_data,
                     size_t title_len,
                     const char* url_data,
                     size_t url_len) override;
  void WriteWebSmartPaste() override;
  ///void WriteBitmap(const SkBitmap& bitmap) override;
  void WriteData(const ClipboardFormatType& format,
                 const char* data_data,
                 size_t data_len) override;
  void WriteBitmapFromHandle(HBITMAP source_hbitmap, const gfx::Size& size);

  // Safely write to system clipboard. Free |handle| on failure.
  void WriteToClipboard(unsigned int format, HANDLE handle);

  // Return the window that should be the clipboard owner, creating it
  // if neccessary.  Marked const for lazily initialization by const methods.
  HWND GetClipboardWindow() const;

  // Mark this as mutable so const methods can still do lazy initialization.
  mutable std::unique_ptr<cr::win::MessageWindow> clipboard_owner_;
};

}  // namespace crui

#endif  // UI_BASE_CLIPBOARD_CLIPBOARD_WIN_H_
