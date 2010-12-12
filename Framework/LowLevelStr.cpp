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
#include "lowlevelstr.h"

void _atowcs(wchar_t *d, size_t size, const char *s)
{
	MultiByteToWideChar(CP_ACP, 0, s, -1, d, (int)size);
	d[size-1]=0;
}

const wchar_t* _tcsrpbrk(const wchar_t* string, const wchar_t* control)
{
	const wchar_t *wcset;
	const wchar_t *str = _tcsend(string) - 1;
	/* 1st char in control string stops search */
	while(str >= string)
	{
		for(wcset = control; *wcset; wcset++)
			if(*wcset == *str)
				return str;
		str--;
	}
	return NULL;
}
