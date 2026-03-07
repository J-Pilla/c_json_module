#pragma once

typedef struct StringMap
{
	struct SMNode* firstNode;
} StringMap;


#define EMPTY_STRING_MAP (StringMap){ (struct SMNode*)0 }

inline StringMap SMNewMap() { return EMPTY_STRING_MAP; }
int SMFreeMap(StringMap* this);
const char* SMGetString(StringMap* this, const char* key);
int SMSetString(StringMap* this, const char* value, const char* key);
int SMAdd(StringMap* this, const char* value, const char* key);
int SMRemove(StringMap* this, const char* key);
