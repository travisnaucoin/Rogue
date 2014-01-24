#include "main.hpp"

bool Pickable::pick(Actor *owner, Actor *wearer) {
	if ( wearer->container && wearer->container->add(owner) ) {
		engine.actors.remove(owner);
		return true;
	}
	return false;
}

bool Pickable::use(Actor *owner, Actor *wearer) {
	if ( wearer->container ) {
		wearer->container->remove(owner);
		delete owner;
		return true;
	}
	return false;
}

Healer::Healer(float amount) : amount(amount) {
}

bool Healer::use(Actor *owner, Actor *wearer) {
	if ( wearer->destructible ) {
		float amountHealed = wearer->destructible->heal(amount);
		if ( amountHealed > 0 ) {
			return Pickable::use(owner,wearer);
		}
	}
	return false;
}

LightningBolt::LightningBolt(float range, float damage, float cost)
    : range(range),damage(damage),cost(cost) {
}

bool LightningBolt::use(Actor *owner, Actor *wearer) {
    Actor *closestMonster=engine.getClosestMonster(wearer->x,wearer->y,range);
    if (! closestMonster ) {
        engine.gui->message(TCODColor::lightGrey,"No enemy is close enough to strike.");
        return false;
    }
    else if (wearer->destructible->mana < cost){
        engine.gui->message(TCODColor::lightOrange,"Not Enough Mana");
        return false;
    }
    // hit closest monster for <damage> hit points
    wearer->destructible->mana -= cost;
    //engine.se->addSpell(TCODColor::yellow,TCODColor::red,'*',5.0f,closestMonster->x,closestMonster->y);
    //engine.se->render();
    effect(closestMonster->x,closestMonster->y,closestMonster);
    engine.gui->message(TCODColor::lightBlue,
    "A lighting bolt strikes the %s \n"
    "The damage is %g hit points. \n"
    "You spent %g Mana \n",
    closestMonster->name,damage,cost);
    closestMonster->destructible->takeDamage(closestMonster,damage);
    return Pickable::use(owner,wearer);
}

void LightningBolt::effect(int x, int y, Actor *a){
    TCODColor first = TCODColor::darkYellow;
    TCODColor second = TCODColor::lightestYellow;
    TCODRandom *rng = TCODRandom::getInstance();
    engine.render();
    int l;
    float timedur = 0;
    while (timedur < 5.0f){
        l = rng->get(1,10);
        //l = CLAMP(0.0f, 1.0f, l);
        TCODSystem::setFps(20);
        TCODConsole::root->putCharEx(x,y,'+',TCODColor::red,TCODColor::yellow);
        TCODConsole::root->putCharEx(x+timedur,y,'-',TCODColor::white,TCODColor::yellow);
        TCODConsole::root->putCharEx(x-timedur,y,'-',TCODColor::white,TCODColor::yellow);
        TCODConsole::root->putCharEx(x,y-timedur,'|',TCODColor::white,TCODColor::yellow);
        TCODConsole::root->putCharEx(x,y+timedur,'|',TCODColor::white,TCODColor::yellow);
        TCODConsole::blit(TCODConsole::root,0,0,engine.screenWidth,engine.screenHeight,TCODConsole::root,0,0);
        engine.gui->render();
        engine.update();
        TCODConsole::flush();
        timedur+= 1.0f;
        }

        TCODConsole::flush();
        TCODSystem::setFps(70);
        engine.render();
}

FireBolt::FireBolt(float range, float damage,float cost) : LightningBolt(range,damage,cost) {
}

bool FireBolt::use(Actor *owner, Actor *wearer) {
    engine.gui->message(TCODColor::cyan, "Left-click a target tile for the fireball,\nor right-click to cancel.");
    int x,y;

    if (! engine.pickATile(&x,&y,0,range)) {
        return false;
    }
    //engine.spellAOE(&x,&y,100.0f,range);
    // burn everything in <range> (including player)
    effect(x,y);
engine.gui->message(TCODColor::orange,"The fireball explodes, burning everything within %g tiles!",range);
    for (Actor **iterator=engine.actors.begin();
        iterator != engine.actors.end(); iterator++) {
        Actor *actor=*iterator;
        if ( actor->destructible && !actor->destructible->isDead()
            && actor->getDistance(x,y) <= range) {
                engine.gui->message(TCODColor::orange,"The %s gets burned for %g hit points.",
                actor->name,damage);
                actor->destructible->takeDamage(actor,damage);
        }
    }
    return Pickable::use(owner,wearer);
}

void FireBolt::effect(int x, int y){
    TCODColor first = TCODColor::darkYellow;
    TCODColor second = TCODColor::lightestYellow;
    TCODRandom *rng = TCODRandom::getInstance();
    engine.render();
    int l;
    float timedur = 0;
    while (timedur < 5.0f){
        l = rng->get(1,10);
        //l = CLAMP(0.0f, 1.0f, l);
        TCODSystem::setFps(20);
        TCODConsole::root->putCharEx(x,y,'+',TCODColor::red,TCODColor::lightestRed);
        TCODConsole::root->putCharEx(x+timedur,y,'+',TCODColor::red,TCODColor::lightestRed);
        TCODConsole::root->putCharEx(x-timedur,y,'+',TCODColor::red,TCODColor::lightestRed);
        TCODConsole::root->putCharEx(x,y+timedur,'+',TCODColor::red,TCODColor::lightestRed);
        TCODConsole::root->putCharEx(x,y-timedur,'+',TCODColor::red,TCODColor::lightestRed);
        TCODConsole::root->putCharEx(x-timedur,y-timedur,'+',TCODColor::red,TCODColor::lightestRed);
        TCODConsole::root->putCharEx(x+timedur,y+timedur,'+',TCODColor::red,TCODColor::lightestRed);
        TCODConsole::root->putCharEx(x-timedur,y+timedur,'+',TCODColor::red,TCODColor::lightestRed);
        TCODConsole::root->putCharEx(x+timedur,y-timedur,'+',TCODColor::red,TCODColor::lightestRed);

        TCODConsole::blit(TCODConsole::root,0,0,engine.screenWidth,engine.screenHeight,TCODConsole::root,0,0);
        engine.gui->render();
        engine.update();
        TCODConsole::flush();
        timedur+= 1.0f;
        }

        TCODConsole::flush();
        TCODSystem::setFps(70);
        engine.render();
}
