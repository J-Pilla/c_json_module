#include "StringList.h"

#include <assert.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct SLNode
{
	char* value;
	SLNode* link;
};

static int errorOutOfBounds();

StringList* SLConstructor()
{
	StringList* list = (StringList*)malloc(sizeof(list));
	assert(list);
	list->firstNode = NULL;
	list->length = 0;
	return list;
}

void SLDestructor(StringList* list)
{
	SLNode* node = list->firstNode;

	while (node)
	{
		list->firstNode = list->firstNode->link;
		free(node);
		node = list->firstNode;
	}

	list->length = 0;
}

const char* SLGetter(StringList* list, int index)
{
	if (index < 0 || index >= list->length)
	{
		errorOutOfBounds();
		return NULL;
	}

	SLNode* node = list->firstNode;

	for (int ctr = 0; ctr < index; ctr++)
		node = node->link;

	return node->value;
}

int SLSetter(StringList* list, char* value, int index)
{
	if (index < 0 || index >= list->length)
		return errorOutOfBounds();

	SLNode* node = list->firstNode;

	for (int ctr = 0; ctr < index; ctr++)
		node = node->link;

	if (node->value)
		free(node->value);

	node->value = (char*)malloc(strlen(value) + 1);

	for (int index = 0; index <= strlen(value); index++)
		node->value[index] = value[index];

	return EXIT_SUCCESS;
}

void SLPush(StringList* list, char* value)
{
	SLNode* builder = malloc(sizeof(SLNode));
	assert(builder);

	builder->value = value;
	builder->link = NULL;

	if (list->firstNode)
	{
		SLNode* node = list->firstNode;
		while (node->link)
			node = node->link;

		node->link = builder;
	}
	else
		list->firstNode = builder;

	list->length++;
}

int SLPop(StringList* list)
{
	if (!list->firstNode)
		return EXIT_FAILURE;

	SLNode* currentNode = list->firstNode, * prevNode = NULL;

	while (currentNode->link)
	{
		prevNode = currentNode;
		currentNode = currentNode->link;
	}

	if (currentNode == list->firstNode)
		list->firstNode = NULL;
	else
		prevNode->link = NULL;

	free(currentNode);
	list->length--;

	return EXIT_SUCCESS;
}

static int errorOutOfBounds()
{
	printf("\nError: out of bounds of the list\npress any key to continue . . . ");
	_getch();
	return EXIT_FAILURE;
}