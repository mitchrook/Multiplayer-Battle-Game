#ifndef STATS_SERVER_H
#define STATS_SERVER_H

#include "../common/stats.h"

extern void (*attackFunctions[NUM_OF_ATTACKS])(Stats* attacker, Stats* defender);

void generateStats(Stats* stats);

void attackStrike(Stats* attacker, Stats* defender);
void attackFireball(Stats* attacker, Stats* defender);
void attackHeal(Stats* attacker, Stats* defender);

#endif
