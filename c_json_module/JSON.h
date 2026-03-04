#pragma once
#include "StringList.h"

typedef struct OLNode OLNode;

// objects and arrays are treated as the same
typedef struct JSONObject
{
	struct JSONObject* objects;
	int objectCount;
	StringList* values;
	struct JSONObject* parent;
} JSONObject;

typedef struct ObjectList
{
	OLNode* firstNode;
	int length;
} ObjectList;


ObjectList ParseJSON(const char* file);
int FreeObjectList(ObjectList* list);
const JSONObject* GetObject(const ObjectList* list, const int index);
