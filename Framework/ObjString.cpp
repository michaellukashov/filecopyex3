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

#include "stdhdr.h"
#include "objstring.h"

const String String::substr(int s, int l) const
{
	if(s>=len() || s<0 || l<=0)
		return String();
	if(l>len())
		l=len();
	return str.substr(s, l);
}

const String String::trim() const
{
	wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
	const wchar_t *p = ptr();
	_tcscpy_s(sp, len() + 1, p);
	wchar_t *ep=sp+len()-1;
	// bug #46 fixed by axxie
	while (*sp && isbadchar(*sp)) sp++;
	while (ep>=sp && isbadchar(*ep)) ep--;
	*(ep+1)=0;
	return sp;
}

const String String::ltrim() const
{
	wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
	const wchar_t *p=ptr();
	_tcscpy_s(sp, len() + 1, p);
	// bug #46 fixed by axxie
	while (*sp && isbadchar(*sp)) sp++;
	return sp;
}

const String String::rtrim() const
{
	wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
	const wchar_t *p=ptr();
	_tcscpy_s(sp, len() + 1, p);
	wchar_t *ep=sp+len()-1;
	// bug #46 fixed by axxie
	while (ep>=sp && isbadchar(*ep)) ep--;
	*(ep+1)=0;
	return sp;
}

const String String::trimquotes() const
{
	wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
	const wchar_t *p=ptr();
	_tcscpy_s(sp, len() + 1, p);
	wchar_t *ep=sp+len()-1;
	if (*sp && *sp=='"') sp++;
	if (ep>=sp && *ep=='"') *ep=0;
	return sp;
}

const String String::rev() const
{
	wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
	const wchar_t *p=ptr();
	wchar_t *ep=sp+len()-1;
	*(ep+1)=0;
	while (*p) *ep--=*p++;
	return sp;
}

const String String::replace(const String &what, const String &with,
							 int nocase) const
{
	if (!what.len()) return *this;
	String res;
	int start=0, p;
	if (!nocase)
	{
		while (p=find(what, start),p!=-1)
		{
			res+=substr(start, p-start);
			res+=with;
			start=p+with.len();
		}
	}
	else
	{
		String what1=what.toUpper(), this1=toUpper();
		while (p=this1.find(what1, start),p!=-1)
		{
			res+=substr(start, p-start);
			res+=with;
			start=p+with.len();
		}
	}
	res+=substr(start);
	return res;
}

const String String::toUpper() const
{
	wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
	const wchar_t *p=ptr();
	_tcscpy_s(sp, len()+1, p);
	CharUpperBuff(sp, len());
	return sp;
}

const String String::toLower() const
{
	wchar_t *sp=(wchar_t*)_alloca((len()+1)*sizeof(wchar_t));
	const wchar_t *p=ptr();
	_tcscpy_s(sp, len() + 1, p);
	CharLowerBuff(sp, len());
	return sp;
}
