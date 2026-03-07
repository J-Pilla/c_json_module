#pragma once

#include "StringMap.h"
#include "StringList.h"

typedef struct JSONObjectList
{
	struct OLNode* first;
	int length;
} JSONObjectList;

typedef struct JSONArrayList
{
	struct ALNode* first;
	int length;
} JSONArrayList;

typedef struct JSONObjectMap
{
	struct OMNode* first;
} JSONObjectMap;

typedef struct JSONArrayMap
{
	struct AMNode* first;
} JSONArrayMap;

typedef struct JSONObject
{
	JSONObjectMap objects;
	JSONArrayMap arrays;
	StringMap values;
	void* parent;
} JSONObject;

typedef struct JSONArray
{
	JSONObjectList objects;
	JSONArrayList arrays;
	StringList values;
	void* parent;
} JSONArray;

JSONObjectList JSONParse(const char* file);
int JSONFree(JSONObjectList* this);

JSONObject* OLGetObject(JSONObjectList* this, int index);
JSONArray* ALGetArray(JSONArrayList* this, int index);
JSONObject* OMGetObject(JSONObjectMap* this, const char* key);
JSONArray* AMGetArray(JSONArrayMap* this, const char* key);
