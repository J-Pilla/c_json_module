#include "JSON.h"

#include <assert.h>
#include <ctype.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SUCCESS 0
#define FAILURE 1

#define ERROR_PARAM_NULL "NULL pointer passed"
#define ERROR_FILE_OPEN "unable to open file"
#define ERROR_FILE_FORMAT "file cannot be parsed"
#define ERROR_FILE_CLOSE "unable to close file"
#define ERROR_FILE_EMPTY "file empty"
#define ERROR_OUT_OF_BOUNDS "out of bounds of the this"
#define ERROR_LIST_EMPTY "list is empty"
#define ERROR_KEY_NOT_FOUND "key not found in this"
#define ERROR_KEY_EXISTS "key already exists in this"
#define ERROR_MAP_EMPTY "map is empty"
#define ERROR_ARRAY_EMPTY "array is empty"

typedef struct JSONObjectList ObjectList;
#define EMPTY_O_LIST (ObjectList){ NULL, 0 }

typedef struct JSONArrayList ArrayList;
#define EMPTY_A_LIST (ArrayList){ NULL, 0 }

typedef struct JSONObjectMap ObjectMap;
#define EMPTY_O_MAP (ObjectMap){ NULL }

typedef struct JSONArrayMap ArrayMap;
#define EMPTY_A_MAP (ArrayMap){ NULL }

typedef struct JSONObject Object;
#define EMPTY_OBJECT (Object)	\
{								\
	EMPTY_O_MAP,				\
	EMPTY_A_MAP,				\
	EMPTY_STRING_MAP,			\
	NULL						\
}								\

typedef struct JSONArray Array;
#define EMPTY_ARRAY (Array)	\
{								\
	EMPTY_O_LIST,				\
	EMPTY_A_LIST,				\
	EMPTY_STRING_LIST,			\
	NULL						\
}								\

typedef struct OLNode
{
	Object value;
	struct OLNode* next;
} OLNode;

typedef struct ALNode
{
	Array value;
	struct OLNode* next;
} ALNode;

typedef struct OMNode
{
	char* key;
	Object value;
	struct OMNode* next;
} OMNode;

typedef struct AMNode
{
	char* key;
	Array value;
	struct AMNode* next;
} AMNode;

typedef uint8_t Type;
#define OBJECT 1
#define ARRAY 2
#define STRING 3
#define NUMBER 4
#define BOOLEAN 5

static char* fileToString(const char* const);

// object list functions
static int freeObjectList(ObjectList*);
static int objectListPush(ObjectList*, Object);

// array list functions
static int freeArrayList(ArrayList*);
static int arrayListPush(ArrayList*, Array);

// object map functions
static int freeObjectMap(ObjectMap*);
static int objectMapAdd(ObjectMap*, Object, const char*);

// array map functions
static int freeArrayMap(ArrayMap*);
static int arrayMapAdd(ArrayMap*, Array, const char*);

// object functions
static Object* objectAllocateObject(Object*, const char*);
static Array* objectAllocateArray(Object*, const char*);
static int freeObject(Object*);

// array functions
static Object* arrayAllocateObject(Array*);
static Array* arrayAllocateArray(Array*);
static int freeArray(Array*);

// key / value functions
static char* allocateString(const char*, size_t*);
static char* allocateNumber(const char*, size_t*);

// type functions
static int pushType(Type**, Type, int*);
static int popType(Type**, int*);

static int error(const char*);
inline static void pause();

ObjectList JSONParse(const char* file)
{
	char* JSON = fileToString(file);
	size_t JSONLength = strlen(JSON);

	if (JSON == NULL || sizeof(JSON) < 5)
	{
		error(ERROR_FILE_FORMAT);
		return EMPTY_O_LIST;
	}

	// typeStack tracks the hiarchy of the JS objects
	Type* typeStack = NULL;
	int stackDepth = 0;

	// cursor walks along JSON
	size_t cursor = 0;

	for (; cursor < JSONLength - 1; cursor++)
	{
		// check if the file starts with an array or object
		if (JSON[cursor] == '[')
		{
			pushType(&typeStack, ARRAY, &stackDepth);

			if (stackDepth == 2)
				break;
		}
		else
		{
			if (JSON[cursor] == '{')
			{
				pushType(&typeStack, OBJECT, &stackDepth);
				cursor++;
			}
			break;
		}
	}

	if (typeStack == NULL || typeStack[stackDepth - 1] != OBJECT)
	{
		free(JSON);
		free(typeStack);
		error(ERROR_FILE_FORMAT);
		return EMPTY_O_LIST;
	}

	ObjectList list = EMPTY_O_LIST;

	// an object to build the current node
	Object objectBuilder = EMPTY_OBJECT;
	Object* currentObject = &objectBuilder;

	// or an array to build the current node
	Array arrayBuilder = EMPTY_ARRAY;
	Array* currentArray = &arrayBuilder;

	char* value = NULL;

	// ROOT tracks the base level for objects
	const int ROOT = stackDepth - 1;

	for (; cursor < JSONLength - 1; cursor++)
	{
		if (typeStack[stackDepth - 1] == OBJECT)
		{
			char* key = NULL;

			if (JSON[cursor] == '\"') // signs a key
			{
				key = allocateString(JSON, &cursor);

				if (key == NULL)
				{
					free(JSON);
					free(typeStack);
					freeObjectList(&list);
					return EMPTY_O_LIST;
				}

				cursor += 2;

				switch (JSON[cursor])
				{
				case '{':
					currentObject = objectAllocateObject(currentObject, key);

					if (currentObject == NULL)
					{
						free(JSON);
						free(typeStack);
						freeObjectList(&list);
						free(key);
						return EMPTY_O_LIST;
					}

					pushType(
						&typeStack,
						OBJECT,
						&stackDepth
					);

					break;
				case '[':
					currentArray = objectAllocateArray(currentObject, key);

					if (currentArray == NULL)
					{
						free(JSON);
						free(typeStack);
						freeObjectList(&list);
						free(key);
						return EMPTY_O_LIST;
					}

					pushType(
						&typeStack,
						ARRAY,
						&stackDepth
					);

					break;
				case '\"':
					SMAdd(
						&currentObject->values,
						allocateString(JSON, &cursor),
						key
					);

					if (SMGetString(&currentObject->values, key) == NULL)
					{
						free(JSON);
						free(typeStack);
						freeObjectList(&list);
						free(key);
						return EMPTY_O_LIST;
					}
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
					SMAdd(
						&currentObject->values,
						allocateNumber(JSON, &cursor),
						key
					);

					if (SMGetString(&currentObject->values, key) == NULL)
					{
						free(JSON);
						free(typeStack);
						freeObjectList(&list);
						free(key);
						return EMPTY_O_LIST;
					}
					break;
				case 't':
				case 'f':
					SMAdd(
						&currentObject->values,
						JSON[cursor] == 't' ? "true" : "false",
						key
					);

					if (SMGetString(&currentObject->values, key) == NULL)
					{
						free(JSON);
						free(typeStack);
						freeObjectList(&list);
						free(key);
						return EMPTY_O_LIST;
					}

					cursor += JSON[cursor] == 't' ? 3 : 4;
				}

				free(key);
				key = NULL;
			}
			else if (JSON[cursor] == '}')
			{
				popType(&typeStack, &stackDepth);

				if (typeStack[stackDepth - 1] == OBJECT)
					currentObject = currentObject->parent;
				else
				{
					currentArray = currentObject->parent;
					currentObject = NULL;
				}
			}
		}
		else // type == array
		{
			switch (JSON[cursor])
			{
			case '{':
				currentObject = arrayAllocateObject(currentArray);

				if (currentObject == NULL)
				{
					free(JSON);
					free(typeStack);
					freeObjectList(&list);
					return EMPTY_O_LIST;
				}

				pushType(
					&typeStack,
					OBJECT,
					&stackDepth
				);
				break;
			case '[':
				currentArray = arrayAllocateArray(currentArray);

				if (currentArray== NULL)
				{
					free(JSON);
					free(typeStack);
					freeObjectList(&list);
					return EMPTY_O_LIST;
				}

				pushType(
					&typeStack,
					ARRAY,
					&stackDepth
				);
				break;
			case '\"':
				SLPush(&currentArray->values, allocateString(JSON, &cursor));

				if (SLGetString(&currentArray->values, currentArray->values.length - 1) == NULL)
				{
					free(JSON);
					free(typeStack);
					freeObjectList(&list);
					return EMPTY_O_LIST;
				}
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
				SLPush(&currentArray->values, allocateNumber(JSON, &cursor));

				if (SLGetString(&currentArray->values, currentArray->values.length - 1) == NULL)
				{
					free(JSON);
					free(typeStack);
					freeObjectList(&list);
					return EMPTY_O_LIST;
				}
				break;
			case 't':
			case 'f':
				SLPush(
					&currentArray->values,
					JSON[cursor] == 't' ? "true" : "false"
				);

				if (SLGetString(&currentArray->values, currentArray->values.length - 1) == NULL)
				{
					free(JSON);
					free(typeStack);
					freeObjectList(&list);
					return EMPTY_O_LIST;
				}

				cursor += JSON[cursor] == 't' ? 3 : 4;
				break;
			case ']':
				popType(&typeStack, &stackDepth);

				if (typeStack[stackDepth - 1] == ARRAY)
					currentArray = currentArray->parent;
				else
				{
					currentObject = currentArray->parent;
					currentArray = NULL;
				}
			}
		}

		if (stackDepth == ROOT && JSON[cursor] == ',')
		{
			objectListPush(&list, objectBuilder);
			objectBuilder = EMPTY_OBJECT;
			currentObject = &objectBuilder;
			pushType(&typeStack, OBJECT, &stackDepth);
			cursor++;
		}
	}

	objectListPush(&list, objectBuilder);
	free(JSON);
	free(typeStack);
	return list;
}

int JSONFree(JSONObjectList* this) { freeObjectList(this); }

JSONObject* OLGetObject(JSONObjectList* this, int index)
{
	if (this == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	if (index < 0 || index >= this->length)
	{
		error(ERROR_OUT_OF_BOUNDS);
		return NULL;
	}

	OLNode* currentNode = this->first;

	for (int ctr = 0; ctr < index; ctr++)
		currentNode = currentNode->next;

	return &currentNode->value;
}

JSONArray* ALGetArray(JSONArrayList* this, int index)
{
	if (this == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	if (index < 0 || index >= this->length)
	{
		error(ERROR_OUT_OF_BOUNDS);
		return NULL;
	}

	ALNode* currentNode = this->first;

	for (int ctr = 0; ctr < index; ctr++)
		currentNode = currentNode->next;

	return &currentNode->value;
}

JSONObject* OMGetObject(JSONObjectMap* this, const char* key)
{
	if (this == NULL || key == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	OMNode* node = this->first;

	while (node != NULL)
	{
		if (strcmp(node->key, key) == 0)
			return &node->value;

		node = node->next;
	}

	error(ERROR_KEY_NOT_FOUND);
	return NULL;
}

JSONArray* AMGetArray(JSONArrayMap* this, const char* key)
{
	if (this == NULL || key == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	AMNode* node = this->first;

	while (node != NULL)
	{
		if (strcmp(node->key, key) == 0)
			return &node->value;

		node = node->next;
	}

	error(ERROR_KEY_NOT_FOUND);
	return NULL;
}

static char* fileToString(const char* const file)
{
	FILE* fin = fopen(file, "r");

	if (fin == NULL)
	{
		printf("%s | ", file);
		error(ERROR_FILE_OPEN);
		return NULL;
	}

	char** buffer = calloc(1, sizeof(char*));
	assert(buffer);

	const int BUFFER = 80;
	int bufferCount = 0;

	size_t cursor = 0, characterCount = 0;

	// while loop header
	{
		int previousInput = 0;
		bool isInString = false;
		while (!feof(fin))
		{
			if (cursor == BUFFER - 1)
			{
				buffer[bufferCount][cursor] = '\0';
				bufferCount++;
				cursor = 0;
				char** temp = realloc(buffer, sizeof(char*) * (bufferCount + 1));
				assert(temp);
				buffer = temp;
			}

			if (cursor == 0)
			{
				buffer[bufferCount] = malloc(BUFFER);
				assert(buffer[bufferCount]);
			}

			int input = fgetc(fin);

			if ((char)input == '"' && (char)previousInput != '\\')
				isInString = !isInString;

			if (isspace(input) != 0 && !isInString)
				continue;

			previousInput = input;
			buffer[bufferCount][cursor] = input >= 0 ? (char)input : '\0';
			characterCount++;
			cursor++;
		}
	}

	if (characterCount < 2)
	{
		free(buffer[0]);
		free(buffer);

		printf("%s | ", file);
		error(ERROR_FILE_FORMAT);
		return NULL;
	}

	char* string = calloc(characterCount, sizeof(char));
	assert(string);

	cursor = 0;

	for (int stringIndex = 0, bufferIndex = 0;
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
		printf("%s | ", file);
		error(ERROR_FILE_CLOSE);
		return NULL;
	}

	return string;
}

// object list functions
static int freeObjectList(ObjectList* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	OLNode* node = this->first;

	while (node != NULL)
	{
		this->first = this->first->next;
		freeObject(&node->value);
		free(node);
		node = this->first;
	}

	this->length = 0;

	return SUCCESS;
}

static int objectListPush(ObjectList* this, Object object)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	OLNode* objectBuilder = malloc(sizeof(OLNode));
	assert(objectBuilder);

	*objectBuilder = (OLNode)
	{
		object,
		NULL
	};

	if (this->first)
	{
		OLNode* node = this->first;
		while (node->next)
			node = node->next;

		node->next = objectBuilder;
	}
	else
		this->first = objectBuilder;

	this->length++;

	return SUCCESS;
}

// array list functions
static int freeArrayList(JSONArrayList* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	ALNode* node = this->first;

	while (node != NULL)
	{
		this->first = this->first->next;
		freeArray(&node->value);
		free(node);
		node = this->first;
	}

	this->length = 0;

	return SUCCESS;
}

static int arrayListPush(ArrayList* this, Array array)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	ALNode* objectBuilder = malloc(sizeof(ALNode));
	assert(objectBuilder);

	*objectBuilder = (ALNode)
	{
		array,
		NULL
	};

	if (this->first)
	{
		ALNode* node = this->first;
		while (node->next)
			node = node->next;

		node->next = objectBuilder;
	}
	else
		this->first = objectBuilder;

	this->length++;

	return SUCCESS;
}

// object map functions
static int freeObjectMap(ObjectMap* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	OMNode* node = this->first;

	while (node != NULL)
	{
		this->first = this->first->next;
		free(node->key);
		freeObject(&node->value);
		free(node);
		node = this->first;
	}

	return SUCCESS;
}

static int objectMapAdd(ObjectMap* this, Object value, const char* key)
{
	if (this == NULL || key == NULL)
		return error(ERROR_PARAM_NULL);

	OMNode* objectBuilder = malloc(sizeof(OMNode));
	assert(objectBuilder);

	size_t size = strlen(key) + 1ull;

	*objectBuilder = (OMNode)
	{
		malloc(size),
		value,
		NULL
	};
	assert(objectBuilder->key);

	strcpy_s(objectBuilder->key, size, key);

	if (this->first != NULL)
	{
		OMNode* node = this->first;

		while (node->next != NULL)
		{
			if (strcmp(node->key, key) == 0)
			{
				free(objectBuilder->key);
				free(objectBuilder);
				return error(ERROR_KEY_EXISTS);
			}

			node = node->next;
		}

		node->next = objectBuilder;
	}
	else
		this->first = objectBuilder;

	return SUCCESS;
}

// array map functions
static int freeArrayMap(ArrayMap* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	AMNode* node = this->first;

	while (node != NULL)
	{
		this->first = this->first->next;
		free(node->key);
		freeArray(&node->value);
		free(node);
		node = this->first;
	}

	return SUCCESS;
}

static int arrayMapAdd(ArrayMap* this, Array value, const char* key)
{
	if (this == NULL || key == NULL)
		return error(ERROR_PARAM_NULL);

	AMNode* objectBuilder = malloc(sizeof(AMNode));
	assert(objectBuilder);

	size_t size = strlen(key) + 1ull;

	*objectBuilder = (AMNode)
	{
		malloc(size),
		value,
		NULL
	};
	assert(objectBuilder->key);

	strcpy_s(objectBuilder->key, size, key);

	if (this->first != NULL)
	{
		AMNode* node = this->first;

		while (node->next != NULL)
		{
			if (strcmp(node->key, key) == 0)
			{
				free(objectBuilder->key);
				free(objectBuilder);
				return error(ERROR_KEY_EXISTS);
			}

			node = node->next;
		}

		node->next = objectBuilder;
	}
	else
		this->first = objectBuilder;

	return SUCCESS;
}

// object functions
static Object* objectAllocateObject(Object* this, const char* key)
{
	if (this == NULL || key == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	ObjectMap* map = &this->objects;
	objectMapAdd(map, EMPTY_OBJECT, key);

	Object* object = OMGetObject(map, key);
	object->parent = this;

	return object;
}

static Array* objectAllocateArray(Object* this, const char* key)
{
	if (this == NULL || key == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	ArrayMap* map = &this->arrays;
	arrayMapAdd(map, EMPTY_ARRAY, key);

	Array* array = AMGetArray(map, key);
	array->parent = this;

	return array;
}

static int freeObject(Object* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	freeObjectMap(&this->objects);
	freeArrayMap(&this->objects);
	SMFreeMap(&this->values);
	this->parent = NULL;

	return SUCCESS;
}

// array functions
static Object* arrayAllocateObject(Array* this)
{
	if (this == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	ObjectList* list = &this->objects;
	objectListPush(list, EMPTY_OBJECT);

	Object* object = OLGetObject(list, list->length - 1);
	object->parent = this;

	return object;
}

static Array* arrayAllocateArray(Array* this)
{
	if (this == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	ArrayList* list = &this->arrays;
	arrayListPush(list, EMPTY_ARRAY);

	Array* array = ALGetArray(list, list->length - 1);
	array->parent = this;

	return array;
}

static int freeArray(JSONArray* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	freeObjectList(&this->objects);
	freeArrayList(&this->objects);
	SLFreeList(&this->values);
	this->parent = NULL;

	return SUCCESS;
}

// key / value functions
static char* allocateString(const char* JSON, size_t* cursor)
{
	int size = 0;
	while (JSON[*cursor + ++size] != '\0')
	{
		if (JSON[*cursor + size] == '\"' && JSON[*cursor + size - 1] != '\\')
			break;
	}

	if (JSON[*cursor + size] == '\0')
	{
		error(ERROR_FILE_FORMAT);
		return NULL;
	}

	char* string = malloc(size);
	assert(string);

	int index = 0;

	for (; index < size - 1; index++)
	{
		string[index] = JSON[*cursor + index + 1];
	}

	string[index] = '\0';

	*cursor += size;

	return string;
}

static char* allocateNumber(const char* JSON, size_t* cursor)
{
	int size = 0;
	(*cursor)--;
	while (JSON[*cursor + ++size] != '\0')
	{
		bool isBreaking = false;

		switch (JSON[*cursor + size])
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
			break;
		default:
			isBreaking = true;
		}

		if (isBreaking)
			break;
	}

	if (JSON[*cursor + size] == '\0')
	{
		error(ERROR_FILE_FORMAT);
		return NULL;
	}

	char* string = malloc(size);
	assert(string);

	int index = 0;

	for (; index < size - 1; index++)
	{
		string[index] = JSON[*cursor + index + 1];
	}

	string[index] = '\0';

	*cursor += size - 1;

	return string;
}

// type functions
static int pushType(Type** stack, Type attribute, int* depth)
{
	if (stack == NULL || depth == NULL)
		return error(ERROR_PARAM_NULL);

	(*depth)++;

	if (*stack == NULL)
	{
		*stack = calloc(*depth, sizeof(Type));
		assert(*stack);
	}
	else
	{
		Type* temp = realloc(*stack, sizeof(Type) * (*depth));
		assert(temp);
		*stack = temp;
	}

	(*stack)[(*depth) - 1] = attribute;

	return SUCCESS;
}

static int popType(Type** stack, int* depth)
{
	if (stack == NULL || depth == NULL)
		return error(ERROR_PARAM_NULL);

	if (depth == 0)
		return error(ERROR_ARRAY_EMPTY);

	(*depth)--;

	if (*depth > 0)
	{
		Type* temp = realloc(*stack, sizeof(Type) * (*depth));
		assert(temp);
		*stack = temp;
	}
	else
	{
		free(*stack);
		*stack = NULL;
	}

	return SUCCESS;
}

static int error(const char* error)
{
	if (error == NULL)
		error = "";

	printf("Error: %s\n", error);
	pause();
	return FAILURE;
}

inline static void pause()
{
	printf("press enter to continue . . . ");
	int key = getchar();
	fflush(stdin);
}
