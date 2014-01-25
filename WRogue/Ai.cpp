#include <stdio.h>
#include <math.h>
#include "main.hpp"

// how many turns the monster chases the player
// after losing his sight
static const int TRACKING_TURNS=5;

MonsterAi::MonsterAi() : moveCount(0) {
}

void MonsterAi::update(Actor *owner) {
    if ( owner->destructible && owner->destructible->isDead() ) {
    	return;
    }
	if ( engine.map->isInFov(owner->x,owner->y) ) {
    	// we can see the player. move towards him
    	moveCount=TRACKING_TURNS;
    } else {
    	moveCount--;
    }
   	if ( moveCount > 0 ) {
   		moveOrAttack(owner, engine.player->x,engine.player->y);
   	}
}

void MonsterAi::moveOrAttack(Actor *owner, int targetx, int targety) {
	int dx = targetx - owner->x;
	int dy = targety - owner->y;
	int stepdx = (dx > 0 ? 1:-1);
	int stepdy = (dy > 0 ? 1:-1);
	float distance=sqrtf(dx*dx+dy*dy);
	if ( distance >= 2 ) {
		dx = (int)(round(dx/distance));
		dy = (int)(round(dy/distance));
		if ( engine.map->canWalk(owner->x+dx,owner->y+dy) ) {
			owner->x += dx;
			owner->y += dy;
		} else if ( engine.map->canWalk(owner->x+stepdx,owner->y) ) {
			owner->x += stepdx;
		} else if ( engine.map->canWalk(owner->x,owner->y+stepdy) ) {
			owner->y += stepdy;
		}
	} else if ( owner->attacker ) {
		owner->attacker->attack(owner,engine.player);
	}
}

void PlayerAi::update(Actor *owner) {
    int levelUpXp = getNextLevelXp();
    if ( owner ->destructible-> xp >= levelUpXp){
        xpLevel++;
        owner->destructible->xp -= levelUpXp;
        engine.gui->message(TCODColor::lightestYellow,"You have reached level %d", xpLevel );
    }
    if ( owner->destructible && owner->destructible->isDead() ) {
    	return;
    }
	int dx=0,dy=0;
	switch(engine.lastKey.vk) {
		case TCODK_UP : dy=-1; break;
		case TCODK_DOWN : dy=1; break;
		case TCODK_LEFT : dx=-1; break;
		case TCODK_RIGHT : dx=1; break;
		case TCODK_CHAR : handleActionKey(owner, engine.lastKey.c); break;
        default:break;
    }
    if (dx != 0 || dy != 0) {
    	engine.gameStatus=Engine::NEW_TURN;
    	if (moveOrAttack(owner, owner->x+dx,owner->y+dy)) {
    		engine.map->computeFov();
    	}
	}
}

bool PlayerAi::moveOrAttack(Actor *owner, int targetx,int targety) {
	if ( engine.map->isWall(targetx,targety) ) return false;
	// look for living actors to attack
	for (Actor **iterator=engine.actors.begin();
		iterator != engine.actors.end(); iterator++) {
		Actor *actor=*iterator;
		if ( actor->destructible && !actor->destructible->isDead()
			 && actor->x == targetx && actor->y == targety ) {
			owner->attacker->attack(owner, actor);
			return false;
		}
	}
	// look for corpses
	for (Actor **iterator=engine.actors.begin();
		iterator != engine.actors.end(); iterator++) {
		Actor *actor=*iterator;
		bool corpseOrItem=(actor->destructible && actor->destructible->isDead())
			|| actor->pickable;
		if ( corpseOrItem
			 && actor->x == targetx && actor->y == targety ) {
			engine.gui->message(TCODColor::lightGrey,"There's a %s here.",actor->name);
		}
	}
	owner->x=targetx;
	owner->y=targety;
	return true;
}

void PlayerAi::handleActionKey(Actor *owner, int ascii) {
	switch(ascii) {
		case 'p' : // pickup item
		{
			bool found=false;
			for (Actor **iterator=engine.actors.begin();
				iterator != engine.actors.end(); iterator++) {
				Actor *actor=*iterator;
				if ( actor->pickable && actor->x == owner->x && actor->y == owner->y ) {
					if (actor->pickable->pick(actor,owner)) {
						found=true;
						engine.gui->message(TCODColor::lightCyan,"You pickup the %s.",
							actor->name);
						break;
					} else if (! found) {
						found=true;
						engine.gui->message(TCODColor::red,"Your inventory is full.");
					}
				}
			}
			if (!found) {
				engine.gui->message(TCODColor::lightGrey,"You grab the air.");
			}
			engine.gameStatus=Engine::NEW_TURN;
		}
		break;

		case 'i' : // display inventory
		{
			Actor *actor=choseFromInventory(owner);
			if ( actor ) {
				actor->pickable->use(actor,owner);
				engine.gameStatus=Engine::NEW_TURN;
			}
		}
		break;

		case 'd' :
            if ( engine.stairs->x == owner->x && engine.stairs->y == owner->y ) {
            engine.nextLevel();
            } else {
            engine.gui->message(TCODColor::lightGrey,"There are no stairs here.");
            }
            break;
	}
}

Actor *PlayerAi::choseFromInventory(Actor *owner) {
	static const int INVENTORY_WIDTH=50;
	static const int INVENTORY_HEIGHT=25;
	static TCODConsole con(INVENTORY_WIDTH,INVENTORY_HEIGHT);

	// display the inventory frame
	con.setDefaultForeground(TCODColor::lighterSea);
	con.printFrame(0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,true,
		TCOD_BKGND_COLOR_BURN,"Bag");

	// display the items with their keyboard shortcut
	con.setDefaultForeground(TCODColor::white);
	int shortcut='a';
	int y=1;
	for (Actor **it=owner->container->inventory.begin();
		it != owner->container->inventory.end(); it++) {
		Actor *actor=*it;
		con.print(2,y,"(%c) %s", shortcut, actor->name);
		y++;
		shortcut++;
	}

	// blit the inventory console on the root console
	TCODConsole::blit(&con, 0,0,INVENTORY_WIDTH,INVENTORY_HEIGHT,
		TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2,
		engine.screenHeight/2-INVENTORY_HEIGHT/2,1.0f,0.5f);
	TCODConsole::flush();

	// wait for a key press
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS,&key,NULL,true);
	if ( key.vk == TCODK_CHAR ) {
		int actorIndex=key.c - 'a';
		if ( actorIndex >= 0 && actorIndex < owner->container->inventory.size() ) {
			return owner->container->inventory.get(actorIndex);
		}
	}
	return NULL;
}

PlayerAi::PlayerAi() : xpLevel(1) {
}

const int LEVEL_UP_BASE = 200;
const int LEVEL_UP_FACTOR = 150;

int PlayerAi::getNextLevelXp(){
    return LEVEL_UP_BASE + xpLevel * LEVEL_UP_FACTOR;
}
