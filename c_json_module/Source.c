#include "JSON.h"
#include <assert.h>
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


	people = (Person*)calloc(list.length, sizeof(Person));
	assert(people);

	JSONObject* currentObject;
	int length = list.length;
	for (int index = 0; index < length; index++)
	{
		currentObject = GetObject(&list, index);
		people[index].name = (char**)calloc(currentObject->objects[0].valueCount, sizeof(char*));
		assert(people[index].name);
		for (int name = 0; name < currentObject->objects[0].valueCount; name++)
		{
			people[index].name[name] = currentObject->objects[0].values[name];
		}
		people[index].dob.month = atoi(currentObject->objects[1].values[0]);
		people[index].dob.day = 20; // atoi(currentObject->objects[1].values[1]); // currentObject->objects[1].values[1] causes read access violation, see setValues for debuging
		people[index].dob.year = atoi(currentObject->objects[1].values[2]);
		people[index].male = currentObject->values[0][0] == 't' ? true : false;
	}

	for (int index = 0; index < length; index++)
	{
		printf("Person #%d:\n", index + 1);
		printf("%s %s\n%d-%d-%d\n%s\n",
			people[index].name[0], people[index].name[1],
			people[index].dob.year, people[index].dob.month, people[index].dob.day,
			(people[index].male ? "Male" : "Female"));
	}

	getchar();
}
