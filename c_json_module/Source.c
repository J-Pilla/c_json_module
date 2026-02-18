#include "JSON.h"

#include <assert.h>
#include <conio.h>
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
	ObjectList list = ParseJSON("Dark Souls 3 NPCs.json");

	if (list.length == 0)
	{
		puts("ERROR: Failed to parse JSON");
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
		const JSONObject* currentObject = GetObject(&list, index);
		const JSONObject* weaponsObject = &currentObject->objects[0];
		const JSONObject* rHand = &weaponsObject->objects[0];
		const JSONObject* lHand = &weaponsObject->objects[1];
		const JSONObject* armorObject = &currentObject->objects[1];
		const JSONObject* arrows = &currentObject->objects[2];
		const JSONObject* bolts = &currentObject->objects[3];
		const JSONObject* spells = &currentObject->objects[4];
		const JSONObject* items = &currentObject->objects[5];
		const JSONObject* attributes = &currentObject->objects[6];

		// string pointers
		const char* name = SLGetter(currentObject->values, 1);
		const char* helm = SLGetter(armorObject->values, 0);
		const char* armor = SLGetter(armorObject->values, 1);
		const char* gauntlets = SLGetter(armorObject->values, 2);
		const char* leggings = SLGetter(armorObject->values, 3);

		// integer assignment
		npc->id = atoi(SLGetter(currentObject->values, 0));
		npc->level = atoi(SLGetter(currentObject->values, 2));

		// count assignment
		npc->weapons.rHandCount = rHand->values->length;
		npc->weapons.lHandCount = lHand->values->length;
		npc->arrowCount = arrows->values->length;
		npc->boltCount = bolts->values->length;
		npc->spellCount = spells->values->length;
		npc->itemCount = items->values->length;

		// attribute assignment
		npc->attributes.vigor = atoi(SLGetter(attributes->values, 0));
		npc->attributes.attunement = atoi(SLGetter(attributes->values, 1));
		npc->attributes.endurance = atoi(SLGetter(attributes->values, 2));
		npc->attributes.vitality = atoi(SLGetter(attributes->values, 3));
		npc->attributes.strength = atoi(SLGetter(attributes->values, 4));
		npc->attributes.dexterity = atoi(SLGetter(attributes->values, 5));
		npc->attributes.intelligence = atoi(SLGetter(attributes->values, 6));
		npc->attributes.faith = atoi(SLGetter(attributes->values, 7));
		npc->attributes.luck = atoi(SLGetter(attributes->values, 8));

		// malloc
		npc->name = (char*)calloc(strlen(name), sizeof(char));
		assert(npc->name);

		npc->weapons.rHand = (char**)calloc(npc->weapons.rHandCount, sizeof(char*));
		assert(npc->weapons.rHand);

		npc->weapons.lHand = (char**)calloc(npc->weapons.lHandCount, sizeof(char*));
		assert(npc->weapons.lHand);

		npc->armor.helm = (char*)calloc(strlen(helm), sizeof(char));
		assert(npc->armor.helm);

		npc->armor.armor = (char*)calloc(strlen(armor), sizeof(char));
		assert(npc->armor.armor);

		npc->armor.gauntlets = (char*)calloc(strlen(gauntlets), sizeof(char));
		assert(npc->armor.gauntlets);

		npc->armor.leggings = (char*)calloc(strlen(leggings), sizeof(char));
		assert(npc->armor.leggings);

		npc->arrows = (char**)calloc(npc->arrowCount, sizeof(char*));
		assert(npc->arrows);

		npc->bolts = (char**)calloc(npc->boltCount, sizeof(char*));
		assert(npc->bolts);

		npc->spells = (char**)calloc(npc->spellCount, sizeof(char*));
		assert(npc->spells);

		npc->items = (char**)calloc(npc->itemCount, sizeof(char*));
		assert(npc->items);

		// string assignments
		// name
		for (int letter = 0; letter <= strlen(name); letter++)
		{
			npc->name[letter] = name[letter];
		}

		// rHandWeapons
		for (int weaponIndex = 0; weaponIndex < npc->weapons.rHandCount; weaponIndex++)
		{
			const char* weapon = SLGetter(rHand->values, weaponIndex);
			npc->weapons.rHand[weaponIndex] = (char*)calloc(strlen(weapon), sizeof(char));
			assert(npc->weapons.rHand[weaponIndex]);
			for (int letter = 0; letter <= strlen(weapon); letter++)
			{
				npc->weapons.rHand[weaponIndex][letter] = weapon[letter];
			}
		}

		// lHandWeapons
		for (int weaponIndex = 0; weaponIndex < npc->weapons.lHandCount; weaponIndex++)
		{
			const char* weapon = SLGetter(lHand->values, weaponIndex);
			npc->weapons.lHand[weaponIndex] = (char*)calloc(strlen(weapon), sizeof(char));
			assert(npc->weapons.lHand[weaponIndex]);
			for (int letter = 0; letter <= strlen(weapon); letter++)
			{
				npc->weapons.lHand[weaponIndex][letter] = weapon[letter];
			}
		}

		// armor
		for (int letter = 0; letter <= strlen(helm); letter++)
		{
			npc->armor.helm[letter] = helm[letter];
		}
		for (int letter = 0; letter <= strlen(armor); letter++)
		{
			npc->armor.armor[letter] = armor[letter];
		}
		for (int letter = 0; letter <= strlen(gauntlets); letter++)
		{
			npc->armor.gauntlets[letter] = gauntlets[letter];
		}
		for (int letter = 0; letter <= strlen(leggings); letter++)
		{
			npc->armor.leggings[letter] = leggings[letter];
		}

		// arrows
		for (int arrowIndex = 0; arrowIndex < npc->arrowCount; arrowIndex++)
		{
			const char* arrow = SLGetter(arrows->values, arrowIndex);
			npc->arrows[arrowIndex] = (char*)calloc(strlen(arrow), sizeof(char));
			assert(npc->arrows[arrowIndex]);
			for (int letter = 0; letter <= strlen(arrow); letter++)
			{
				npc->arrows[arrowIndex][letter] = arrow[letter];
			}
		}

		// bolts
		for (int boltIndex = 0; boltIndex < npc->boltCount; boltIndex++)
		{
			const char* bolt = SLGetter(bolts->values, boltIndex);
			npc->bolts[boltIndex] = (char*)calloc(strlen(bolt), sizeof(char));
			assert(npc->bolts[boltIndex]);
			for (int letter = 0; letter <= strlen(bolt); letter++)
			{
				npc->bolts[boltIndex][letter] = bolt[letter];
			}
		}

		// spells
		for (int spellIndex = 0; spellIndex < npc->spellCount; spellIndex++)
		{
			const char* spell = SLGetter(spells->values, spellIndex);
			npc->spells[spellIndex] = (char*)calloc(strlen(spell), sizeof(char));
			assert(npc->spells[spellIndex]);
			for (int letter = 0; letter <= strlen(spell); letter++)
			{
				npc->spells[spellIndex][letter] = spell[letter];
			}
		}

		// items
		for (int itemIndex = 0; itemIndex < npc->itemCount; itemIndex++)
		{
			const char* item = SLGetter(items->values, itemIndex);
			npc->items[itemIndex] = (char*)calloc(strlen(item), sizeof(char));
			assert(npc->items[itemIndex]);
			for (int letter = 0; letter <= strlen(item); letter++)
			{
				npc->items[itemIndex][letter] = item[letter];
			}
		}
	}

	FreeObjectList(&list);

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

	printf("program ended successfully\npress any key to continue . . . ");
	{ int key = _getch(); }
	return EXIT_SUCCESS;
}
