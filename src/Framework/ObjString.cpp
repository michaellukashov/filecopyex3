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

const bool isbadchar(wchar_t c) 
{ 
	return c >= '\0' && c <= ' '; 
}

String String::substr(size_t s, size_t l) const
{
	if (s >= len()) {
		return String();
	}
	return str.substr(s, l);
}

String String::trim() const
{
	int start = 0, end = len()-1;
	while (start <= end && isbadchar((*this)[start]) ) {
		start++;
	};
	while (end >= start && isbadchar((*this)[end]) ) {
		end--;
	};
	return substr(start, end-start+1);
}

String String::ltrim() const
{
	int start = 0, end = len()-1;
	while (start <= end && isbadchar((*this)[start]) ) {
		start++;
	};
	return substr(start, end-start+1);
}

String String::rtrim() const
{
	int start = 0, end = len()-1;
	while (end >= start && isbadchar((*this)[end]) ) {
		end--;
	};
	return substr(start, end-start+1);

}

String String::trimquotes() const
{
	int start = 0, end = len()-1;
	while (start <= end && ((*this)[start]) == '"' ) {
		start++;
	};
	while (end >= start && ((*this)[end]) == '"' ) {
		end--;
	};
	return substr(start, end-start+1);
}

String String::rev() const
{
	String res;
	res.str.assign(str.rend(), str.rbegin());
	return res;
}

String String::replace(const String &what, const String &with) const
{
	if (what.empty()) {
		return *this;
	}
	String res;
	int start=0;
	int p;
	while ((p = find(what, start)) != -1) {
		res += substr(start, p-start);
		res += with;
		start = p + what.len();
	}
	res += substr(start);
	return res;
}

String String::toUpper() const
{
	String res(str);
	CharUpperBuff((LPTSTR)res.c_str(), len());
	return res;
}

String String::toLower() const
{
	String res(str);
	CharLowerBuff((LPTSTR)res.c_str(), len());
	return res;
}

const String emptyString;

int npos_minus1(size_t pos)
{
	return (pos == std::string::npos) ? -1 : (int)pos;
}
