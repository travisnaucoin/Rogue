#include "main.hpp"

Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP),
	player(NULL),map(NULL),fovRadius(10),
	screenWidth(screenWidth),screenHeight(screenHeight), level(1) {
    TCODSystem::setFps(70);
    TCODConsole::setCustomFont("terminal2.png",TCOD_FONT_LAYOUT_ASCII_INROW);

    gui = new Gui();
}

void Engine::init() {
    TCODConsole::initRoot(screenWidth,screenHeight,"0xD153A53",false);
    TCODConsole::root->setDefaultBackground(TCODColor::black);
    player = new Actor(40,25,'@',"player",TCODColor::white);
    player->destructible=new PlayerDestructible(30,30,30,2,"your cadaver");
    player->attacker=new Attacker(5);
    player->ai = new PlayerAi();
    player->container = new Container(26);
    actors.push(player);
    stairs = new Actor(0,0,'>',"stairs",TCODColor::white);
    stairs->blocks=false;
    stairs->fovOnly=false;
    actors.push(stairs);
    map = new Map(80,43);
    map->init(true);
    gui->message(TCODColor::lightestGreen,
    	"0xD153A53,has begun \n");
    gameStatus = STARTUP;
}

Engine::~Engine() {
    term();
    delete gui;
}

void Engine::term(){
    actors.clearAndDelete();
    if (map) delete map;
    gui->clear();

}


void Engine::update() {
	if ( gameStatus == STARTUP ) map->computeFov();
   	gameStatus=IDLE;
    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
    if ( lastKey.vk == TCODK_ESCAPE ) {
    	save();
    	load();
    }
    player->update();
    if ( gameStatus == NEW_TURN ) {
	    for (Actor **iterator=actors.begin(); iterator != actors.end();
	        iterator++) {
	        Actor *actor=*iterator;
	        if ( actor != player ) {
	            actor->update();
	        }
	    }
	}
}

void Engine::render() {
	TCODConsole::root->clear();
	// draw the map
	map->render();
	// draw the actors
	//se->render();
	for (Actor **iterator=actors.begin();
	    iterator != actors.end(); iterator++) {
		Actor *actor=*iterator;
		if ( actor != player && ((!actor->fovOnly && map->isExplored(actor->x,actor->y))
            || map->isInFov(actor->x,actor->y)) ) {
	        actor->render();
	    }
	}
	player->render();
	gui->render();

}

void Engine::sendToBack(Actor *actor) {
	actors.remove(actor);
	actors.insertBefore(actor,0);
}

Actor *Engine::getClosestMonster(int x, int y, float range) const {
    Actor *closest = NULL;
    float bestDistance = 1E6f;

    for (Actor **iterator = actors.begin(); iterator != actors.end(); iterator++){
        Actor *actor = *iterator;
        if (actor != player && actor->destructible && !actor->destructible->isDead()){
            float distance = actor->getDistance(x,y);
            if (distance < bestDistance && (distance <= range || range == 0.0f)){
                closest = actor;
            }
        }
    }
    return closest;
}

bool Engine::pickATile(int *x, int *y, float maxRange, float aoe) {
	while ( !TCODConsole::isWindowClosed() ) {
		render();
		TCODRandom *rng = TCODRandom::getInstance();
		// highlight the possible range
		for (int cx=0; cx < map->width; cx++) {
			for (int cy=0; cy < map->height; cy++) {
				if ( map->isInFov(cx,cy)
					&& ( maxRange == 0 || player->getDistance(cx,cy) <= maxRange) ) {
					TCODColor col=TCODConsole::root->getCharBackground(cx,cy);
					col = TCODColor::darkestBlue;
					float l = rng->get(0.0f,1.0f);
					TCODConsole::root->setCharBackground(cx,cy,TCODColor::lerp(col,TCODColor::darkBlue,l));
				}
			}
		}
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE,&lastKey,&mouse);
		if ( map->isInFov(mouse.cx,mouse.cy)
			&& ( maxRange == 0 || player->getDistance(mouse.cx,mouse.cy) <= maxRange )) {
            TCODRandom *rng = TCODRandom::getInstance();
            float l = rng->get(0.0f,1.0f);
            if (aoe > 0){
                if ( map->isInFov(mouse.cx,mouse.cy)
			&& ( maxRange == 0 || player->getDistance(mouse.cx,mouse.cy) <= maxRange )) {
            for (int i = 0 ; i < aoe+1; i++){
            TCODRandom *rng = TCODRandom::getInstance();
            float l = rng->get(0.0f,1.0f);
            for (int j = 0; j < aoe+1; j++){
                int c = rng->get(0,100);
                TCODConsole::root->setCharBackground(mouse.cx+i,mouse.cy+j,TCODColor::blue);
                TCODConsole::root->setCharBackground(mouse.cx-i,mouse.cy-j,TCODColor::blue);
                TCODConsole::root->setCharBackground(mouse.cx+i,mouse.cy-j,TCODColor::blue);
                TCODConsole::root->setCharBackground(mouse.cx-i,mouse.cy+j,TCODColor::blue);
            }
            }

            }
            }
            else if (aoe == 0) {
            //TCODConsole::root->putChar(mouse.cx,mouse.cy,'X',TCOD_BKGND_DEFAULT);
            TCODConsole::root->putChar(mouse.cx-1,mouse.cy,'-',TCOD_BKGND_DEFAULT);
            TCODConsole::root->putChar(mouse.cx+1,mouse.cy,'-',TCOD_BKGND_DEFAULT);
            TCODConsole::root->putChar(mouse.cx,mouse.cy-1,'|',TCOD_BKGND_DEFAULT);
            TCODConsole::root->putChar(mouse.cx,mouse.cy+1,'|',TCOD_BKGND_DEFAULT);
			TCODConsole::root->setCharBackground(mouse.cx,mouse.cy,TCODColor::lerp(TCODColor::red,TCODColor::yellow,l));
            }
			if ( mouse.lbutton_pressed ) {
				*x=mouse.cx;
				*y=mouse.cy;
				return true;
			}
		}
		if (mouse.rbutton_pressed || lastKey.vk != TCODK_NONE) {
			return false;
		}
		TCODConsole::flush();
	}
	return false;
}

void Engine::nextLevel() {
    level++;
    gui->message(TCODColor::lightViolet,"You take a moment to rest, and recover your strength.");
    player->destructible->heal(player->destructible->maxHp/2);
    gui->message(TCODColor::red,"After a rare moment of peace, you descend\ndeeper into the heart of the dungeon...");
    delete map;
    // delete all actors but player and stairs
    for (Actor **it=actors.begin(); it!=actors.end(); it++) {
        if ( *it != player && *it != stairs ) {
            delete *it;
            it = actors.remove(it);
        }
    }
    // create a new map
    map = new Map(80,43);
    map->init(true);
    gameStatus=STARTUP;
}

