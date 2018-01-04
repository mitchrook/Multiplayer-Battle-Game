#ifndef STATS_H
#define STATS_H

typedef struct {
	int hp, atk, def, magic, mana;
} Stats;

enum {
	ATK_STRIKE = 0,
	ATK_FIREBALL,
	ATK_HEAL,
	NUM_OF_ATTACKS,
};

#define FIREBALL_COST 20
#define HEAL_COST 10

static inline int isValidAttackIndex(int idx) {
	return (idx >= 0 && idx < NUM_OF_ATTACKS) ? 1 : 0;
}

#endif
