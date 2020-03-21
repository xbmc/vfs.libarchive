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

#include <kodi/addon-instance/VFS.h>
#include <kodi/Filesystem.h>
#include <kodi/General.h>
#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <set>
#include <string>

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
      sprintf(temp,"%%%2.2x", (unsigned int)((unsigned char)kar));
      strResult += temp;
    }
  }

  return strResult;
}

class ATTRIBUTE_HIDDEN CArchiveFile : public kodi::addon::CInstanceVFS
{
  struct CbData
  {
    std::string url;
    kodi::vfs::CFile file;
    std::vector<uint8_t> buff;

    CbData(const std::string& path) : url(path) {}
  };

  struct ArchiveCtx
  {
    struct archive* ar = nullptr;
    struct archive_entry* entry = nullptr;
    int64_t pos = 0;
    std::vector<CbData> cbs;
    kodi::vfs::CFile file;

    bool Open(const std::string& url)
    {
      // check for multi-volume rar
      bool oldStyle = false;
      std::regex ext_re("(.+[/\\\\])(.+\\.rar)$");
      std::smatch match;
      if (std::regex_match(url, match, ext_re))
      {
        std::regex part_re("(.+\\.part)[0-9]+\\.rar$");
        std::smatch match2;
        bool checkDir = false;
        std::string fname = match[2].str();
        if (std::regex_match(fname, match2, part_re))
        {
          checkDir = true;
          fname = match2[2].str();
        }
        else
        {
          std::string nurl(url);
          nurl[nurl.size()-2] = '0';
          nurl[nurl.size()-1] = '0';
          if (kodi::vfs::FileExists(nurl.c_str(), true))
          {
            checkDir = true;
            oldStyle = true;
            fname = match[2].str();
            fname.erase(fname.size()-3);
          }
        }

        if (checkDir)
        {
          std::vector<kodi::vfs::CDirEntry> items;
          kodi::vfs::GetDirectory(match[1].str(), "", items);
          std::regex fname_re(".*\\.r(ar|[0-9]+)$");
          for (auto& it : items)
          {
            if (it.Label().find(fname) != std::string::npos &&
                std::regex_match(it.Label(), fname_re))
              cbs.emplace_back(CbData(it.Path()));
          }
        }
      }

      if (cbs.empty())
        cbs.emplace_back(CbData(url));

      auto&& CbSort = [](const CbData& d1, const CbData& d2) -> bool
                      {
                        return d2.url.compare(d1.url) > 0;
                      };

      std::sort(cbs.begin(), cbs.end(), CbSort);
      if (oldStyle)
      {
        cbs.insert(cbs.begin(), cbs.back());
        cbs.pop_back();
      }

      ar = archive_read_new();
      archive_read_support_filter_all(ar);
      archive_read_support_format_all(ar);
      // TODO: Probe VFS for seekability
      archive_read_set_seek_callback(ar, ArchiveSeek);
      archive_read_set_read_callback(ar, ArchiveRead);
      archive_read_set_close_callback(ar, ArchiveClose);
      archive_read_set_switch_callback(ar, ArchiveSwitch);
      archive_read_set_open_callback(ar, ArchiveOpen);

      for (auto& it : cbs)
        archive_read_append_callback_data(ar, &it);

      if (archive_read_open1(ar) != ARCHIVE_OK)
      {
        archive_read_free(ar);
        return false;
      }

      return true;
    }
  };

public:
  static int ArchiveOpen(archive* a, void* client_data)
  {
    CbData* ctx = static_cast<CbData*>(client_data);
    if (!ctx->file.OpenFile(ctx->url))
      return ARCHIVE_FATAL;

    size_t chunk = ctx->file.GetChunkSize();
    ctx->buff.resize(chunk ? chunk : 10240);

    return ARCHIVE_OK;
  }

  static int ArchiveSwitch(archive* a, void* client_data1, void* client_data2)
  {
    ArchiveClose(a, client_data1);
    return ArchiveOpen(a, client_data2);
  }

  //! \brief Read callback for VFS.
  static la_ssize_t ArchiveRead(struct archive*,
                                void* client_data, const void** buff)
  {
    CbData* ctx = static_cast<CbData*>(client_data);
    *buff = ctx->buff.data();
    la_ssize_t read = ctx->file.Read(ctx->buff.data(), ctx->buff.size());
    return read;
  }

  //! \brief Seek callback for VFS.
  static la_int64_t ArchiveSeek(struct archive*, void* client_data,
                                la_int64_t offset, int whence)
  {
    CbData* ctx = static_cast<CbData*>(client_data);
    return ctx->file.Seek(offset, whence);
  }

  //! \brief Close callback for VFS.
  static int ArchiveClose(struct archive*, void* client_data)
  {
    CbData* ctx = static_cast<CbData*>(client_data);
    ctx->file.Close();
    ctx->buff.clear();

    return ARCHIVE_OK;
  }

  CArchiveFile(KODI_HANDLE instance) : CInstanceVFS(instance) { }

  void* Open(const VFSURL& url) override
  {
    ArchiveCtx* ctx = new ArchiveCtx;
    if (!ctx->Open(url.hostname))
    {
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

  ssize_t Read(void* context, void* buffer, size_t uiBufSize) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);
    if (!ctx || !ctx->ar)
      return 0;

    ssize_t read = 0;
    while (1)
    {
      read = archive_read_data(ctx->ar, buffer, uiBufSize);
      if (read == ARCHIVE_RETRY)
        continue;
      if (read > 0)
        ctx->pos += read;
      break;
    }

    return read;
  }

  int64_t Seek(void* context, int64_t position, int whence) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);

    if (!ctx || !ctx->ar)
      return -1;

    ctx->pos = archive_seek_data(ctx->ar, position, whence);
    return ctx->pos;
  }

  int64_t GetLength(void* context) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);
    if (!ctx || !ctx->ar)
      return -1;

    return archive_entry_size(ctx->entry);
  }

  int64_t GetPosition(void* context) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);
    if (!ctx || !ctx->ar)
      return -1;

    return ctx->pos;
  }

  int IoControl(void* context, VFS_IOCTRL request, void* param) override
  {
    return -1;
  }

  int Stat(const VFSURL& url, struct __stat64* buffer) override
  {
    return -1;
  }

  bool Close(void* context) override
  {
    ArchiveCtx* ctx = static_cast<ArchiveCtx*>(context);
    if (!ctx)
      return true;

    if (ctx->ar)
      archive_read_free(ctx->ar);
    delete ctx;
    return true;
  }

  bool Exists(const VFSURL& url) override
  {
    ArchiveCtx* ctx = new ArchiveCtx;
    if (!ctx->Open(url.hostname))
    {
      delete ctx;
      return false;
    }

    std::string encoded = URLEncode(url.hostname);
    std::vector<kodi::vfs::CDirEntry> items;
    ListArchive(ctx->ar, "archive://"+encoded+"/", items, false, "");
    archive_read_free(ctx->ar);
    delete ctx;

    for (kodi::vfs::CDirEntry& item : items)
    {
      if (item.Path() == url.url)
        return true;
    }

    return false;
  }

  bool DirectoryExists(const VFSURL& url) override
  {
    return false;
  }

  bool GetDirectory(const VFSURL& url,
                            std::vector<kodi::vfs::CDirEntry>& items,
                            CVFSCallbacks callbacks) override
  {
    ArchiveCtx* ctx = new ArchiveCtx;
    if (!ctx->Open(url.hostname))
    {
      delete ctx;
      return false;
    }

    ListArchive(ctx->ar, url.url, items, false, url.filename);
    archive_read_free(ctx->ar);
    delete ctx;

    return !items.empty();
  }

  bool ContainsFiles(const VFSURL& url,
                             std::vector<kodi::vfs::CDirEntry>& items,
                             std::string& rootpath) override
  {
    if (strstr(url.filename, ".rar"))
    {
      std::string fname(url.filename);
      size_t spos = fname.rfind('/');
      if (spos == std::string::npos)
        spos = fname.rfind('\\');
      fname.erase(0, spos);
      std::regex part_re("\\.part([0-9]+)\\.rar$");
      std::smatch match;
      if (std::regex_search(fname, match, part_re))
      {
        if (std::stoul(match[1].str()) != 1)
          return false;
      }
    }
    std::string encoded = URLEncode(url.url);
    rootpath = "archive://"+encoded + "/";
    ArchiveCtx* ctx = new ArchiveCtx;
    if (!ctx->Open(url.url))
    {
      delete ctx;
      return false;
    }

    ListArchive(ctx->ar, rootpath, items, true);
    archive_read_free(ctx->ar);
    delete ctx;

    return !items.empty();
  }
private:
  std::vector<std::string> splitString(const std::string& whole)
  {
    std::vector<std::string> result;
    std::istringstream f(whole);
    std::string s;
    while (getline(f, s, '/'))
      result.push_back(s);

    return result;
  }

  void ListArchive(struct archive* ar, const std::string& rootpath,
                   std::vector<kodi::vfs::CDirEntry>& items,
                   bool flat,
                   const std::string& subdir = "")
  {
    struct archive_entry* entry;
    std::set<std::string> folders;
    std::vector<std::string> rootSplit = splitString(subdir);

    int ret = ARCHIVE_OK;
    while (1)
    {
      ret = archive_read_next_header(ar, &entry);
      if (ret != ARCHIVE_OK)
        break;
      if (ret == ARCHIVE_RETRY)
        continue;

      std::string name = archive_entry_pathname_utf8(entry);
      std::vector<std::string> split = splitString(name);
      if (split.size() > rootSplit.size())
      {
        bool folder = false;
        bool match = true;
        for (size_t i = 0; i < rootSplit.size(); ++i)
        {
          if (rootSplit[i] != split[i])
          {
            match = false;
            break;
          }
        }

        if (flat ||
            (match && folders.find(split[rootSplit.size()]) == folders.end()))
        {
          kodi::vfs::CDirEntry kentry;
          std::string label = split[rootSplit.size()];
          std::string path = rootpath + split[rootSplit.size()];
          bool folder = false;
          if (split.size() > rootSplit.size()+1 || name.back() == '/')
          {
            path += '/';
            folder = true;
            folders.insert(split[rootSplit.size()]);
          }
          kentry.SetLabel(label);
          kentry.SetTitle(label);
          kentry.SetPath(path);
          kentry.SetFolder(folder);
          kentry.SetSize(archive_entry_size(entry));
          kentry.SetDateTime(archive_entry_mtime(entry));
          items.push_back(kentry);
        }
      }
      archive_read_data_skip(ar);
    }

    if (ret != ARCHIVE_OK && ret != ARCHIVE_EOF)
    {
      std::string errorString = archive_error_string(ar);
      if (ret == ARCHIVE_WARN)
      {
        kodi::Log(ADDON_LOG_WARNING, "ListArchive generated: '%s'", errorString.c_str());
        kodi::QueueFormattedNotification(QUEUE_WARNING, "%s", TranslateErrorString(errorString).c_str());
      }
      else if (ret == ARCHIVE_FAILED)
      {
        kodi::Log(ADDON_LOG_ERROR, "ListArchive generated: '%s'", errorString.c_str());
        kodi::QueueFormattedNotification(QUEUE_ERROR, "%s", TranslateErrorString(errorString).c_str());
      }
      else if (ret == ARCHIVE_FATAL)
      {
        kodi::Log(ADDON_LOG_FATAL, "ListArchive generated: '%s'", errorString.c_str());
        kodi::QueueFormattedNotification(QUEUE_ERROR, "%s", TranslateErrorString(errorString).c_str());
      }
    }
  }

  std::string TranslateErrorString(const std::string& errorString)
  {
    if (errorString == "RAR solid archive support unavailable.")
    {
      return kodi::GetLocalizedString(30000, errorString);
    }
    if (errorString == "Truncated RAR file data")
    {
      return kodi::GetLocalizedString(30001, errorString);
    }
    if (errorString == "Can't allocate rar data")
    {
      return kodi::GetLocalizedString(30002, errorString);
    }
    if (errorString == "Couldn't find out RAR header")
    {
      return kodi::GetLocalizedString(30003, errorString);
    }
    if (errorString == "Invalid marker header")
    {
      return kodi::GetLocalizedString(30004, errorString);
    }
    if (errorString == "Invalid header size" || errorString == "Invalid header size too small")
    {
      return kodi::GetLocalizedString(30005, errorString);
    }
    if (errorString == "RAR encryption support unavailable.")
    {
      return kodi::GetLocalizedString(30006, errorString);
    }
    if (errorString == "Header CRC error")
    {
      return kodi::GetLocalizedString(30007, errorString);
    }
    if (errorString == "Invalid sizes specified.")
    {
      return kodi::GetLocalizedString(30008, errorString);
    }
    if (errorString == "Bad RAR file")
    {
      return kodi::GetLocalizedString(30009, errorString);
    }
    if (errorString == "Unsupported compression method for RAR file.")
    {
      return kodi::GetLocalizedString(30010, errorString);
    }
    if (errorString == "Error during seek of RAR file")
    {
      return kodi::GetLocalizedString(30011, errorString);
    }
    if (errorString == "Invalid filename")
    {
      return kodi::GetLocalizedString(30012, errorString);
    }
    if (errorString == "Mismatch of file parts split across multi-volume archive")
    {
      return kodi::GetLocalizedString(30013, errorString);
    }
    if (errorString == "File CRC error")
    {
      return kodi::GetLocalizedString(30014, errorString);
    }
    if (errorString == "Parsing filters is unsupported.")
    {
      return kodi::GetLocalizedString(30015, errorString);
    }
    if (errorString == "Invalid symbol")
    {
      return kodi::GetLocalizedString(30016, errorString);
    }
    if (errorString == "Internal error extracting RAR file")
    {
      return kodi::GetLocalizedString(30017, errorString);
    }

    return errorString;
  }
};

class ATTRIBUTE_HIDDEN CMyAddon : public kodi::addon::CAddonBase
{
public:
  CMyAddon() = default;
  ADDON_STATUS CreateInstance(int instanceType, std::string instanceID, KODI_HANDLE instance, KODI_HANDLE& addonInstance) override
  {
    addonInstance = new CArchiveFile(instance);
    return ADDON_STATUS_OK;
  }
};

ADDONCREATOR(CMyAddon);
