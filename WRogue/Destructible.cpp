#include <stdio.h>
#include "main.hpp"

Destructible::Destructible(float maxHp, float maxMana, float maxFood, float defense, const char *corpseName, int xp) :
	maxHp(maxHp),hp(maxHp), maxMana(maxMana), mana(maxMana), maxFood(maxFood), food(maxFood), defense(defense), xp(xp) {
	    this->corpseName = strdup(corpseName);
}

Destructible::~Destructible() {
    //free(corpseName);
}

float Destructible::heal(float amount){
    hp += amount;
    if (hp >= maxHp){
        amount -= hp-maxHp;
        hp = maxHp;
    }
    return amount;
}

float Destructible::takeDamage(Actor *owner, float damage) {
	damage -= defense;
	if ( damage > 0 ) {
		hp -= damage;
		if ( hp <= 0 ) {
			die(owner);
		}
	} else {
		damage=0;
	}
	return damage;
}

void Destructible::die(Actor *owner) {
	// transform the actor into a corpse!
	owner->ch='%';
	owner->col=TCODColor::darkRed;
	owner->name=corpseName;
	owner->blocks=false;
	// make sure corpses are drawn before living actors
	engine.sendToBack(owner);
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char *corpseName, int xp) :
	Destructible(maxHp,maxMana,maxFood,defense,corpseName,xp) {
}

void MonsterDestructible::die(Actor *owner) {
	// transform it into a nasty corpse! it doesn't block, can't be
	// attacked and doesn't move
	engine.gui->message(TCODColor::lightGrey,"%s is dead. You gain %d XP",owner->name,xp);
	engine.player->destructible->xp += xp;
	Destructible::die(owner);
}

PlayerDestructible::PlayerDestructible(float maxHp, float maxMana, float maxFood, float defense, const char *corpseName) :
	Destructible(maxHp,maxMana,maxFood,defense,corpseName,xp) {
}

void PlayerDestructible::die(Actor *owner) {
	engine.gui->message(TCODColor::red,"You died!");
	Destructible::die(owner);
	engine.gameStatus=Engine::DEFEAT;
}
