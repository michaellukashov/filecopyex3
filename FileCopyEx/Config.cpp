#include <stdhdr.h>
#include "lowlevelstr.h"
#include "filecopyex.h"
#include "far/interface/farkeys.hpp"

void reloadmacro();
void getcolors();
void fixbind();

void FileCopyExPlugin::MiscInit()
{
  DescFiles.LoadFromString(Registry.GetString("\\Software\\Far2\\Descriptions", "ListNames", 
                            "Descript.ion,Files.bbs"), ',');
//  DescFiles.SetOptions(slSorted | slIgnoreCase);
}

String regkey = "\\Software\\Far2\\KeyMacros\\Shell";

int isour(const String &key)
{
  String src=regkey+"\\"+key;
  String buf=Registry.GetString(src, "Sequence", "");
  return (!buf.nicmp("F11 >", 5) || !buf.icmp("F5") || !buf.icmp("F6"));
}

void restore(const String &key)
{
  if (isour(key))
  {
    String src=regkey+".backup\\"+key;
    String dst=regkey+"\\"+key;
    Registry.DeleteKey(dst);
    Registry.CopyKey(src, dst);
    Registry.DeleteKey(src);
  }
}

void dobind(const String& key, const String& seq)
{
  String src=regkey+"\\"+key;
  String dst=regkey+".backup\\"+key;
  if (!isour(key))
  {
    Registry.DeleteKey(dst);
    Registry.CopyKey(src, dst);
    Registry.DeleteKey(src);
  }
  Registry.SetString(src, "Sequence", seq);  
  Registry.SetInt(src, "DisableOutput", 1);  
  Registry.SetInt(src, "NoPluginPanels", 0);  
  Registry.SetInt(src, "PluginPanels", 1);  
}

int getmod()
{
  if (isour("AltShiftF5")) return KEY_ALT | KEY_SHIFT;
  else if (isour("CtrlShiftF5")) return KEY_CTRL | KEY_SHIFT;
  else if (isour("CtrlAltF5")) return KEY_CTRL | KEY_ALT;
  else return 0;
}

void reloadmacro()
{
  ActlKeyMacro prm;
  memset(&prm, 0, sizeof(prm));
  prm.Command=MCMD_LOADALL;
  Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &prm);
}

void FileCopyExPlugin::KeyConfig()
{
  FarDialog &dlg=Dialogs["KeysDialog"];
  dlg.ResetControls();

  int altShift, ctrlShift, ctrlAlt, bind;
  bind = isour("F5") && isour("F6") 
    && isour("ShiftF5") && isour("ShiftF6");
  altShift = isour("AltShiftF5") && isour("AltShiftF6");
  ctrlShift = isour("CtrlShiftF5") && isour("CtrlShiftF6");
  ctrlAlt = isour("CtrlAltF5") && isour("CtrlAltF6");
  int bindAddToQueue = isour("CtrlShiftQ");
  int bindShowQueue = isour("AltShiftQ");

  dlg["BindToF5"]("Selected")=bind;
  dlg["AltShiftF5"]("Selected")=!bind || altShift;
  dlg["CtrlShiftF5"]("Selected")=bind && ctrlShift;
  dlg["CtrlAltF5"]("Selected")=bind && ctrlAlt;
  dlg["BindAddToQueue"]("Selected")=bindAddToQueue;
  dlg["BindShowQueue"]("Selected")=bindShowQueue;

  if (dlg.Execute()==-1) return;

  if (dlg["BindToF5"]("Selected") != bind
    || dlg["AltShiftF5"]("Selected") != altShift
    || dlg["CtrlShiftF5"]("Selected") != ctrlShift
    || dlg["CtrlAltF5"]("Selected") != ctrlAlt
    || dlg["BindAddToQueue"]("Selected") != bindAddToQueue
    || dlg["BindShowQueue"]("Selected") != bindShowQueue)
  {
    ActlKeyMacro prm;
    memset(&prm, 0, sizeof(prm));
    prm.Command=MCMD_SAVEALL;
    Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &prm);

    restore("F5"); restore("F6");
    restore("ShiftF5"); restore("ShiftF6");
    restore("AltShiftF5"); restore("AltShiftF6");
    restore("CtrlShiftF5"); restore("CtrlShiftF6");
    restore("CtrlAltF5"); restore("CtrlAltF6");
    restore("CtrlShiftQ"); restore("AltShiftQ"); 

    if (dlg["BindToF5"]("Selected"))
    {
      dobind("F5", "F11 > 1"); 
      dobind("F6", "F11 > 2");
      dobind("ShiftF5", "F11 > 3"); 
      dobind("ShiftF6", "F11 > 4");
      String key;
      if (dlg["AltShiftF5"]("Selected")) key="AltShift";
      else if (dlg["CtrlShiftF5"]("Selected")) key="CtrlShift";
      else if (dlg["CtrlAltF5"]("Selected")) key="CtrlAlt";
      dobind(key+"F5", "F5");
      dobind(key+"F6", "F6");
    }

    if (dlg["BindAddToQueue"]("Selected")) dobind("CtrlShiftQ", "F11 > 5");
    if (dlg["BindShowQueue"]("Selected")) dobind("AltShiftQ", "F11 > 6");

    reloadmacro();
  }
}


#ifdef _WIN64
#define VersionStr "version 2.0.0 beta (x64 Unicode), " __DATE__
#else
#define VersionStr "version 2.0.0 beta (x86 Unicode), " __DATE__
#endif

void FileCopyExPlugin::About()
{
  FarDialog &dlg=Dialogs["AboutDialog"];
  dlg.ResetControls();
  dlg["Label2"]("Text") = VersionStr;
  dlg.Execute();
}


int bn;
void beep();
void beep2();
void beep3();

void FileCopyExPlugin::Config()
{
  FarDialog &dlg=Dialogs["SetupDialog"];
  dlg.ResetControls();
  dlg.LoadState(Options);

rep:
  int res=dlg.Execute();
  if (res==1) 
  {
    KeyConfig();
    goto rep;
  }
  else if (res==2)
  {
    About();
    goto rep;
  }
  else if (res==3)
  {
    if (bn==0) beep();
    else if (bn==1) beep2();
    else if (bn==2) beep3();
    bn++;
    if (bn>2) bn=0;
    goto rep;
  }
  else if (res==0)
  {
    dlg.SaveState(Options);
  }
}

int clrFrame=0x7F, clrTitle=0x70, clrBar=0x70, 
  clrText=0x70, clrLabel=0x71;

void dobeep(int hz, int delay)
{
  if(WinNT)
	  Beep(hz, delay);
}

#pragma warning(disable:4305)
#pragma warning(disable:4244)
      
float tone=1.122462048309, htone=1.059463094359;
float A=440, H=A*tone, C=H*htone, D=C*tone, E=D*tone,
      F=E*htone, G=F*tone;
float A1=A*2, H1=H*2, C1=C*2, D1=D*2, E1=E*2, F1=F*2, G1=G*2;
float A2=A*4, H2=H*4, C2=C*4, D2=D*4, E2=E*4, F2=F*4, G2=G*4;
int dur=75;

void playAm()
{
  dobeep(A, dur); dobeep(A1, dur); dobeep(C1, dur);
  dobeep(E1, dur); dobeep(C1, dur); dobeep(A1, dur);
}

void playDm()
{
  dobeep(D, dur); dobeep(D1, dur); dobeep(F1, dur);
  dobeep(A1, dur); dobeep(F1, dur); dobeep(D1, dur);
}

void playC()
{
  dobeep(C, dur); dobeep(C1, dur); dobeep(E1, dur);
  dobeep(G1, dur); dobeep(E1, dur); dobeep(C1, dur);
}

void playG()
{
  dobeep(G, dur); dobeep(G1, dur); dobeep(H1, dur);
  dobeep(D1, dur); dobeep(H1, dur); dobeep(G1, dur);
}

void beep()
{
  playAm(); 
}

void beep2()
{
  playC(); playG();
}

void beep3()
{
  playDm(); playC(); playG();
}