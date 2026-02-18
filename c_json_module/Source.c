#include "JSON.h"

#include <assert.h>
#include <conio.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Date
{
	int month;
	int day;
	int year;
} Date;

typedef struct Person
{
	char** name;
	int nameCount;
	Date dob;
	bool male;
} Person;

int main()
{
	Person* people = NULL;
	ObjectList list = ParseJSON("example.json");
	
	if (list.length == 0)
	{
		puts("ERROR: Failed to parse JSON");
		return 1;
	}

	int length = list.length;
	people = (Person*)calloc(length, sizeof(Person));
	assert(people);

	JSONObject* currentObject;
	for (int index = 0; index < length; index++)
	{
		currentObject = GetObject(&list, index);
		people[index].nameCount = currentObject->objects[0].values->length;
		people[index].name = (char**)calloc(people[index].nameCount, sizeof(char*));
		assert(people[index].name);
		for (int name = 0; name < people[index].nameCount; name++)
		{
			people[index].name[name] = (char*)calloc(sizeof(SLGetter(currentObject->objects[0].values, name)), sizeof(char));
			assert(people[index].name[name]);
			for (int letter = 0; letter <= strlen(SLGetter(currentObject->objects[0].values, name)); letter++)
			{
				people[index].name[name][letter] = SLGetter(currentObject->objects[0].values, name)[letter];
			}
		}
		people[index].dob.month = atoi(SLGetter(currentObject->objects[1].values, 0));
		people[index].dob.day = atoi(SLGetter(currentObject->objects[1].values, 1));
		people[index].dob.year = atoi(SLGetter(currentObject->objects[1].values, 2));
		people[index].male = SLGetter(currentObject->values, 0)[0] == 't' ? true : false;
	}

	FreeObjectList(&list);

	for (int index = 0; index < length; index++)
	{
		printf("Person #%d:\n", index + 1);
		for (int name = 0; name < people[index].nameCount; name++)
		{
			if (name > 0)
				printf(" ");
			printf("%s", people[index].name[name]);
		}
		printf("\n%d-%d-%d\n%s\n",
			people[index].dob.year, people[index].dob.month, people[index].dob.day,
			(people[index].male ? "Male" : "Female"));
	}

	printf("program ended successfully\npress any key to continue . . . ");
	_getch();
	return EXIT_SUCCESS;
}
