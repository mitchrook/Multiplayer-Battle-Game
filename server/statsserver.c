#include "statsserver.h"
#include <stdlib.h>

#define NUM_BASE_STATS 4
#define TOTAL_STATS 100
#define MAX_STATS 100

void (*attackFunctions[NUM_OF_ATTACKS])(Stats* attacker, Stats* defender) = {
	attackStrike,
	attackFireball,
	attackHeal,
};

void generateStats(Stats* stats) {
	int base[NUM_BASE_STATS];
	int sum = 0;
	int i;
	for(i = 0; i < NUM_BASE_STATS; i++) {
		base[i] = rand() % MAX_STATS;
		sum += base[i];
	}
	for(i = 0; i < NUM_BASE_STATS; i++) {
		base[i] = base[i] * TOTAL_STATS / sum;
	}

	stats->hp = base[0] * 2 + 20;
	stats->atk = base[1] + 5;
	stats->def = base[2];
	stats->magic = base[3];
	stats->mana = base[3] * 2;
}

void attackStrike(Stats* attacker, Stats* defender) {
	int damage = attacker->atk - (defender->def / 3);
	if(damage < 1) {
		damage = 1;
	}

	defender->hp -= damage;

	if(defender->hp < 0) {
		defender->hp = 0;
	}
}

void attackFireball(Stats* attacker, Stats* defender) {
	int damage = 1;
	if(attacker->mana >= FIREBALL_COST) {
		damage = attacker->magic - (defender->def / 8) - (defender->magic / 8);
	}
	if(damage < 1) {
		damage = 1;
	}

	defender->hp -= damage;
	attacker->mana -= FIREBALL_COST;

	if(defender->hp < 0) {
		defender->hp = 0;
	}
	if(attacker->mana < 0) {
		attacker->mana = 0;
	}
}

void attackHeal(Stats* attacker, Stats* defender) {
	int heal = 1;
	if(attacker->mana >= HEAL_COST) {
		heal = (attacker->magic) + (attacker->def / 2);
	}

	attacker->hp += heal;
	attacker->mana -= HEAL_COST;

	if(attacker->mana < 0) {
		attacker->mana = 0;
	}
}
