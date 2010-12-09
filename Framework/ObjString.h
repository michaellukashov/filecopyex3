#pragma once
#include "lowlevelstr.h"
#include <string>

class String;

class String
{
public:
	String() {}
	String(const char* v)
	{
		str.resize(strlen(v), '\0');
		_atotcs(ptr(), v, str.length()+1);
	}
	String(wchar_t ch, int len)
	{
		str.resize(len, ch);
	}
	String(const wchar_t* v)
	{
		str = v;
	}
	String(const std::wstring& v)
	{
		str = v;
	}
	inline bool operator==(const String& v) const { return cmp(v)==0; }
	inline bool operator!=(const String& v) const { return cmp(v)!=0; }
	inline bool operator<(const String& v) const { return cmp(v)<0; }
	inline bool operator>(const String& v) const { return cmp(v)>0; }
	inline bool operator<=(const String& v) const { return cmp(v)<=0; }
	inline bool operator>=(const String& v) const { return cmp(v)>=0; }
	inline void operator+=(const String& v) { str += v.str; }
	const String operator+(const String& v) const { return str + v.str; }

	inline int len() const { return (int)str.length(); }

	inline wchar_t operator[] (int i) const
	{
		if (i>=0 && i<(int)str.length()) return str[i];
		else return 0;
	}

	int AsInt() const { return _ttoi(ptr()); }
	double AsFloat() const { return _tstof(ptr()); }
	bool AsBool() const { return operator==(_T("1")); } 
	void ToUnicode(wchar_t* buf, size_t sz) const { CopyTo(buf, sz); }
	void CopyTo(wchar_t* buf, size_t sz) const { _tcopy(buf, ptr(), sz); }

	explicit String(int v)
	{
		wchar_t buf[64];
		_itot_s(v, buf, 64, 10);
		str = buf;
	}
	explicit String(__int64 v)
	{
		wchar_t buf[64];
		_i64tot_s(v, buf, 64, 10);
		str = buf;
	}
	explicit String(double v)
	{
		wchar_t buf[64];
		_stprintf_s(buf, 64, _T("%g"), v);
		str = buf;
	}
	explicit String(bool v)
	{
		if (v) str = _T("1");
		else str = _T("0");
	}

	int cmp(const String& v) const { return ncmp(v, 0x7FFFFFFF); }
	int icmp(const String& v) const { return nicmp(v, 0x7FFFFFFF); }
	int ncmp(const String& v, int sz) const { return _tcsncmp(ptr(), v.ptr(), sz); }
	int nicmp(const String& v, int sz) const { return _tcsnicmp(ptr(), v.ptr(), sz); }
	// bug #46 fixed by axxie
	const bool isbadchar(wchar_t c) const { return c >= _T('\0') && c <= _T(' '); }

	const String substr(int s, int l = 0x7FFFFFFF) const;
	const String left(int n) const { return substr(0, n); }
	const String right(int n) const { return substr(len()-n, n); }

	const String trim() const;
	const String ltrim() const;
	const String rtrim() const;
	const String trimquotes() const;

	const String rev() const;

	const String replace(const String& what, const String& with, int nocase=0) const;
	const String toUpper() const;
	const String toLower() const;

	int find(const String& v, int start=0) const
	{
		wchar_t* p=ptr(), *rp=_tcsstr(p+start, v.ptr());
		if (!rp) return -1; 
		else return (int)(rp-p);
	}
	int rfind(const String& v) const
	{
		wchar_t* p=ptr(), *rp=_tcsrstr(p, v.ptr());
		if (!rp) return -1; 
		else return (int)(rp-p);
	}
	int cfind(wchar_t v, int start=0) const
	{
		wchar_t* p=ptr(), *rp=_tcschr(p+start, v);
		if (!rp) return -1; 
		else return (int)(rp-p);
	}
	int crfind(wchar_t v) const
	{
		wchar_t* p=ptr(), *rp=_tcsrchr(p, v);
		if (!rp) return -1; 
		else return (int)(rp-p);
	}
	int cfind(const String& v, int start=0) const
	{
		wchar_t* p=ptr(), *rp=_tcspbrk(p+start, v.ptr());
		if (!rp) return -1; 
		else return (int)(rp-p);
	}
	int cnfind(const String& v, int start=0) const
	{
		wchar_t* p=ptr(), *rp=_tcsspnp(p+start, v.ptr());
		if (!rp) return -1; 
		else return (int)(rp-p);
	}
	int crfind(const String& v) const 
	{ 
		wchar_t* p=ptr(), *rp=_tcsrpbrk(p, v.ptr());
		if (!rp) return -1; 
		else return (int)(rp-p);
	}
	int cnrfind(const String& v) const 
	{ 
		wchar_t* p=ptr(), *rp=_tcsrspnp(p, v.ptr());
		if (!rp) return -1; 
		else return (int)(rp-p);
	}

	wchar_t* ptr() const { return Lock(); }
	wchar_t* Lock() const { return (wchar_t*)str.c_str(); }
	void Unlock() const {}

private:
	std::wstring str;
};
