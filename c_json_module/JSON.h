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
const JSONObject* GetObject(const ObjectList* list, const int index);
void FreeObjectList(ObjectList* list);
