#include "StringMap.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#define SUCCESS 0
#define FAILURE 1

#define ERROR_PARAM_NULL "Error: NULL pointer passed"
#define ERROR_KEY_NOT_FOUND "Error: key not found in this"
#define ERROR_KEY_EXISTS "Error: key already exists in this"
#define ERROR_MAP_EMPTY "Error: this is empty"

typedef struct SMNode
{
	char* key;
	char* value;
	struct SMNode* link;
} SMNode;

static int error(char*);
inline static void pause();

int SMFreeMap(StringMap* this)
{
	if (this == NULL)
		return error(ERROR_PARAM_NULL);

	SMNode* node = this->firstNode;

	while (node != NULL)
	{
		this->firstNode = this->firstNode->link;
		free(node->key);
		free(node->value);
		free(node);
		node = this->firstNode;
	}

	return SUCCESS;
}

const char* SMGetString(StringMap* this, const char* key)
{
	if (this == NULL || key == NULL)
	{
		error(ERROR_PARAM_NULL);
		return "";
	}

	SMNode* node = this->firstNode;

	while (node != NULL)
	{
		if (node->key == key)
			return node->value;
	}

	error(ERROR_KEY_NOT_FOUND);
	return "";
}

int SMSetString(StringMap* this, const char* value, const char* key)
{
	if (this == NULL || value == NULL || key == NULL)
		return error(ERROR_PARAM_NULL);

	SMNode* node = this->firstNode;

	while (node != NULL)
	{
		if (node->key == key)
			break;

		node = node->link;
	}

	if (node == NULL)
		return error(ERROR_KEY_NOT_FOUND);

	free(node->value);

	node->value = calloc(strlen(value) + 1, sizeof(char));
	assert(node->value);

	for (int index = 0; index <= strlen(value); index++)
		node->value[index] = value[index];

	return SUCCESS;
}

int SMAdd(StringMap* this, const char* value, const char* key)
{
	if (this == NULL || value == NULL || key == NULL)
		return error(ERROR_PARAM_NULL);

	SMNode* builder = malloc(sizeof(SMNode));
	assert(builder);

	*builder = (SMNode)
	{
		calloc(strlen(key) + 1, sizeof(char)),
		calloc(strlen(value) + 1, sizeof(char)),
		NULL
	};
	assert(builder->key);
	assert(builder->value);

	if (this->firstNode != NULL)
	{
		SMNode* node = this->firstNode;
		
		while (node->link != NULL)
		{
			if (node->key == key)
			{
				free(builder->value);
				free(builder);
				return error(ERROR_KEY_EXISTS);
			}

			node = node->link;
		}

		node->link = builder;
	}
	else
		this->firstNode = builder;

	return SUCCESS;
}

int SMRemove(StringMap* this, const char* key)
{
	if (this == NULL || key == NULL)
		return error(ERROR_PARAM_NULL);

	if (this->firstNode == NULL)
		return error(ERROR_MAP_EMPTY);

	SMNode* currentNode = this->firstNode, * prevNode = NULL;

	while (currentNode->link != NULL)
	{
		if (currentNode->key == key)
			break;

		prevNode = currentNode;
		currentNode = currentNode->link;
	}

	if (currentNode == NULL)
		return error(ERROR_KEY_NOT_FOUND);

	if (currentNode == this->firstNode)
		this->firstNode = currentNode->link;
	else
		prevNode->link = currentNode->link;

	free(currentNode->key);
	free(currentNode->value);
	free(currentNode);

	return SUCCESS;
}

static int error(char* error)
{
	if (error == NULL)
		error = "Error";

	puts(error);
	pause();
	return FAILURE;
}

inline static void pause()
{
	printf("press enter to continue . . . ");
	int key = getchar();
	fflush(stdin);
}
