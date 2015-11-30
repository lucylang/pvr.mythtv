/*
 *      Copyright (C) 2005-2015 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "nobroadcasting.h"
#include "client.h"
#include <platform/os.h>
#include <inttypes.h>

#define MPEGTS_FILENAME "nobroadcasting.ts"
#define FILE_READ_SIZE  131072

using namespace ADDON;

NoBroadcasting::NoBroadcasting()
: m_valid(false)
, m_file(0)
, m_flen(0)
, m_pos(0)
{
  std::string filePath;
  filePath = g_szClientPath + PATH_SEPARATOR_STRING + "resources" + PATH_SEPARATOR_STRING + MPEGTS_FILENAME;
  m_valid = _init(filePath.c_str());
}

NoBroadcasting::~NoBroadcasting()
{
  if (m_file)
    XBMC->CloseFile(m_file);
}

int NoBroadcasting::Read(void* buffer, unsigned n)
{
  if (!m_valid)
    return -1;

  char* b = (char*)buffer;
  bool eof = false;
  n = (n > FILE_READ_SIZE ? FILE_READ_SIZE : n);
  unsigned r = n;
  do
  {
    size_t s = XBMC->ReadFile(m_file, b, r);
    if (s > 0)
    {
      r -= s;
      b += s;
      m_pos += s;
      eof = false;
    }
    else
    {
      if (eof)
        break;
      eof = true;
      XBMC->SeekFile(m_file, 0, 0);
    }
  } while (r > 0 || eof);
  if (eof)
    XBMC->Log(LOG_DEBUG, "%s: EOF", __FUNCTION__);
  if (g_bExtraDebug)
    XBMC->Log(LOG_DEBUG, "%s: flen=%" PRId64 " pos=%" PRId64, __FUNCTION__, m_flen, m_pos);
  return (int)(n -r);
}

int64_t NoBroadcasting::Seek(int64_t offset, Myth::WHENCE_t whence)
{
  switch (whence)
  {
  case Myth::WHENCE_SET:
    if (offset <= GetSize() && offset >= 0)
      return (m_pos = XBMC->SeekFile(m_file, offset, 0));
    break;
  case Myth::WHENCE_CUR:
    if ((m_pos + offset) <= GetSize() && m_pos + offset >= 0)
      return (m_pos = XBMC->SeekFile(m_file, m_pos + offset, 0));
    break;
  case Myth::WHENCE_END:
    if (offset >= 0 && (GetSize() - offset) >= 0)
      return (m_pos = XBMC->SeekFile(m_file, GetSize() - offset, 0));
    break;
  default:
    break;
  }
  return -1;
}

bool NoBroadcasting::_init(const char* filePath)
{
  m_file = XBMC->OpenFile(filePath, 0);
  if (m_file)
  {
    m_flen = XBMC->GetFileLength(m_file);
    return true;
  }
  return false;
}

