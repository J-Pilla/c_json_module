#include "JSON.h"

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Weapons
{
	char** rHand;
	int rHandCount;
	char** lHand;
	int lHandCount;
} Weapons;

typedef struct Armor
{
	char* helm;
	char* armor;
	char* gauntlets;
	char* leggings;
} Armor;

typedef struct Attributes
{
	int vigor;
	int attunement;
	int endurance;
	int vitality;
	int strength;
	int dexterity;
	int intelligence;
	int faith;
	int luck;
} Attributes;

typedef struct DSIIINPC
{
	int id;
	char* name;
	Weapons weapons;
	Armor armor;
	char** arrows;
	int arrowCount;
	char** bolts;
	int boltCount;
	char** spells;
	int spellCount;
	char** items;
	int itemCount;
	int level;
	Attributes attributes;
} DSIIINPC;

int main()
{
	JSONObjectList list = JSONParse("Dark Souls 3 NPCs.json");

	if (list.length == 0)
	{
		puts("ERROR: Failed to parse JSON");
		printf("press enter to exit . . . ");
		int key = getchar();
		fflush(stdin);
		return EXIT_FAILURE;
	}

	DSIIINPC* npcs = NULL;
	int length = list.length;
	npcs = (DSIIINPC*)calloc(length, sizeof(DSIIINPC));
	assert(npcs);

	for (int index = 0; index < length; index++)
	{
		DSIIINPC* npc = &npcs[index];

		// object pointers
		const JSONObject* currentObject = OLGetObject(&list, index);
		const JSONObject* weapons = OMGetObject(&currentObject->objects, "Weapons");
		const JSONObject* armorObject = OMGetObject(&currentObject->objects, "Armor");
		const JSONObject* attributes = OMGetObject(&currentObject->objects, "Attributes");

		// array pointers
		const JSONArray* rHand = AMGetArray(&weapons->arrays, "R-hand Weapons");
		const JSONArray* lHand = AMGetArray(&weapons->arrays, "L-hand Weapons");
		const JSONArray* arrows = AMGetArray(&currentObject->arrays, "Arrows");
		const JSONArray* bolts = AMGetArray(&currentObject->arrays, "Bolts");
		const JSONArray* spells = AMGetArray(&currentObject->arrays, "Spells");
		const JSONArray* items = AMGetArray(&currentObject->arrays, "Items");

		// string pointers
		const char* name = SMGetString(&currentObject->values, "Name");
		const char* helm = SMGetString(&armorObject->values, "Helm");
		const char* armor = SMGetString(&armorObject->values, "Armor");
		const char* gauntlets = SMGetString(&armorObject->values, "Gauntlets");
		const char* leggings = SMGetString(&armorObject->values, "Leggings");

		// integer assignment
		npc->id = atoi(SMGetString(&currentObject->values, "ID"));
		npc->level = atoi(SMGetString(&currentObject->values, "Name"));

		// array count assignment
		npc->weapons.rHandCount = rHand->values.length;
		npc->weapons.lHandCount = lHand->values.length;
		npc->arrowCount = arrows->values.length;
		npc->boltCount = bolts->values.length;
		npc->spellCount = spells->values.length;
		npc->itemCount = items->values.length;

		// array allocation
		npc->weapons.rHand = (char**)calloc(npc->weapons.rHandCount, sizeof(char*));
		assert(npc->weapons.rHand);

		npc->weapons.lHand = (char**)calloc(npc->weapons.lHandCount, sizeof(char*));
		assert(npc->weapons.lHand);

		npc->arrows = (char**)calloc(npc->arrowCount, sizeof(char*));
		assert(npc->arrows);

		npc->bolts = (char**)calloc(npc->boltCount, sizeof(char*));
		assert(npc->bolts);

		npc->spells = (char**)calloc(npc->spellCount, sizeof(char*));
		assert(npc->spells);

		npc->items = (char**)calloc(npc->itemCount, sizeof(char*));
		assert(npc->items);

		// attribute assignment
		npc->attributes.vigor = atoi(SMGetString(&attributes->values, "VIG"));
		npc->attributes.attunement = atoi(SMGetString(&attributes->values, "ATT"));
		npc->attributes.endurance = atoi(SMGetString(&attributes->values, "END"));
		npc->attributes.vitality = atoi(SMGetString(&attributes->values, "VIT"));
		npc->attributes.strength = atoi(SMGetString(&attributes->values, "STR"));
		npc->attributes.dexterity = atoi(SMGetString(&attributes->values, "SKL"));
		npc->attributes.intelligence = atoi(SMGetString(&attributes->values, "INT"));
		npc->attributes.faith = atoi(SMGetString(&attributes->values, "FTH"));
		npc->attributes.luck = atoi(SMGetString(&attributes->values, "LCK"));

		// string assignments
		// Name
		size_t size = strlen(name) + 1;
		npc->name = malloc(size);
		assert(npc->name);
		strcpy_s(npc->name, size, name);

		// Armor
		size = strlen(helm) + 1;
		npc->armor.helm = malloc(size);
		assert(npc->armor.helm);
		strcpy_s(npc->armor.helm, size, helm);

		size = strlen(armor) + 1;
		npc->armor.armor = malloc(size);
		assert(npc->armor.armor);
		strcpy_s(npc->armor.armor, size, armor);

		size = strlen(gauntlets) + 1;
		npc->armor.gauntlets = malloc(size);
		assert(npc->armor.gauntlets);
		strcpy_s(npc->armor.gauntlets, size, gauntlets);

		size = strlen(leggings) + 1;
		npc->armor.leggings = malloc(size);
		assert(npc->armor.leggings);
		strcpy_s(npc->armor.leggings, size, leggings);

		// R-hand Weapons
		for (int index = 0; index < npc->weapons.rHandCount; index++)
		{
			const char* weapon = SLGetString(&rHand->values, index);

			size = strlen(weapon) + 1;
			npc->weapons.rHand[index] = malloc(size);
			assert(npc->weapons.rHand[index]);
			strcpy_s(npc->weapons.rHand[index], size, weapon);
		}

		// L-hand Weapons
		for (int index = 0; index < npc->weapons.lHandCount; index++)
		{
			const char* weapon = SLGetString(&lHand->values, index);

			size = strlen(weapon) + 1;
			npc->weapons.lHand[index] = malloc(size);
			assert(npc->weapons.lHand[index]);
			strcpy_s(npc->weapons.lHand[index], size, weapon);
		}

		// Arrows
		for (int index = 0; index < npc->arrowCount; index++)
		{
			const char* arrow = SLGetString(&arrows->values, index);

			size_t size = strlen(arrow) + 1;
			npc->arrows[index] = malloc(size);
			assert(npc->arrows[index]);
			strcpy_s(npc->arrows[index], size, arrow);
		}

		// Bolts
		for (int index = 0; index < npc->boltCount; index++)
		{
			const char* bolt = SLGetString(&bolts->values, index);

			size_t size = strlen(bolt) + 1;
			npc->bolts[index] = malloc(size);
			assert(npc->bolts[index]);
			strcpy_s(npc->bolts[index], size, bolt);
		}

		// Spells
		for (int index = 0; index < npc->spellCount; index++)
		{
			const char* spell = SLGetString(&spells->values, index);

			size_t size = strlen(spell) + 1;
			npc->spells[index] = malloc(size);
			assert(npc->spells[index]);
			strcpy_s(npc->spells[index], size, spell);
		}

		// Items
		for (int index = 0; index < npc->itemCount; index++)
		{
			const char* item = SLGetString(&items->values, index);

			size_t size = strlen(item) + 1;
			npc->items[index] = malloc(size);
			assert(npc->items[index]);
			strcpy_s(npc->items[index], size, item);
		}
	}

	JSONFree(&list);

	for (int index = 0; index < length; index++)
	{
		DSIIINPC* npc = &npcs[index];

		printf("ID: %d\n%s\nRight Hand Weapons:\n", npc->id, npc->name);
		for (int index = 0; index < npc->weapons.rHandCount; index++)
		{
			printf("%s\n", npc->weapons.rHand[index]);
		}

		puts("Left Hand Weapons:");
		for (int index = 0; index < npc->weapons.lHandCount; index++)
		{
			printf("%s\n", npc->weapons.lHand[index]);
		}

		printf(
			"Armor:\n%s\n%s\n%s\n%s\n",
			npc->armor.helm,
			npc->armor.armor,
			npc->armor.gauntlets,
			npc->armor.leggings
		);

		puts("Arrows:");
		for (int index = 0; index < npc->arrowCount; index++)
		{
			printf("%s\n", npc->arrows[index]);
		}

		puts("Bolts:");
		for (int index = 0; index < npc->boltCount; index++)
		{
			printf("%s\n", npc->bolts[index]);
		}

		puts("Spells:");
		for (int index = 0; index < npc->spellCount; index++)
		{
			printf("%s\n", npc->spells[index]);
		}

		puts("Items:");
		for (int index = 0; index < npc->itemCount; index++)
		{
			printf("%s\n", npc->items[index]);
		}

		puts("Attributes:");
		printf("Vigor: %d\n", npc->attributes.vigor);
		printf("Attunement: %d\n", npc->attributes.attunement);
		printf("Endurance: %d\n", npc->attributes.endurance);
		printf("Vitality: %d\n", npc->attributes.vitality);
		printf("Strength: %d\n", npc->attributes.strength);
		printf("Dexterity: %d\n", npc->attributes.dexterity);
		printf("Intelligence: %d\n", npc->attributes.intelligence);
		printf("Faith: %d\n", npc->attributes.faith);
		printf("Luck: %d\n\n", npc->attributes.luck);
	}
	
	puts("program ended successfully");
	printf("press enter to exit . . . ");
	int key = getchar();
	fflush(stdin);
	return EXIT_SUCCESS;
}
