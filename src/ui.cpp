/*
FileCopyEx - Extended File Copy plugin for Far 2 file manager

Copyright (C) 2004 - 2010
Idea & core: Max Antipin
Coding: Serge Cheperis aka craZZy
Bugfixes: slst, CDK, Ivanych, Alter, Axxie and Nsky
Special thanks to Vitaliy Tsubin
Far 2 (32 & 64 bit) full unicode version by djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "guid.hpp"
#include "ui.h"

intptr_t ShowMessage(const String & title, const String & msg, int Flags)
{
  return ShowMessageHelp(title, msg, Flags, L"");
}

intptr_t ShowMessageOK(const String & title, const String & msg)
{
  return ShowMessage(title, msg, FMSG_MB_OK);
}

intptr_t ShowMessageHelp(const String & title, const String & msg, int Flags, const String & help)
{
  String msgbuf = title + L"\n" + msg + L"\n\x01";
  intptr_t res = Info.Message(&MainGuid, &UnkGuid,
                              Flags | FMSG_ALLINONE,
                              help.ptr(),
                              (const wchar_t **)(const wchar_t *)msgbuf.ptr(), 0, 0
                             );
  return res;
}

intptr_t ShowMessageEx(const String & title, const String & msg,
                       const String & buttons, int flags)
{
  return ShowMessageExHelp(title, msg, buttons, flags, L"");
}

intptr_t ShowMessageExHelp(const String & title, const String & msg,
                           const String & buttons, int flags, const String & help)
{
  size_t nb = 0;
  for (const wchar_t * p = buttons.ptr(); *p; p++)
  {
    if (*p == '\n')
      nb++;
  }
  String msgbuf = title + L"\n" + msg + L"\n\x01\n" + buttons;
  intptr_t res = Info.Message(&MainGuid, &UnkGuid, flags | FMSG_ALLINONE,
                              help.ptr(), (const wchar_t **)(const wchar_t *)msgbuf.ptr(), 0, nb + 1
                             );
  return res;
}

int msgw()
{
  return 50;
}

void Error(const String & s, int code)
{
  ShowMessageEx(LOC(L"Framework.Error"),
                s + L"\n" + SplitWidth(GetErrText(code), msgw()),
                LOC(L"Framework.OK"),
                FMSG_WARNING
               );
}

void Error2(const String & s, const String & fn, int code)
{
  ShowMessageEx(LOC(L"Framework.Error"),
                s + L"\n" + FormatWidthNoExt(fn, msgw()) + L"\n" + SplitWidth(GetErrText(code), msgw()),
                LOC(L"Framework.OK"),
                FMSG_WARNING
               );
}

intptr_t Error2RS(const String & s, const String & fn, int code)
{
  intptr_t res = ShowMessageEx(LOC(L"Framework.Error"),
                               s + L"\n" + FormatWidthNoExt(fn, msgw()) + L"\n" + SplitWidth(GetErrText(code), msgw()),
                               LOC(L"Framework.Retry") + L"\n" + LOC(L"Framework.Skip"),
                               FMSG_WARNING
                              );
  if (res == 0)
  {
    return RES_RETRY;
  }
  return RES_SKIP;
}

String GetErrText(int code)
{
  wchar_t buf[1024];
  FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, code, 0, buf, LENOF(buf), nullptr);
  return buf;
}

String FormatWidth(const String & s, intptr_t len)
{
  intptr_t dif = (intptr_t)s.len() - len;
  if (dif > 0)
  {
    return String(L"...") + s.right(len - 3);
  }
  else
  {
    return s + String(' ', -dif);
  }
}

String FormatWidthNoExt(const String & s, intptr_t len)
{
  intptr_t dif = (intptr_t)s.len() - len;
  if (dif > 0)
  {
    return String(L"...") + s.right(len - 3);
  }
  else
  {
    return s;
  }
}

String SplitWidth(const String & s, intptr_t w)
{
  String res;
  res.reserve(s.len());
  intptr_t curW = 0;
  for (size_t i = 0; i < s.len(); i++)
  {
    wchar_t c = s[i];

    if (c == '\n' || c == '\r')
    {
      continue;
    }
    if (c == ' ' && (curW > w))
    {
      res += '\n';
      curW = 0;
      continue;
    }
    res += c;
    curW++;
  }
  return res;
}
