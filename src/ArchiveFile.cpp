/*
 *      Copyright (C) 2017 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "p8-platform/threads/mutex.h"
#include <kodi/addon-instance/VFS.h>
#include <kodi/General.h>

extern "C" {
#include <archive.h>
#include <archive_entry.h>
}


static std::string URLEncode(const std::string& strURLData)
{
  std::string strResult;

  /* wonder what a good value is here is, depends on how often it occurs */
  strResult.reserve( strURLData.length() * 2 );

  for (size_t i = 0; i < strURLData.size(); ++i)
  {
    const unsigned char kar = strURLData[i];

    // Don't URL encode "-_.!()" according to RFC1738
    //! @todo Update it to "-_.~" after Gotham according to RFC3986
    if (std::isalnum(kar) || kar == '-' || kar == '.' || kar == '_' || kar == '!' || kar == '(' || kar == ')')
      strResult.push_back(kar);
    else
    {
      char temp[128];
      sprintf(temp,"%%%2.2X", (unsigned int)((unsigned char)kar));
      strResult += temp;
    }
  }

  return strResult;
}


class CArchiveFile
  : public kodi::addon::CInstanceVFS
{
  struct ArchiveCtx
  {
    struct archive* ar = nullptr;
    struct archive_entry* entry = nullptr;
    int64_t pos = 0;
  };
public:
  CArchiveFile(KODI_HANDLE instance) : CInstanceVFS(instance) { }

  virtual void* Open(const VFSURL& url) override
  {
    ArchiveCtx* ctx = new ArchiveCtx;
    ctx->ar = archive_read_new();
    archive_read_support_filter_all(ctx->ar);
    archive_read_support_format_all(ctx->ar);
    int r = archive_read_open_filename(ctx->ar, url.hostname, 10240);
    if (r != ARCHIVE_OK)
    {
      archive_read_free(ctx->ar);
      delete ctx;
      return nullptr;
    }

    while (archive_read_next_header(ctx->ar, &ctx->entry) == ARCHIVE_OK)
    {
      std::string name = archive_entry_pathname_utf8(ctx->entry);
      if (name == url.filename)
        return ctx;

      archive_read_data_skip(ctx->ar);
    }

    archive_read_free(ctx->ar);
    delete ctx;
    return nullptr;
  }

  virtual ssize_t Read(void* context, void* buffer, size_t uiBufSize) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);
    if (!ctx || !ctx->ar)
      return 0;

    ssize_t read = archive_read_data(ctx->ar, buffer, uiBufSize);
    if (read > 0)
      ctx->pos += read;

    return read;
  }

  virtual int64_t Seek(void* context, int64_t position, int whence) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);

    if (!ctx || !ctx->ar)
      return -1;

    ctx->pos = archive_seek_data(ctx->ar, position, whence);
    return ctx->pos;
  }

  virtual int64_t GetLength(void* context) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);
    if (!ctx || !ctx->ar)
      return -1;

    return archive_entry_size(ctx->entry);
  }

  virtual int64_t GetPosition(void* context) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);
    if (!ctx || !ctx->ar)
      return -1;

    return ctx->pos;
  }

  virtual int IoControl(void* context, XFILE::EIoControl request, void* param) override
  {
    return -1;
  }

  virtual int Stat(const VFSURL& url, struct __stat64* buffer) override
  {
    return -1;
  }

  virtual bool Close(void* context) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);
    if (!ctx)
      return true;

    if (ctx->ar)
      archive_read_free(ctx->ar);
    delete ctx;
    return true;
  }

  virtual bool Exists(const VFSURL& url) override
  {
    return false;
  }

  virtual bool DirectoryExists(const VFSURL& url) override
  {
    return false;
  }
  
  virtual bool GetDirectory(const VFSURL& url, 
                            std::vector<kodi::vfs::CDirEntry>& items,
                            CVFSCallbacks callbacks) override
  {
    struct archive* ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);
    int r = archive_read_open_filename(ar, url.hostname, 10240);
    if (r != ARCHIVE_OK)
      return false;

    ListArchive(ar, url.url, items);
    archive_read_free(ar);

    return !items.empty();
  }
  
  virtual bool ContainsFiles(const VFSURL& url, 
                             std::vector<kodi::vfs::CDirEntry>& items,
                             std::string& rootpath) override
  {
    std::string encoded = URLEncode(url.url);
    rootpath = "archive://"+encoded + "/";

    struct archive* ar = archive_read_new();
    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);
    int r = archive_read_open_filename(ar, url.url, 10240);
    if (r != ARCHIVE_OK)
      return false;

    ListArchive(ar, rootpath, items);
    archive_read_free(ar);

    return !items.empty();
  }
private:
  void ListArchive(struct archive* ar, const std::string& rootpath,
                   std::vector<kodi::vfs::CDirEntry>& items)
  {
    struct archive_entry* entry;
    while (archive_read_next_header(ar, &entry) == ARCHIVE_OK)
    {
      kodi::vfs::CDirEntry kentry;  
      std::string name = archive_entry_pathname_utf8(entry);
      kentry.SetLabel(name);
      kentry.SetPath(rootpath+name);
      kentry.SetTitle(name);
      kentry.SetSize(archive_entry_size(entry));
      kentry.SetDateTime(archive_entry_mtime(entry));
      items.push_back(kentry);
      archive_read_data_skip(ar);
    }
  }
};

class CMyAddon : public kodi::addon::CAddonBase
{
public:
  CMyAddon() { }
  virtual ADDON_STATUS CreateInstance(int instanceType, std::string instanceID, KODI_HANDLE instance, KODI_HANDLE& addonInstance) override
  {
    addonInstance = new CArchiveFile(instance);
    return ADDON_STATUS_OK;
  }
};

ADDONCREATOR(CMyAddon);
