#include <stdhdr.h>
#include "lowlevelstr.h"
#include "far/farplugin.h"

FarMenu::FarMenu(void)
{
  Flags=Selection=0;
}

FarMenu::~FarMenu(void)
{
	for(int i = 0; i < items.Count(); ++i)
	{
		free((void*)items[i].Text);
	}
}

void FarMenu::SetItemText(FarMenuItem* item, const String& text)
{
	size_t len = text.len() + 1;
	wchar_t* t = (wchar_t*)malloc(sizeof(wchar_t) * len);
	text.ToUnicode(t, len);
	item->Text = t;
}

void FarMenu::AddLine(const String& line)
{
  FarMenuItem item;
  item.Checked=0;
  item.Selected=Selection==items.Count();
  item.Separator=0;
  SetItemText(&item, line);
  items.Add(item);
}

void FarMenu::AddLineCheck(const String& line, int check)
{
  FarMenuItem item;
  item.Checked=check;
  item.Selected=Selection==items.Count();
  item.Separator=0;
  SetItemText(&item, line);
  items.Add(item);
}

void FarMenu::AddSep()
{
  FarMenuItem item;
  item.Checked=0;
  item.Separator=1;
  item.Selected=0;
  SetItemText(&item, String());
  items.Add(item);
}

int FarMenu::Execute()
{
  return Info.Menu(Info.ModuleNumber, -1, -1, 0, Flags,
    Title.ptr(), Bottom.ptr(), HelpTopic.ptr(), NULL, NULL, items.Storage(),
    items.Count());
}

void FarMenu::SetBottom(const String& v)
{
  Bottom=v;
}

void FarMenu::SetFlags(int f)
{
  Flags=f;
}

void FarMenu::SetHelpTopic(const String& v)
{
  HelpTopic=v;
}

void FarMenu::SetSelection(int n)
{
  Selection=n;
}

void FarMenu::SetTitle(const String& v)
{
  Title=v;
}

int ShowMessage(const String& title, const String& msg, int Flags)
{
  return ShowMessageHelp(title, msg, Flags, "");
}

int ShowMessageHelp(const String& title, const String& msg, int Flags, const String& help)
{
  String msgbuf=title+"\n"+msg+"\n\x01";
  int res=Info.Message(Info.ModuleNumber, 
                        Flags | FMSG_ALLINONE, 
                        help.ptr(), 
                        (const wchar_t**)(const wchar_t*)msgbuf.ptr(), 0, 0);
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
  for (wchar_t *p=buttons.Lock(); *p; p++)
    if (*p=='\n') nb++;
  buttons.Unlock();
  String msgbuf=title+"\n"+msg+"\n\x01\n"+buttons;
  int res=Info.Message(Info.ModuleNumber, flags | FMSG_ALLINONE, 
    help.ptr(), (const wchar_t**)(const wchar_t*)msgbuf.ptr(), 0, nb+1);
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

String FormatWidth(const String& s, int len)
{
  int dif = s.len()-len;
  if (dif>0) 
    return String("...")+s.right(len-3);
  else
    return s+String(' ', -dif);
}

String FormatWidthNoExt(const String& s, int len)
{
  int dif = s.len()-len;
  if (dif>0) 
    return String("...")+s.right(len-3);
  else
    return s;
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

String SplitWidth(const String& s, int w)
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
      _tcat(res, op, 1024);
      _tcat(res, _T("\n"), 1024);
      op=p+1;
      *p=t;
    }
  }
  while (*p++);
  if (*op) _tcat(res, op, 1024);
  p=(wchar_t*)_tcsend(res)-1;
  while (p>=res && *p=='.') *p--=0;
  return res;
}