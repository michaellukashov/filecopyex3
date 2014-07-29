#include "FileNameStoreEnum.h"
#include "../Common.h"

String FileNameStoreEnum::GetNext()
{
  if (cur >= store->Count())
  {
    return L"";
  }

  const FileName & fn = (*store)[cur++];

  switch (fn.getDirection())
  {
    case FileName::levelPlus:
    {
      if (!curPath.empty())
      {
        curPath += L"\\";
      };
      curPath += fn.getName();
      buffer = curPath;
      break;
    }

    case FileName::levelMinus:
    {
      buffer = curPath;
      curPath = curPath.substr(0, curPath.rfind('\\'));

      break;
    }

    case FileName::levelStar:
    {
      buffer = curPath;
      curPath = L"";
      break;
    }

    case FileName::levelSame:
    {
      buffer = curPath;
      if (!buffer.empty())
      {
        buffer += L"\\";
      }
      buffer += fn.getName();
    }
  }

  return buffer;
}

void FileNameStoreEnum::Skip()
{
  if (cur >= store->Count())
  {
    return;
  }

  const FileName & fn = (*store)[cur++];

  switch (fn.getDirection())
  {
    case FileName::levelPlus:
    {
      if (!curPath.empty())
      {
        curPath += L"\\";
      };
      curPath += fn.getName();
      break;
    }

    case FileName::levelMinus:
    {
      curPath = curPath.substr(0, curPath.rfind('\\'));
      break;
    }

    case FileName::levelStar:
    {
      curPath = L"";
      break;
    }
  }
}

void FileNameStoreEnum::ToFirst()
{
  cur = 0;
  curPath = L"";
  buffer = L"";
}

String FileNameStoreEnum::GetByNum(size_t n)
{
  if (cur > n + 1)
  {
    FWError(L"FileNameStoreEnum::GetByNum - assertion failure");
    return L"";
  }

  if (cur == n + 1)
  {
    return buffer;
  }
  else
  {
    while (cur < n) Skip();
    return GetNext();
  }
}
