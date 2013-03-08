#ifndef __STRINGVECTOR_H__
#define __STRINGVECTOR_H__

#include <vector>

#include "StringParent.h"

class StringVector: public StringParent
{
public:
	StringVector() {};
	virtual ~StringVector() {};

	virtual void Clear() { data.clear(); };
	virtual const String& operator[](size_t i) const { return data[i]; };
	virtual void AddString(const String& v) { data.push_back(v); };
	virtual size_t Count() const { return data.size(); };

private:
	std::vector<String> data;
};

#endif