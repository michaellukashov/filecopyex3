#include "FileNameStoreEnum.h"
#include "../common.h"

String FileNameStoreEnum::GetNext()
{
	if (cur >= store->Count()) {
		return "";
	}

	const FileName &fn = (*store)[cur++];

	switch (fn.getDirection()) {
		case FileName::levelPlus: {
			if (!curPath.empty()) {
				curPath += "\\";
			};
			curPath += fn.getName();
			buffer = curPath;
			break;
		}

		case FileName::levelMinus: {
			buffer = curPath;
			curPath = curPath.substr(0, curPath.rfind('\\'));

			break;
		}

		case FileName::levelStar: {
			buffer = curPath;
			curPath = "";
			break;
		}

		case FileName::levelSame: {
			buffer = curPath;
			if (!buffer.empty()) {
				buffer += "\\";
			}
			buffer += fn.getName();
		}
	}

	return buffer;
}

void FileNameStoreEnum::Skip()
{
	if (cur >= store->Count()) {
		return;
	}

	const FileName &fn = (*store)[cur++];

	switch (fn.getDirection()) {
		case FileName::levelPlus: {
			if (!curPath.empty()) {
				curPath += "\\";
			};
			curPath += fn.getName();
			break;
		}

		case FileName::levelMinus: {
			curPath = curPath.substr(0, curPath.rfind('\\'));
			break;
		}

		case FileName::levelStar: {
			curPath = "";
			break;
		}
	}
}

void FileNameStoreEnum::ToFirst()
{
	cur = 0;
	curPath = "";
	buffer = "";
}

String FileNameStoreEnum::GetByNum(size_t n)
{
	if (cur > n+1) {
		FWError(L"FileNameStoreEnum::GetByNum - assertion failure");
		return "";
	}

	if (cur == n+1) {
		return buffer;
	} else {
		while (cur < n) Skip();
		return GetNext();
	}
}
