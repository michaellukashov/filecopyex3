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

#include "common.h"
#include "ui.h"

int ShowMessage(const String& title, const String& msg, int Flags)
{
  return ShowMessageHelp(title, msg, Flags, "");
}

int ShowMessageHelp(const String& title, const String& msg, int Flags, const String& help)
{
  String msgbuf=title+"\n"+msg+"\n\x01";
  int res=0; /* XXX Info.Message(Info.ModuleNumber, 
                        Flags | FMSG_ALLINONE, 
                        help.ptr(), 
                        (const wchar_t**)(const wchar_t*)msgbuf.ptr(), 0, 0); */
  return res;
}

int ShowMessageEx(const String& title, const String& msg, 
                  const String& buttons, int flags)
{
  return ShowMessageExHelp(title, msg, buttons, flags, "");
}

int ShowMessageExHelp(const String& title, const String& msg, 
                  const String& buttons, int flags, const String& help)
{
  int nb=0;
  for (const wchar_t *p=buttons.ptr(); *p; p++)
    if (*p=='\n') nb++;
  String msgbuf=title+"\n"+msg+"\n\x01\n"+buttons;
  int res=0; /* XXX Info.Message(Info.ModuleNumber, flags | FMSG_ALLINONE, 
    help.ptr(), (const wchar_t**)(const wchar_t*)msgbuf.ptr(), 0, nb+1); */
  return res;
}

int msgw()
{
  return 50;
}

void Error(const String& s, int code)
{
  ShowMessageEx(LOC("Framework.Error"), 
    s+"\n"+SplitWidth(GetErrText(code), msgw()), 
    LOC("Framework.OK"), 
    FMSG_WARNING);
}

void Error2(const String& s, const String& fn, int code)
{
  ShowMessageEx(LOC("Framework.Error"), 
    s+"\n"+FormatWidthNoExt(fn, msgw())+"\n"+SplitWidth(GetErrText(code), msgw()), 
    LOC("Framework.OK"), 
    FMSG_WARNING);
}

int Error2RS(const String& s, const String& fn, int code)
{
  int res=ShowMessageEx(LOC("Framework.Error"), 
    s+"\n"+FormatWidthNoExt(fn, msgw())+"\n"+SplitWidth(GetErrText(code), msgw()), 
    LOC("Framework.Retry")+"\n"+LOC("Framework.Skip"), 
    FMSG_WARNING);
  if (res==0) return RES_RETRY;
  else return RES_SKIP;
}

String GetErrText(int code)
{
  wchar_t buf[1024];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, 0, buf, 1024, NULL);
  return buf;
}

String FormatWidth(const String &s, int len)
{
	int dif = s.len()-len;
	if (dif > 0) {
		return String("...") + s.right(len-3);
	} else {
		return s + String(' ', -dif);
	}
}

String FormatWidthNoExt(const String& s, int len)
{
	int dif = s.len()-len;
	if (dif > 0) {
		return String("...") + s.right(len-3);
	} else {
		return s;
	}
}

static void stripnl(wchar_t *msg)
{
  wchar_t *pa=msg, *pm=msg;
  while (*pa)
  {
    if (*pa!='\n' && *pa!='\r') *pm++=*pa;
    pa++;
  }
  *pm=0;
}

String SplitWidth(const String &s, int w)
{
  wchar_t msg[1024], res[1024];
  s.CopyTo(msg, 1024);
  stripnl(msg);
  wchar_t *p=(wchar_t*)msg, *op=p;
  res[0]=0;
  do
  {
    while (*p && *p!=' ') p++;
    if (p-op>w)
    {
      wchar_t t=*p;
      *p=0;
      wcscat_s(res, 1024, op);
      wcscat_s(res, 1024, L"\n");
      op=p+1;
      *p=t;
    }
  }  while (*p++);
  if (*op) wcscat_s(res, 1024, op);
  p=(wchar_t*)_tcsend(res)-1;
  while (p>=res && *p=='.') *p--=0;
  return res;
}