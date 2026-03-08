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

typedef struct JSONList List;
typedef struct JSONMap Map;
typedef struct JSONObject Object;
typedef struct JSONArray Array;

union JSON
{
	JSONObject object;
	JSONArray array;
};

typedef bool Type;
#define ARRAY 0
#define OBJECT 1

typedef struct JSONType
{
	union JSON json;
	Type type;
} JSON;

typedef struct JSONListNode
{
	JSON value;
	struct ListNode* next;
} ListNode;

typedef struct JSONMapNode
{
	char* key;
	JSON value;
	struct MapNode* next;
} MapNode;

#define EMPTY_LIST (List){ NULL, 0 }
#define EMPTY_MAP (Map){ NULL }

#define EMPTY_OBJECT (Object)	\
{								\
	EMPTY_MAP,					\
	EMPTY_MAP,					\
	EMPTY_STRING_MAP,			\
	NULL						\
}

#define EMPTY_ARRAY (Array)	\
{							\
	EMPTY_LIST,				\
	EMPTY_LIST,				\
	EMPTY_STRING_LIST,		\
	NULL					\
}

#define EMPTY_JSON (union JSON) { EMPTY_OBJECT }

#define blankJson(type) (JSON)	\
{								\
	EMPTY_JSON,					\
	type						\
}

static char* fileToString(const char* const);

// list functions
static JSON* allocateJsonL(union JSON*, Type);
static JSON* getJsonL(List*, int);
static int pushJsonL(List*, JSON);

// map functions
static JSON* allocateJsonM(union JSON*, Type, const char*);
static JSON* getJsonM(Map*, const char*);
static int addJsonM(Map*, JSON, const char*);

// key / value functions
static char* allocateString(const char*, size_t*);
static char* allocateNumber(const char*, size_t*);

// type functions
static int pushType(Type**, Type, int*);
static int popType(Type**, int*);

// memory cleanup
inline static int freeJson(JSON*);
static int freeObject(Object*);
static int freeArray(Array*);
static int freeList(List*);
static int freeMap(Map*);

// error handling
static int error(const char*);
inline static void pause();

JSONList JSONParse(const char* file)
{
	char* json = fileToString(file);
	size_t jsonLength = strlen(json);

	if (json == NULL || sizeof(json) < 5)
	{
		error(ERROR_FILE_FORMAT);
		return EMPTY_LIST;
	}

	// typeStack tracks the hiarchy of the JS objects
	Type* typeStack = NULL;
	int stackDepth = 0;

	// cursor walks along json
	size_t cursor = 0;

	for (; cursor < jsonLength - 1; cursor++)
	{
		// check if the file starts with an array or object
		if (json[cursor] == '[')
		{
			pushType(&typeStack, ARRAY, &stackDepth);

			if (stackDepth == 2)
				break;
		}
		else
		{
			if (json[cursor] == '{')
			{
				pushType(&typeStack, OBJECT, &stackDepth);
				cursor++;
			}
			break;
		}
	}

	if (typeStack == NULL || typeStack[stackDepth - 1] != OBJECT)
	{
		free(json);
		free(typeStack);
		error(ERROR_FILE_FORMAT);
		return EMPTY_LIST;
	}

	List this = EMPTY_LIST;

	// a json to build the current node
	JSON builder = blankJson(OBJECT);

	// a pointer to work on the current depth
	JSON* currentJson = &builder;

	// ROOT tracks the base level for objects
	const int ROOT = stackDepth - 1;

	for (; cursor < jsonLength - 1; cursor++)
	{
		if (currentJson->type == OBJECT)
		{
			char* key = NULL;

			if (json[cursor] == '\"') // signs a key
			{
				key = allocateString(json, &cursor);

				if (key == NULL)
				{
					free(json);
					free(typeStack);
					freeList(&this);
					return EMPTY_LIST;
				}

				cursor += 2;

				switch (json[cursor])
				{
				case '{':
				case '[':
				{
					Type type = json[cursor] == '{' ? OBJECT : ARRAY;
					currentJson = allocateJsonM(&currentJson->json, type, key);

					if (currentJson == NULL)
					{
						free(json);
						free(typeStack);
						freeList(&this);
						free(key);
						return EMPTY_LIST;
					}

					pushType(
						&typeStack,
						type,
						&stackDepth
					);
					break;
				} 
				case '\"':
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
				case 't':
				case 'f':
					if (json[cursor] == 't')
					{
						SMAdd(
							&currentJson->json.object.values,
							"true",
							key
						);
						cursor += 3;
					}
					else if (json[cursor] == 'f')
					{
						SMAdd(
							&currentJson->json.object.values,
							"false",
							key
						);
						cursor += 4;
					}
					else
						SMAdd(
							&currentJson->json.object.values,
							json[cursor] == '\"' ?
							allocateString(json, &cursor) :
							allocateNumber(json, &cursor),
							key
						);

					if (SMGetString(&currentJson->json.object.values, key) == NULL)
					{
						free(json);
						free(typeStack);
						freeList(&this);
						free(key);
						return EMPTY_LIST;
					}
				}

				free(key);
				key = NULL;
			}
			else if (json[cursor] == '}')
			{
				if (currentJson->json.object.parent != NULL)
					currentJson = currentJson->json.object.parent;
				popType(&typeStack, &stackDepth);
			}
		}
		else // type == array
		{
			switch (json[cursor])
			{
			case '{':
			case '[':
			{
				Type type = json[cursor] == '{' ? OBJECT : ARRAY;
				currentJson = allocateJsonL(&currentJson->json, type);

				if (currentJson == NULL)
				{
					free(json);
					free(typeStack);
					freeList(&this);
					return EMPTY_LIST;
				}

				pushType(
					&typeStack,
					type,
					&stackDepth
				);
				break;
			}
			case '\"':
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
			case 't':
			case 'f':
				if (json[cursor] == 't')
				{
					SLPush(
						&currentJson->json.array.values,
						"true"
					);
					cursor += 3;
				}
				else if (json[cursor] == 'f')
				{
					SLPush(
						&currentJson->json.array.values,
						"false"
					);
					cursor += 4;
				}
				else
					SLPush(
						&currentJson->json.array.values,
						json[cursor] == '\"' ?
						allocateString(json, &cursor) :
						allocateNumber(json, &cursor)
					);

				if (SLGetString(
					&currentJson->json.array.values,
					currentJson->json.array.values.length - 1
				) == NULL)
				{
					free(json);
					free(typeStack);
					freeList(&this);
					return EMPTY_LIST;
				}
				break;
			case ']':
				currentJson = currentJson->json.array.parent;
				popType(&typeStack, &stackDepth);
			}
		}

		if (stackDepth == ROOT && json[cursor] == ',')
		{
			pushJsonL(&this, builder);

			builder = blankJson(OBJECT);
			currentJson = &builder;

			pushType(&typeStack, OBJECT, &stackDepth);
			cursor++;
		}
	}

	pushJsonL(&this, builder);
	free(json);
	free(typeStack);
	return this;
}

int JSONFree(JSONList* this) { return freeList(this); }

union JSON* JSONListGet(JSONList* this, int index) { return &getJsonL(this, index)->json; }

union JSON* JSONMapGet(JSONMap* this, const char* key) { return &getJsonM(this, key)->json; }

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

// list functions
static JSON* allocateJsonL(union JSON* this, Type type)
{
	if (this == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	List* list = type == OBJECT ?
		&this->array.objects : &this->array.arrays;

	pushJsonL(list, blankJson(type));

	JSON* child = getJsonL(list, list->length - 1);

	if (child->type == OBJECT)
		child->json.object.parent = this;
	else
		child->json.array.parent = this;

	return child;
}

static JSON* getJsonL(List* this, int index)
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

	ListNode* currentNode = this->first;

	for (int ctr = 0; ctr < index; ctr++)
		currentNode = currentNode->next;

	return &currentNode->value;
}

static int pushJsonL(List* this, JSON value)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	ListNode* builder = malloc(sizeof(ListNode));
	assert(builder);

	*builder = (ListNode)
	{
		value,
		NULL
	};

	if (this->first)
	{
		ListNode* node = this->first;
		while (node->next)
			node = node->next;

		node->next = builder;
	}
	else
		this->first = builder;

	this->length++;

	return SUCCESS;
}

// map functions
static JSON* allocateJsonM(union JSON* this, Type type, const char* key)
{
	if (this == NULL || key == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	Map* map = type == OBJECT ?
		&this->object.objects : &this->object.arrays;

	addJsonM(map, blankJson(type), key);

	JSON* child = getJsonM(map, key);

	if (child->type == OBJECT)
		child->json.object.parent = this;
	else
		child->json.array.parent = this;

	return child;
}

static JSON* getJsonM(Map* this, const char* key)
{
	if (this == NULL || key == NULL)
	{
		error(ERROR_PARAM_NULL);
		return NULL;
	}

	MapNode* node = this->first;

	while (node != NULL)
	{
		if (strcmp(node->key, key) == 0)
			return &node->value;

		node = node->next;
	}

	error(ERROR_KEY_NOT_FOUND);
	return NULL;
}

static int addJsonM(Map* this, JSON value, const char* key)
{
	if (this == NULL || key == NULL)
		return error(ERROR_PARAM_NULL);

	MapNode* builder = malloc(sizeof(MapNode));
	assert(builder);

	size_t size = strlen(key) + 1ull;

	*builder = (MapNode)
	{
		malloc(size),
		value,
		NULL
	};
	assert(builder->key);

	strcpy_s(builder->key, size, key);

	if (this->first != NULL)
	{
		MapNode* node = this->first;

		while (node->next != NULL)
		{
			if (strcmp(node->key, key) == 0)
			{
				free(builder->key);
				free(builder);
				return error(ERROR_KEY_EXISTS);
			}

			node = node->next;
		}

		node->next = builder;
	}
	else
		this->first = builder;

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

// memory cleanup
inline static int freeJson(JSON* this)
{
	return this->type == OBJECT ?
		freeObject(&this->json) : freeArray(&this->json);
}

static int freeObject(Object* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	freeMap(&this->objects);
	freeMap(&this->arrays);
	SMFreeMap(&this->values);
	this->parent = NULL;

	return SUCCESS;
}

static int freeArray(Array* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	freeList(&this->objects);
	freeList(&this->arrays);
	SLFreeList(&this->values);
	this->parent = NULL;

	return SUCCESS;
}

static int freeList(List* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	ListNode* node = this->first;

	while (node != NULL)
	{
		this->first = this->first->next;
		freeJson(&node->value);
		free(node);
		node = this->first;
	}

	this->length = 0;

	return SUCCESS;
}

static int freeMap(Map* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	MapNode* node = this->first;

	while (node != NULL)
	{
		this->first = this->first->next;
		free(node->key);
		freeJson(&node->value);
		free(node);
		node = this->first;
	}

	return SUCCESS;
}

// error handling
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
