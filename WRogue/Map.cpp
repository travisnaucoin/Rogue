#include "main.hpp"
#include <stdio.h>

static const int MAX_ROOM_ITEMS = 2;
static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int RIGHT_BOT_CORNER = 1;
static const int RIGHT_TOP_CORNER = 2;
static const int VERTICAL_SIDE = 3;
static const int HORIZONTAL_SIDE = 4;
static const int TOP_LEFT_CORNER = 5;
static const int BOTTOM_LEFT_CORNER = 6;
static const int FLOOR0 = 7;
static const int FLOOR1 = 9;
static const int FLOOR2 = 10;
static const int WALL_CAP = 8;


class BspListener : public ITCODBspCallback {
private :
    Map &map; // a map to dig
    int roomNum; // room number
    int lastx,lasty; // center of the last room
public :
    BspListener(Map &map) : map(map), roomNum(0) {}
    bool visitNode(TCODBsp *node, void *userData) {
    	if ( node->isLeaf() ) {
    		int x,y,w,h;
    		bool withActors=(bool)userData;
			// dig a room
			w=map.rng->getInt(ROOM_MIN_SIZE, node->w-2);
			h=map.rng->getInt(ROOM_MIN_SIZE, node->h-2);
			x=map.rng->getInt(node->x+1, node->x+node->w-w-1);
			y=map.rng->getInt(node->y+1, node->y+node->h-h-1);
			map.createRoom(roomNum == 0, x, y, x+w-1, y+h-1, withActors);
			if ( roomNum != 0 ) {
			    // dig a corridor from last room
			    map.dig(lastx,lasty,x+w/2,lasty);
			    map.dig(x+w/2,lasty,x+w/2,y+h/2);
			}
            lastx=x+w/2;
            lasty=y+h/2;
            roomNum++;
        }
        return true;
    }
};

Map::Map(int width, int height) : width(width),height(height) {
    seed=TCODRandom::getInstance()->getInt(0,0x7FFFFFFF);
}

void Map::init(bool withActors){
    TCODConsole::mapAsciiCodeToFont(1,12,11);
    TCODConsole::mapAsciiCodeToFont(2,11,11);
    TCODConsole::mapAsciiCodeToFont(3,10,11);
    TCODConsole::mapAsciiCodeToFont(4,13,12);
    TCODConsole::mapAsciiCodeToFont(5,9,12);
    TCODConsole::mapAsciiCodeToFont(6,8,12);
    TCODConsole::mapAsciiCodeToFont(7,0,11);//floor0
    TCODConsole::mapAsciiCodeToFont(9,1,11);//floor1
    TCODConsole::mapAsciiCodeToFont(10,2,11);//floor2
    TCODConsole::mapAsciiCodeToFont(8,14,15);//wall cap

    rng = new TCODRandom(seed, TCOD_RNG_CMWC);
    tiles=new Tile[width*height];
    map=new TCODMap(width,height);
    TCODBsp bsp(0,0,width,height);
    bsp.splitRecursive(rng,5,ROOM_MAX_SIZE,ROOM_MAX_SIZE,1.5f,1.5f);
    BspListener listener(*this);
    bsp.traverseInvertedLevelOrder(&listener,(void *)withActors);
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){
             if (isWall(x,y)){tiles[x+y*width].highlightColor = TCODColor::lightGrey;}
            else setMapChar(tiles[x+y*width]);
        }
    }
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){
             if (isWall(x,y)){
                setWallChar(x,y,tiles[x+y*width]);
             }
            //else setMapChar(tiles[x+y*width]);
        }
    }
}

Map::~Map() {
    delete [] tiles;
    delete map;
}

void Map::setMapChar(Tile &t){
    TCODRandom *randChar = TCODRandom::getInstance();
    t.wallCheck = false;
    t.isFloor = true;
    t.emptySpace = true;
    int r = randChar->getInt(1,3);
    //printf("hey %i \n",r);
    switch(r){
        case 1: t.mapChar = '.'; t.mapCharColor = TCODColor::blue; t.highlightColor = TCODColor::green; t.backColor = TCODColor::green; t.backColorDark = TCODColor::darkestGreen; break;
        case 2: t.mapChar = ','; t.mapCharColor = TCODColor::darkBlue; t.highlightColor = TCODColor::green; t.backColor = TCODColor::green; t.backColorDark = TCODColor::darkestGreen; break;
        case 3: t.mapChar = ':'; t.mapCharColor = TCODColor::lightestBlue; t.highlightColor = TCODColor::green; t.backColor = TCODColor::green; t.backColorDark = TCODColor::darkestGreen; break;
    }
}

void Map::setWallChar(int x, int y,Tile &t){
     t.mapCharColor = TCODColor::darkerGreen;
    if (tiles[(x+1)+(y)*width].wallCheck && tiles[(x-1)+(y)*width].wallCheck
            && tiles[(x)+(y-1)*width].wallCheck && tiles[(x)+(y-1)*width].wallCheck
            && tiles[(x+1)+(y-1)*width].wallCheck && tiles[(x-1)+(y+1)*width].wallCheck
            && tiles[(x+1)+(y+1)*width].wallCheck && tiles[(x-1)+(y-1)*width].wallCheck){
                t.emptySpace = true;
                t.isFloor = false;
               // t.mapChar =
                //t.mapCharColor = TCODColor::white;
               // t.explored = true;
                return;
            }

    else if ((tiles[(x)+(y-1)*width].wallCheck && !tiles[(x)+(y-1)*width].emptySpace)
             & (tiles[(x+1)+(y)*width].wallCheck && !tiles[(x+1)+(y)*width].emptySpace)
             & tiles[(x+1)+(y-1)*width].getFloor()){ //left corners
        t.mapChar = BOTTOM_LEFT_CORNER;
    }
    else if ((tiles[(x)+(y-1)*width].wallCheck && !tiles[(x)+(y-1)*width].emptySpace)
             & (tiles[(x-1)+(y)*width].wallCheck && !tiles[(x-1)+(y)*width].emptySpace)
             & tiles[(x-1)+(y-1)*width].getFloor()){ //right corners
        t.mapChar = RIGHT_BOT_CORNER;
    }
    else if ((tiles[(x)+(y+1)*width].wallCheck && !tiles[(x)+(y+1)*width].emptySpace)
             & (tiles[(x-1)+(y)*width].wallCheck && !tiles[(x-1)+(y)*width].emptySpace)
             & tiles[(x-1)+(y+1)*width].getFloor()){ //top right corner
        t.mapChar = RIGHT_TOP_CORNER;
    }
    else if ((tiles[(x)+(y+1)*width].wallCheck && !tiles[(x)+(y+1)*width].emptySpace)
             & (tiles[(x+1)+(y)*width].wallCheck && !tiles[(x+1)+(y)*width].emptySpace)
             & tiles[(x+1)+(y+1)*width].getFloor()){ //top left corner
        t.mapChar = TOP_LEFT_CORNER;
    }
    else if (tiles[x+(y-1)*width].wallCheck && tiles[x+(y+1)*width].wallCheck){ //up down
        t.mapChar = VERTICAL_SIDE;
    }
    else if (tiles[(x-1)+(y)*width].wallCheck && tiles[(x+1)+(y)*width].wallCheck){ //left right
        t.mapChar = HORIZONTAL_SIDE;
    }
    else {
        t.mapChar = WALL_CAP;
    }


}

void Map::dig(int x1, int y1, int x2, int y2) {
    if ( x2 < x1 ) {
        int tmp=x2;
        x2=x1;
        x1=tmp;
    }
    if ( y2 < y1 ) {
        int tmp=y2;
        y2=y1;
        y1=tmp;
    }
    for (int tilex=x1; tilex <= x2; tilex++) {
        for (int tiley=y1; tiley <= y2; tiley++) {
            map->setProperties(tilex,tiley,true,true);
        }
    }
}

void Map::addMonster(int x, int y) {
    TCODRandom *rng=TCODRandom::getInstance();
    if ( rng->getInt(0,100) < 80 ) {
        // create an orc
        Actor *orc = new Actor(x,y,'z',"zergling",
            TCODColor::lime);
        orc->destructible = new MonsterDestructible(10,0,"dead zergling",50);
        orc->attacker = new Attacker(3);
        orc->ai = new MonsterAi();
        engine.actors.push(orc);
    } else {
        // create a troll
        Actor *troll = new Actor(x,y,'H',"Hyrdralisk",
             TCODColor::darkAmber);
        troll->destructible = new MonsterDestructible(16,1,"Hydralisk parts",100);
        troll->attacker = new Attacker(4);
        troll->ai = new MonsterAi();
        engine.actors.push(troll);
    }
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors) {
    dig (x1,y1,x2,y2);
    if (!withActors) {
        return;
    }
    if ( first ) {
        // put the player in the first room
        engine.player->x=(x1+x2)/2;
        engine.player->y=(y1+y2)/2;
    } else {
		TCODRandom *rng=TCODRandom::getInstance();
		int nbMonsters=rng->getInt(0,MAX_ROOM_MONSTERS);
		while (nbMonsters > 0) {
		    int x=rng->getInt(x1,x2);
		    int y=rng->getInt(y1,y2);
		    if ( canWalk(x,y) ) {
		        addMonster(x,y);
		    }
		    nbMonsters--;
		}
		int nbItems=rng->getInt(0,MAX_ROOM_ITEMS);
    while (nbItems > 0) {
        int x=rng->getInt(x1,x2);
        int y=rng->getInt(y1,y2);
        if ( canWalk(x,y) ) {
            addItem(x,y);
        }
        nbItems--;
    }
    }
    // set stairs position
    engine.stairs->x=(x1+x2)/2;
    engine.stairs->y=(y1+y2)/2;
}

bool Map::isWall(int x, int y) const {
    return !map->isWalkable(x,y);
}

bool Map::canWalk(int x, int y) const {
    if (isWall(x,y)) {
        // this is a wall
        return false;
    }
    for (Actor **iterator=engine.actors.begin();
        iterator!=engine.actors.end();iterator++) {
        Actor *actor=*iterator;
        if ( actor->blocks && actor->x == x && actor->y == y ) {
            // there is a blocking actor here. cannot walk
            return false;
        }
    }
    return true;
}

bool Map::isExplored(int x, int y) const {
    return tiles[x+y*width].explored;
}

bool Map::isInFov(int x, int y) const {
    if ( x < 0 || x >= width || y < 0 || y >= height ) {
		return false;
	}
    if ( map->isInFov(x,y) ) {
        tiles[x+y*width].explored=true;
        return true;
    }
    return false;
}

void Map::computeFov() {
    map->computeFov(engine.player->x,engine.player->y,
        engine.fovRadius);
}

void Map::render() const {
	#define TORCH_RADIUS 10.0f
	#define SQUARED_TORCH_RADIUS (TORCH_RADIUS*TORCH_RADIUS)
	static const TCODColor darkWall(TCODColor::darkestBlue);
	static const TCODColor darkGround(TCODColor::darkestGreen);
	//static const TCODColor lightWall(TCODColor::darkerSky);
	//static const TCODColor lightGround(TCODColor::lightestHan);
	static float torchx = 0.0f; // torch light position in the perlin noise
	static TCODNoise *noise = NULL;
	noise = new TCODNoise(1);
	float dx = 0.0f, dy = 0.0f, di = 0.0f;


	for (int x = 0; x < width; x++){
		for (int y = 0; y < height; y++){
			if (isInFov(x, y)){


				// slightly change the perlin noise parameter
				torchx += 0.2f;
				// randomize the light position between -1.5 and 1.5
				float tdx = torchx + 20.0f;
				dx = noise->get(&tdx)*1.0f;
				tdx += 30.0f;
				dy = noise->get(&tdx)*1.5f;
				// randomize the light intensity between -0.2 and 0.2
				di = 0.2f * noise->get(&torchx);
				//TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? lightWall : lightGround);
				// torch flickering fx
				TCODColor base = (isWall(x, y) ? darkWall : darkGround);
				TCODColor light = (isWall(x, y) ? tiles[x+y*width].highlightColor : tiles[x+y*width].highlightColor);
				TCODColor backLight = (isWall(x,y) ? tiles[x+y*width].backColor : tiles[x+y*width].backColor);
				TCODColor backDark =  (isWall(x,y) ? tiles[x+y*width].backColorDark : tiles[x+y*width].backColorDark);
				// cell distance to torch (squared)
				float r = (float)((x - engine.player->x + dx)*(x - engine.player->x + dx) + (y - engine.player->y + dy)*(y - engine.player->y + dy));
				if (r < SQUARED_TORCH_RADIUS) {
					// l = 1.0 at player position, 0.0 at a radius of 10 cells
					float l = (SQUARED_TORCH_RADIUS - r) / SQUARED_TORCH_RADIUS + di;
					l = CLAMP(0.0f, 1.0f, l);
					// interpolate the color
					base = TCODColor::lerp(base, light, l);
					backLight = TCODColor::lerp(backDark,backLight,l);
				}
				light = base;


				TCODConsole::root->setChar(x, y, isWall(x, y) ? tiles[x+y*width].mapChar : tiles[x+y*width].mapChar);
                (isWall(x,y)) ? TCODConsole::root->setCharForeground(x,y,light) : TCODConsole::root->setCharForeground(x,y,TCODColor::lightGrey);
                (isWall(x,y)) ? TCODConsole::root->setCharForeground(x,y,light) : TCODConsole::root->setCharBackground(x,y,backLight);
			}
			else if (isExplored(x, y)){
				TCODConsole::root->setChar(x, y, isWall(x,y) ? tiles[x+y*width].mapChar : tiles[x+y*width].mapChar);
				TCODConsole::root->setCharForeground(x, y,isWall(x, y) ? darkWall : darkGround);
			}
		}
	}
}

void Map::addItem(int x, int y) {
    TCODRandom *rng = TCODRandom::getInstance();
    int r = rng->getInt(0,100);

        if(r < 5){
            Actor *healthPotion=new Actor(x,y,'o',"Health Potion",
            TCODColor::lightRed);
            healthPotion->blocks=false;
            healthPotion->pickable=new Healer(4);
            engine.actors.push(healthPotion);
            engine.sendToBack(healthPotion);
        }
        else if(r < 4){
            Actor *superHealthPotion=new Actor(x,y,'o',"Super Health Potion",
            TCODColor::lightViolet);
            superHealthPotion->blocks=false;
            superHealthPotion->pickable=new Healer(10);
            engine.actors.push(superHealthPotion);
            engine.sendToBack(superHealthPotion);
        }
        else if(r > 1000){
            Actor *spellOfLightiningBolt=new Actor(x,y,'#',"Spell of Lightining Bolt",
            TCODColor::yellow);
            spellOfLightiningBolt->blocks=false;
            spellOfLightiningBolt->pickable=new LightningBolt(3,20,20);
            engine.actors.push(spellOfLightiningBolt);
            engine.sendToBack(spellOfLightiningBolt);
        }
        else if ( r > 0 ) {
    // create a scroll of fireball
        Actor *scrollOfFirebolt=new Actor(x,y,'#',"scroll of firebolt",
        TCODColor::lightYellow);
        scrollOfFirebolt->blocks=false;
        scrollOfFirebolt->pickable=new FireBolt(2,12,50);
        engine.actors.push(scrollOfFirebolt);
        engine.sendToBack(scrollOfFirebolt);
        }
}


