#include "JSON.h"

#include <assert.h>
#include <ctype.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define EMPTY_OBJECT_LIST (ObjectList){ ((void *)0), 0 }

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
static void pushDepth(Type**, Type, int*);
static void popDepth(Type**, int*);
static void allocateObject(JSONObject**);
static void allocateInput(char**, size_t);
static void setValue(char**, JSONObject*, int*);
static void setValueTrue(JSONObject*, int*);
static void setValueFalse(JSONObject*, int*);
static void freeObject(JSONObject*);

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
	JSONObject builder = { NULL, 0, NULL, NULL };
	builder.values = SLConstructor();
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
					pushDepth(&depthTypes, Number, &depth);
					cursor--;
					break;
				case 't':
					setValueTrue(currentObject, &cursor);
					break;
				case 'f':
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
				pushDepth(&depthTypes, Number, &depth);
				cursor--;
				break;
			case 't':
				setValueTrue(currentObject, &cursor);
				break;
			case 'f':
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
			builder = (JSONObject){ NULL, 0, NULL, NULL };
			builder.values = SLConstructor();
			currentObject = &builder;
			pushDepth(&depthTypes, Object, &depth);
			cursor++;
		}
	}

	pushListObject(&list, &builder);
	free(JSON);
	return list;
}

const JSONObject* GetObject(const ObjectList* list, int index)
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

		for (int index = 0; index < node->value.objectCount; index++)
		{
			if (node->value.objects != NULL)
			{
				freeObject(&node->value.objects[index]);
			}
		}
		free(node->value.objects);
		node->value.objects = NULL;

		SLDestructor(node->value.values);

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

	char** buffer = calloc(1, sizeof(char*));
	assert(buffer);

	const int BUFFER = 80;
	int characterCount = 0, bufferCount = 0;
	{ // while loop header
		int cursor = 0, input, previousInput = 0;
		bool isInString = false;
		while (!feof(fin))
		{
			if (cursor == BUFFER - 1)
			{
				buffer[bufferCount][cursor] = '\0';
				bufferCount++;
				cursor = 0;
				char** temp = realloc(buffer, sizeof(char*) * (uint64_t)(bufferCount + 1));
				assert(temp);
				buffer = temp;
			}

			if (cursor == 0)
			{
				buffer[bufferCount] = calloc(BUFFER, sizeof(char));
				assert(buffer[bufferCount]);
			}

			input = fgetc(fin);
			
			if ((char)input == '"' && (char)previousInput != '\\')
			{
				isInString = !isInString;
			}

			if (isspace(input) != 0 && !isInString)
			{
				continue;
			}

			previousInput = input;
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

	char* string = calloc(characterCount, sizeof(char));
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
	OLNode* builder = malloc(sizeof(OLNode));
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

static void pushDepth(Type** array, Type attribute, int* depth)
{
	(*depth)++;

	if (*array == NULL)
	{
		*array = calloc(*depth, sizeof(Type));
		assert(*array);
	}
	else
	{
		Type* temp = realloc(*array, sizeof(Type) * (*depth));
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
		Type* temp = realloc(*array, sizeof(Type) * (*depth));
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
		(*currentObject)->objects = calloc(1, sizeof(JSONObject));
		assert((*currentObject)->objects);
	}
	else
	{
		JSONObject* temp = realloc((*currentObject)->objects, sizeof(JSONObject) * (*currentObject)->objectCount);
		assert(temp);
		(*currentObject)->objects = temp;
	}

	(*currentObject)->objects[(*currentObject)->objectCount - 1] = (JSONObject){ NULL, 0, NULL, *currentObject };
	(*currentObject)->objects[(*currentObject)->objectCount - 1].values = SLConstructor();
	*currentObject = &(*currentObject)->objects[(*currentObject)->objectCount - 1];
}

static void allocateInput(char** input, size_t count)
{
	if (*input == NULL)
	{
		*input = calloc(count + 1, sizeof(char));
	}
	else
	{
		char* temp = realloc(*input, sizeof(char) * count + 1);
		assert(temp);
		*input = temp;
	}
}

static void setValue(char** input, JSONObject* value, int* count)
{
	(*input)[*count] = '\0';
	SLPush(value->values, *input);
	*input = NULL;
	*count = 0;
}

static void setValueTrue(JSONObject* object, int* cursor)
{
	char* input = calloc(sizeof("true"), sizeof(char));
	assert(input);
	input[0] = 't';
	input[1] = 'r';
	input[2] = 'u';
	input[3] = 'e';
	input[4] = '\0';
	SLPush(object->values, input);
	(*cursor) += 3;
}

static void setValueFalse(JSONObject* object, int* cursor)
{
	char* input = calloc(sizeof("false"), sizeof(char));
	assert(input);
	input[0] = 'f';
	input[1] = 'a';
	input[2] = 'l';
	input[3] = 's';
	input[4] = 'e';
	input[5] = '\0';
	SLPush(object->values, input);
	(*cursor) += 4;
}

static void freeObject(JSONObject* object)
{
	for (int index = 0; index < object->objectCount; index++)
	{
		if (object->objects != NULL)
		{
			freeObject(&object->objects[index]);
		}
	}
	free(object->objects);
	object->objects = NULL;

	SLDestructor(object->values);
}