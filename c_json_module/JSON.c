#include "JSON.h"
#include <assert.h>
#include <ctype.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct OLNode
{
	JSONObject value;
	OLNode* link;
};

typedef uint8_t Type;
#define Object 1
#define Array 2
#define String 3
#define Number 4
#define Boolean 5

static char* fileToString(const char*);
static void pushListObject(ObjectList*, const JSONObject*);
static void pushDepth(Type**, const Type, int*);
static void popDepth(Type**, int*);
static void allocateObject(JSONObject**);
static void allocateValue(JSONObject**);
static void allocateInput(char**, const int);
static void setValue(char**, JSONObject*, int*);
static void setValueTrue(JSONObject*, int*);
static void setValueFalse(JSONObject*, int*);
static void freeObjectArray(JSONObject*);
static void freeStringArray(char**, const int);

ObjectList ParseJSON(const char* file)
{
	char* JSON = fileToString(file);

	if (JSON == NULL || sizeof(JSON) < 5)
	{
		return EMPTY_OBJECT_LIST;
	}

	// depth tracks the hiarchy of the JS objects
	Type* depthTypes = NULL;
	int depth = 0;
	// cursor walks along JSON
	int cursor = 0;

	for (; cursor < strlen(JSON) - 1; cursor++)
	{
		// check if the file starts with an array or object
		if (JSON[cursor] == '[')
		{
			pushDepth(&depthTypes, Array, &depth);
			if (depth == 2)
			{
				break;
			}
		}
		else
		{
			if (JSON[cursor] == '{')
			{
				pushDepth(&depthTypes, Object, &depth);
				cursor++;
			}
			break;
		}
	}

	if (depthTypes == NULL || depthTypes[depth - 1] != Object)
	{
		free(JSON);
		free(depthTypes);
		return EMPTY_OBJECT_LIST;
	}

	ObjectList list = { NULL, 0 };

	// an object to build the current node
	JSONObject builder = { NULL, 0, NULL, 0, NULL };
	char* strInput = NULL;

	JSONObject* currentObject = &builder;
	// ROOT tracks the base level for objects
	const int ROOT = depth - 1;
	// inCount is how many characters in a value
	int inCount = 0;

	for (; cursor < strlen(JSON) - 1; cursor++)
	{
		switch (depthTypes[depth - 1])
		{
		case Object:
			switch (JSON[cursor])
			{
			case '\"': // signs a key, keys are ignored
				cursor++;
				for (; cursor < strlen(JSON) - 1; cursor++)
				{
					if (JSON[cursor] == '\"' && JSON[cursor - 1] != '\\')
					{
						break;
					}
				}
				break;
			case ':': // ':' signs a value
				cursor++;
				switch (JSON[cursor])
				{
				case '{':
					allocateObject(&currentObject);
					pushDepth(&depthTypes, Object, &depth);
					break;
				case '[':
					allocateObject(&currentObject);
					pushDepth(&depthTypes, Array, &depth);
					break;
				case '\"':
					allocateValue(&currentObject);
					pushDepth(&depthTypes, String, &depth);
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					allocateValue(&currentObject);
					pushDepth(&depthTypes, Number, &depth);
					cursor--;
					break;
				case 't':
					allocateValue(&currentObject);
					setValueTrue(currentObject, &cursor);
					break;
				case 'f':
					allocateValue(&currentObject);
					setValueFalse(currentObject, &cursor);
					break;
				}
				break;
			case '}':
				popDepth(&depthTypes, &depth);
				currentObject = currentObject->parent;
				break;
			}
			break;
		case Array:
			switch (JSON[cursor])
			{
			case '{':
				allocateObject(&currentObject);
				pushDepth(&depthTypes, Object, &depth);
				break;
			case '[':
				allocateObject(&currentObject);
				pushDepth(&depthTypes, Array, &depth);
				break;
			case '\"':
				allocateValue(&currentObject);
				pushDepth(&depthTypes, String, &depth);
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				allocateValue(&currentObject);
				pushDepth(&depthTypes, Number, &depth);
				cursor--;
				break;
			case 't':
				allocateValue(&currentObject);
				setValueTrue(currentObject, &cursor);
				break;
			case 'f':
				allocateValue(&currentObject);
				setValueFalse(currentObject, &cursor);
				break;
			case ']':
				popDepth(&depthTypes, &depth);
				currentObject = currentObject->parent;
				break;
			}
			break;
		case String:
			allocateInput(&strInput, inCount);
			switch (JSON[cursor])
			{
			case '\\':
				cursor++;
				switch (JSON[cursor])
				{
				case 'n':
					strInput[inCount] = '\n';
					break;
				case 'f':
					strInput[inCount] = '\f';
					break;
				case 'r':
					strInput[inCount] = '\r';
					break;
				case '\\':
					strInput[inCount] = '\\';
					break;
				case 'b':
					strInput[inCount] = '\b';
					break;
				case 't':
					strInput[inCount] = '\t';
					break;
				case '\"':
					strInput[inCount] = '\"';
					break;
				case '\'':
					strInput[inCount] = '\'';
					break;
				default:
					strInput[inCount] = JSON[cursor];
					// \v and \0 aren't allowed in JSON strings... so wikipedia says
				}
				inCount++;
				break;
			case '\"':
				setValue(&strInput, currentObject, &inCount);
				popDepth(&depthTypes, &depth);
				break;
			default:
				strInput[inCount] = JSON[cursor];
				inCount++;
			}
			break;
		case Number:
			allocateInput(&strInput, inCount);
			switch (JSON[cursor])
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				strInput[inCount] = JSON[cursor];
				inCount++;
				break;
			default:
				setValue(&strInput, currentObject, &inCount);
				popDepth(&depthTypes, &depth);
				cursor--;
			}
			break;
		}

		if (depth == ROOT && JSON[cursor] == ',')
		{
			pushListObject(&list, &builder);
			builder = (JSONObject){ NULL, 0, NULL, 0, NULL };
			currentObject = &builder;
			pushDepth(&depthTypes, Object, &depth);
			cursor++;
		}
	}

	pushListObject(&list, &builder);
	free(JSON);
	return list;
}

const JSONObject* GetObject(const ObjectList* list, const int index)
{
	if (index < 0 || index >= list->length)
	{
		return NULL;
	}

	OLNode* currentNode = list->firstNode;

	for (int ctr = 0; ctr < index; ctr++)
		currentNode = currentNode->link;

	return &currentNode->value;
}

void FreeObjectList(ObjectList* list)
{
	OLNode* node = list->firstNode;

	while (node != NULL)
	{
		list->firstNode = list->firstNode->link;

		if (node->value.objects != NULL)
		{
			freeObjectArray(node->value.objects);
		}

		if (node->value.values != NULL)
		{
			freeStringArray(node->value.values, node->value.valueCount);
		}

		free(node);
		node = list->firstNode;
	}

	list->length = 0;
}

static char* fileToString(const char* file)
{
	FILE* fin = fopen(file, "r");

	if (fin == NULL)
	{
		printf("ERROR: unable to open %s\n", file);
		return NULL;
	}

	char** buffer = (char**)calloc(1, sizeof(char*));
	assert(buffer);

	const int BUFFER = 80;
	int characterCount = 0, bufferCount = 0;
	{ // while loop header
		int cursor = 0, input;
		while (!feof(fin))
		{
			if (cursor == BUFFER - 1)
			{
				buffer[bufferCount][cursor] = '\0';
				bufferCount++;
				cursor = 0;
				char** temp = (char**)realloc(buffer, sizeof(char*) * (uint64_t)(bufferCount + 1));
				assert(temp);
				buffer = temp;
			}

			if (cursor == 0)
			{
				buffer[bufferCount] = (char*)calloc(BUFFER, sizeof(char));
				assert(buffer[bufferCount]);
			}

			input = fgetc(fin);

			if (isspace(input) != 0)
			{
				continue;
			}

			buffer[bufferCount][cursor] = input >= 0 ? (char)input : '\0';
			characterCount++;
			cursor++;
		}
	}

	if (characterCount < 2)
	{
		printf("%s is empty", file);

		free(buffer[0]);
		free(buffer);

		return NULL;
	}

	char* string = (char*)calloc(characterCount, sizeof(char));
	assert(string);

	for (int stringIndex = 0, bufferIndex = 0, cursor = 0;
		stringIndex < characterCount;
		stringIndex++, cursor++)
	{
		if (cursor == BUFFER - 1)
		{
			bufferIndex++;
			cursor = 0;
		}

		string[stringIndex] = buffer[bufferIndex][cursor];
	}

	for (int index = 0; index < bufferCount; index++)
	{
		free(buffer[index]);
	}

	free(buffer);

	if (fclose(fin) != 0)
	{
		printf("ERROR: unable to close %s\n", file);

		return NULL;
	}

	return string;
}

static void pushListObject(ObjectList* list, const JSONObject* object)
{
	OLNode* builder = (OLNode*)malloc(sizeof(OLNode));
	assert(builder);

	builder->value = *object;
	builder->link = NULL;

	if (list->firstNode)
	{
		OLNode* node = list->firstNode;
		while (node->link)
			node = node->link;

		node->link = builder;
	}
	else
		list->firstNode = builder;

	list->length++;
}

static void pushDepth(Type** array, const Type attribute, int* depth)
{
	(*depth)++;

	if (*array == NULL)
	{
		*array = (Type*)calloc(*depth, sizeof(Type));
		assert(*array);
	}
	else
	{
		Type* temp = (Type*)realloc(*array, sizeof(Type) * (*depth));
		assert(temp);
		*array = temp;
	}

	(*array)[(*depth) - 1] = attribute;
}

static void popDepth(Type** array, int* depth)
{
	(*depth)--;

	if (depth > 0)
	{
		Type* temp = (Type*)realloc(*array, sizeof(Type) * (*depth));
		assert(temp);
		*array = temp;
	}
	else
	{
		free(*array);
		*array = NULL;
	}
}

static void allocateObject(JSONObject** currentObject)
{
	(*currentObject)->objectCount++;

	if ((*currentObject)->objects == NULL)
	{
		(*currentObject)->objects = (JSONObject*)calloc(1, sizeof(JSONObject));
		assert((*currentObject)->objects);
	}
	else
	{
		JSONObject* temp = (JSONObject*)realloc((*currentObject)->objects, sizeof(JSONObject) * (*currentObject)->objectCount);
		assert(temp);
		(*currentObject)->objects = temp;
	}

	(*currentObject)->objects[(*currentObject)->objectCount - 1] = (JSONObject){ NULL, 0, NULL, 0, *currentObject };
	*currentObject = &(*currentObject)->objects[(*currentObject)->objectCount - 1];
}

static void allocateValue(JSONObject** currentObject)
{
	if ((*currentObject)->values == NULL)
	{
		(*currentObject)->values = (char**)calloc(1, sizeof(char*));
		assert((*currentObject)->values);
	}
	else
	{
		char** temp = (char**)realloc((*currentObject)->values, sizeof(char*) * (*currentObject)->valueCount + 1);
		assert(temp);
		(*currentObject)->values = temp;
	}
}

static void allocateInput(char** input, const int count)
{
	if (*input == NULL)
	{
		*input = (char*)calloc((uint64_t)(count + 1), sizeof(char));
	}
	else
	{
		char* temp = (char*)realloc(*input, sizeof(char) * (uint64_t)(count + 1));
		assert(temp);
		*input = temp;
	}
}

static void setValue(char** input, JSONObject* value, int* count)
{
	// only on MONTH does an error occur (for both objects), both names, year, and day all work
	(*input)[*count] = '\0'; // value->valueCount = 1
	value->values[value->valueCount] = *input; // value->values[value->valueCount] = 20 / 4 (first list item / second list item), according to debugger
	value->valueCount++;
	*input = NULL;
	*count = 0;
}

static void setValueTrue(JSONObject* object, int* cursor)
{
	object->values[object->valueCount] = (char*)calloc(sizeof("true\0"), sizeof(char));
	assert(object->values[object->valueCount]);

	object->values[object->valueCount][0] = 't';
	object->values[object->valueCount][1] = 'r';
	object->values[object->valueCount][2] = 'u';
	object->values[object->valueCount][3] = 'e';
	object->values[object->valueCount][4] = '\0';

	object->valueCount++;
	(*cursor) += 3;
}

static void setValueFalse(JSONObject* object, int* cursor)
{
	object->values[object->valueCount] = (char*)calloc(sizeof("false\0"), sizeof(char*));
	assert(object->values[object->valueCount]);

	object->values[object->valueCount][0] = 'f';
	object->values[object->valueCount][1] = 'a';
	object->values[object->valueCount][2] = 'l';
	object->values[object->valueCount][3] = 's';
	object->values[object->valueCount][4] = 'e';
	object->values[object->valueCount][5] = '\0';

	object->valueCount++;
	(*cursor) += 4;
}

static void freeObjectArray(JSONObject* array)
{
	if (array->objects != NULL)
	{
		freeObjectArray(array->objects);
	}

	if (array->values != NULL)
	{
		freeStringArray(array->values, array->valueCount);
	}

	free(array);
}

static void freeStringArray(char** array, const int count)
{
	for (int index = 0; index < count; index++)
	{
		free(array[index]);
	}

	free(array);
}