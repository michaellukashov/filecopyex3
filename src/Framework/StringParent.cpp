#include "StdHdr.h"
#include "StringParent.h"
#include "FrameworkUtils.h"

int StringParent::loadFromFile(FILE * f)
{
  Clear();
  union
  {
    char c[2];
    wchar_t uc;
  } sign;
  int read = (int)fread(&sign, 1, sizeof(sign), f);
  int unicode = (read == 2) && (sign.uc == 0xFEFF || sign.uc == 0xFFFE);
  int inv = (unicode) && (sign.uc == 0xFFFE);
  const int bsize = 4096, ssize = 4096;
  if (unicode)
  {
    const wchar_t CR = '\r', LF = '\n';
    wchar_t buffer[bsize], string[ssize];
    buffer[0] = sign.uc;
    int bpos = 1, spos = 0;
    while (1)
    {
      wchar_t oldc = 0;
      read = (int)fread(buffer + bpos, sizeof(wchar_t), bsize - bpos, f);
      if (read < 1)
        break;
      for (int i = bpos; i < read; i++)
      {
        if (inv)
          buffer[i] = ((buffer[i] & 0x00FF) << 8) | ((buffer[i] & 0xFF00) >> 8);
        if (buffer[i] == CR || (buffer[i] == LF && oldc != CR))
        {
          string[spos] = 0;
          AddString(string);
          spos = 0;
        }
        else
        {
          if (buffer[i] != CR && buffer[i] != LF && spos < ssize)
          {
            string[spos++] = buffer[i];
          }
        }
        oldc = buffer[i];
      }
      bpos = 0;
    }
    if (spos)
    {
      string[spos] = 0;
      AddString(string);
    }
  }
  else
  {
    const char CR = '\r', LF = '\n';
    char buffer[bsize + 1], string[ssize + 1];
    int bpos = 0, spos = 0;
    if (read >= 1)
      buffer[0] = sign.c[0];
    if (read >= 2)
      buffer[1] = sign.c[1];
    bpos = read;
    while (1)
    {
      char oldc = 0;
      read = (int)fread(buffer + bpos, sizeof(char), bsize - bpos, f);
      if (read < 1)
        break;
      for (int i = 0; i < read + bpos; i++)
      {
        if (buffer[i] == CR || (buffer[i] == LF && oldc != CR))
        {
          string[spos] = 0;
          AddString(string);
          spos = 0;
        }
        else
        {
          if (buffer[i] != CR && buffer[i] != LF && spos < ssize)
          {
            string[spos++] = buffer[i];
          }
        }
        oldc = buffer[i];
      }
      bpos = 0;
    }
    if (spos)
    {
      string[spos] = 0;
      AddString(string);
    }
  }
  return 1;
}


int StringParent::loadFromFile(const String & fn)
{
  FILE * f = nullptr;
  _wfopen_s(&f, fn.ptr(), L"rb");
  if (!f)
  {
    return 0;
  }
  int res = loadFromFile(f);
  fclose(f);
  return res;
}


bool StringParent::saveToFile(FILE * f, TextFormat tf)
{
  if (tf == tfUnicode)
  {
    uint16_t sign = 0xFEFF;
    fwrite(&sign, sizeof(sign), 1, f);
  }
  else if (tf == tfUnicodeBE)
  {
    uint16_t sign = 0xFFFE;
    fwrite(&sign, sizeof(sign), 1, f);
  }
  for (size_t i = 0; i < Count(); i++)
  {
    const wchar_t * s = (*this)[i].c_str();
    const int ssize = 4096;
    if (tf != tfANSI)
    {
      wchar_t buf[ssize];
      wcscpy_s(buf, ssize, s);
      if (tf == tfUnicodeBE)
      {
        for (size_t j = 0; j < wcslen(buf); j++)
        {
          buf[j] = ((buf[j] & 0x00FF) << 8) | ((buf[j] & 0xFF00) >> 8);
        }
      }
      fwrite(buf, sizeof(wchar_t), wcslen(buf), f);
      wcscpy_s(buf, ssize, L"\r\n");
      if (tf == tfUnicodeBE)
      {
        for (size_t j = 0; j < wcslen(buf); j++)
        {
          buf[j] = ((buf[j] & 0x00FF) << 8) | ((buf[j] & 0xFF00) >> 8);
        }
      }
      fwrite(buf, sizeof(wchar_t), 2, f);
    }
    else
    {
      char buf[ssize];
      _wtoacs(buf, ssize, s);
      fwrite(buf, sizeof(char), strlen(buf), f);
      fwrite("\r\n", sizeof(char), 2, f);
    }
  }
  return true;
}

bool StringParent::saveToFile(const String & fn, TextFormat tf)
{
  DWORD attr = ::GetFileAttributes(fn.ptr());
  ::SetFileAttributes(fn.ptr(), FILE_ATTRIBUTE_NORMAL);
  FILE * f = nullptr;
  _wfopen_s(&f, fn.ptr(), L"wb");
  bool res = false;
  if (f)
  {
    res = saveToFile(f, tf);
    fclose(f);
  }
  if (attr != INVALID_FILE_ATTRIBUTES)
  {
    ::SetFileAttributes(fn.ptr(), attr);
  }
  return res;
}

void StringParent::loadFromString(const wchar_t * s, wchar_t delim)
{
  Clear();
  wchar_t * p = (wchar_t *)s;
  wchar_t * pp = p;
  wchar_t buf[4096];
  do
  {
    if (!*p || *p == delim)
    {
      size_t len = __min((size_t)(p - pp), LENOF(buf) - 1);
      wcsncpy_s(buf, LENOF(buf), pp, len);
      buf[len] = 0;
      pp = p + 1;
      AddString(buf);
    }
  }
  while (*p++);
}

void StringParent::loadFromString(const String & s, wchar_t delim)
{
  loadFromString(s.c_str(), delim);
}
