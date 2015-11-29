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

#define MPEGTS_FILENAME "nobroadcasting.ts"
#define BUFFER_SIZE     131072

using namespace ADDON;

NoBroadcasting::NoBroadcasting()
: m_valid(false)
, m_file(0)
, m_buf(0)
, m_beg(0)
, m_end(0)
, m_len(0)
, m_flen(0)
, m_pos(0)
{
  std::string filePath;
  filePath = g_szClientPath + PATH_SEPARATOR_STRING + "resources" + PATH_SEPARATOR_STRING + MPEGTS_FILENAME;
  if ((m_valid = _init(filePath.c_str())))
  {
    m_buf = new char[BUFFER_SIZE];
    m_beg = m_buf;
    m_end = m_buf + BUFFER_SIZE;
  }
}

NoBroadcasting::~NoBroadcasting()
{
  SAFE_DELETE(m_buf);
  if (m_file)
    XBMC->CloseFile(m_file);
}

int NoBroadcasting::Read(void* buffer, unsigned n)
{
  if (!m_valid)
    return -1;

  char* b = (char*)buffer;
  bool eof = false;
  n = (n > BUFFER_SIZE ? BUFFER_SIZE : n);
  unsigned r = n;
  do
  {
    if (m_len >= r)
    {
      memcpy(b, m_beg, r);
      m_beg += r;
      m_len -= r;
      m_pos += r;
      return (int)n;
    }
    memcpy(b, m_beg, m_len);
    r -= m_len;
    m_pos += m_len;
    b += m_len;
    m_beg = m_buf;
    m_len = 0;
    do
    {
      size_t s = XBMC->ReadFile(m_file, m_buf, BUFFER_SIZE);
      if (s > 0)
      {
        m_len += s;
        eof = false;
        break;
      }
      if (eof)
        break;
      else
        eof = true;
      XBMC->SeekFile(m_file, 0, 0);
    } while (eof);
  } while (r > 0 && !eof);
  XBMC->Log(LOG_DEBUG, "%s: EOF", __FUNCTION__);
  return (int)(n - r);
}

int64_t NoBroadcasting::Seek(int64_t offset, Myth::WHENCE_t whence)
{
  switch (whence)
  {
    case Myth::WHENCE_SET:
    m_pos = (offset > GetSize() ? GetSize() : (offset > 0 ? offset : m_pos));
    break;
  case Myth::WHENCE_CUR:
    m_pos += (m_pos + offset > GetSize() ? GetSize() : (m_pos + offset < 0 ? 0 : m_pos + offset));
    break;
  case Myth::WHENCE_END:
    m_pos = (offset < 0 ? GetSize() : (GetSize() - offset > 0 ? GetSize() - offset : 0));
    break;
  default:
    return -1;
  }
  m_pos = XBMC->SeekFile(m_file, m_pos % m_flen, 0);
  return m_pos;
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

