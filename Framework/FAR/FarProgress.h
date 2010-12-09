#pragma once
#include "farplugin.h"

#define WIN_NONE          0
#define WIN_MESSAGE       1
#define WIN_PROGRESS      2
#define WIN_SCAN_PROGRESS 3

class FarProgress
{
public:
  FarProgress(void);
  virtual ~FarProgress(void);
  void ShowMessage(const String&);
  void ShowProgress(const String&);
  void ShowScanProgress(const String& msg);
  void SetScanProgressInfo(__int64 NumberOfFiles, __int64 TotalSize);
  void Hide();
  void SetPercent(float);
  int InverseBars;
protected:
  int clrFrame, clrTitle, clrBar, clrText, clrLabel;
  int ProgX1, ProgX2, ProgY, WinType;
  HANDLE hScreen;
  void DrawWindow(int, int, int, int, const String&);
  void GetConSize(int&, int&);
  void DrawProgress(int, int, int, float);
  void DrawText(int, int, int, const String&);
  void SetTitle(const String&);
  void SetTitle2(const String&);
  String GetTitle();
  String TitleBuf, ProgTitle;
  void DrawScanProgress(int x1, int x2, int y, __int64 NumberOfFiles, __int64 TotalSize);
  __int64 LastUpdate;
};
