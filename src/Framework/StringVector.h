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
	virtual size_t AddString(const String& v) { data.push_back(v); return data.size()-1;};
	virtual size_t Count() const { return data.size(); };

private:
	std::vector<String> data;
};

#endif