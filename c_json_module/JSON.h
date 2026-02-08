#pragma once
typedef struct OLNode OLNode;

// objects and arrays are treated as the same
typedef struct JSONObject
{
	struct JSONObject* objects;
	int objectCount;
	char** values;
	int valueCount;
	struct JSONObject* parent;
} JSONObject;

#define EMPTY_OBJECT = (JSONObject){ (void *)0, 0, (void *)0, 0, (void *)0 }

typedef struct ObjectList
{
	OLNode* firstNode;
	int length;
} ObjectList;

#define EMPTY_OBJECT_LIST (ObjectList){ (void *)0, 0 }

ObjectList ParseJSON(const char* file);
const JSONObject* GetObject(const ObjectList* list, const int index);
void FreeObjectList(ObjectList* list);
