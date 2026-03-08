#pragma once

#include "StringMap.h"
#include "StringList.h"

typedef struct JSONList
{
	struct JSONListNode* first;
	int length;
} JSONList;

typedef struct JSONMap
{
	struct JSONMapNode* first;
} JSONMap;

typedef struct JSONObject
{
	JSONMap objects;
	JSONMap arrays;
	StringMap values;
	void* parent;
} JSONObject;

typedef struct JSONArray
{
	JSONList objects;
	JSONList arrays;
	StringList values;
	void* parent;
} JSONArray;

JSONList JSONParse(const char* file);
int JSONFree(JSONList* this);

union JSON* JSONListGet(JSONList* this, int index);
union JSON* JSONMapGet(JSONMap* this, const char* key);
