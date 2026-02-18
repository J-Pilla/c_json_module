#pragma once
typedef struct SLNode SLNode;

typedef struct StringList
{
	SLNode* firstNode;
	int length;
} StringList;

StringList* SLConstructor();
void SLDestructor(StringList* list);
const char* SLGetter(StringList* list, int index);
int SLSetter(StringList* list, char* value, int index);
void SLPush(StringList* list, char* value);
int SLPop(StringList* list);
