#include "store.h"

Store::Store()
{
}

Store::~Store()
{
	for(int i = 0; i < Count(); ++i)
	{
		free(items[i].ptr);
	}
}

int Store::Add(int size)
{
	Item item;
	item.ptr = malloc(size);
	item.size = size;
	items.push_back(item);
	return (int)items.size() - 1;
}
