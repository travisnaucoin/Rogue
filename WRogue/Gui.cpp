#include <stdio.h>
#include <stdarg.h>
#include "main.hpp"

static const int PANEL_HEIGHT=49;
static const int BAR_WIDTH=30;
static const int MSG_X=1;
static const int MSG_HEIGHT=5;
static const int LOG_HEIGHT = 8;
float l = 0.0f;



Gui::Gui() {
	con = new TCODConsole(engine.screenWidth,PANEL_HEIGHT);
	battleLog = new TCODConsole (engine.screenWidth,LOG_HEIGHT);
}

Gui::~Gui() {
    delete battleLog;
	delete con;
	clear();
}

void Gui::clear(){
    log.clearAndDelete();
}

void Gui::render() {
	// clear the GUI console
	//TCODRandom *ranCol = TCODRandom::getInstance();
	if (l == 0.0f & l != 1.0f){
        l += 0.5f;
	}
	else if (l <= 1.0f ){
        l -= 0.05f;
	}
    //printf("%f",engine.player->destructible->maxHp);
	//l = CLAMP(0.0f, 1.0f, l);
	//printf("%i \n",l);

    con->setDefaultForeground(TCODColor::lightestGrey);
	con->setDefaultBackground(TCODColor::darkestGrey);
	con->clear();
    battleLog->printFrame(0,0,engine.screenWidth-BAR_WIDTH,LOG_HEIGHT,true,TCOD_BKGND_DEFAULT,"Combat Log");
	con->printFrame(0,0,BAR_WIDTH,PANEL_HEIGHT,true,TCOD_BKGND_DEFAULT,"Character Information");

	con->hline(1,1,BAR_WIDTH-2,TCOD_BKGND_DEFAULT);
	con->hline(1,7,BAR_WIDTH-2,TCOD_BKGND_DEFAULT);
    con->print(1,2,"Life:");
    con->print(1,4,"Mana:");
    con->print(1,6,"Food:");

	// draw the health bar
	if (engine.player->destructible->hp <= (engine.player->destructible->maxHp/2)){
    renderBar(7,2,BAR_WIDTH-10,"",engine.player->destructible->hp,
		engine.player->destructible->maxHp,
		(TCODColor::lerp(TCODColor::lightestRed,TCODColor::red,l)),TCODColor::darkestCrimson);
	}
	else if (engine.player->destructible->hp > (engine.player->destructible->maxHp/2)){
        renderBar(7,2,BAR_WIDTH-10,"",engine.player->destructible->hp,
		engine.player->destructible->maxHp,
		TCODColor::red,TCODColor::darkestCrimson);
	}
    //mana bar
    if (engine.player->destructible->mana <= (engine.player->destructible->maxMana/2)){
    renderBar(7,4,BAR_WIDTH-10,"",engine.player->destructible->mana,
		engine.player->destructible->maxMana,
		(TCODColor::lerp(TCODColor::lightestBlue,TCODColor::blue,l)),TCODColor::darkestBlue);
    }
    else if (engine.player->destructible->mana > (engine.player->destructible->maxMana/2)){
    renderBar(7,4,BAR_WIDTH-10,"",engine.player->destructible->mana,
		engine.player->destructible->maxMana,
		(TCODColor::blue),TCODColor::darkestBlue);
    }
    //food bar
    if (engine.player->destructible->mana <= (engine.player->destructible->maxMana/2)){
    renderBar(7,6,BAR_WIDTH-10,"",engine.player->destructible->food,
		engine.player->destructible->maxFood,
		(TCODColor::lerp(TCODColor::lightestGreen,TCODColor::green,l)),TCODColor::darkestGreen);
    }
    else if (engine.player->destructible->mana > (engine.player->destructible->maxMana/2)){
    renderBar(7,6,BAR_WIDTH-10,"",engine.player->destructible->food,
		engine.player->destructible->maxFood,
		(TCODColor::green),TCODColor::darkestGreen);
    }


	// draw the message log
	int y=1;
	float colorCoef=0.4f;
	for (Message **it=log.begin(); it != log.end(); it++) {
		Message *message=*it;
		battleLog->setDefaultForeground(message->col * colorCoef);
		battleLog->print(MSG_X,y,message->text);
		y++;
		if ( colorCoef < 1.0f ) {
			colorCoef+=0.3f;
		}
	}

	// mouse look
	renderMouseLook();

	// blit the GUI console on the root console
	// dungeon level
    battleLog->setDefaultForeground(TCODColor::white);
    battleLog->print(3,3,"Dungeon level %d",engine.level);
	TCODConsole::blit(con,0,0,BAR_WIDTH,engine.screenHeight,TCODConsole::root,engine.screenWidth-BAR_WIDTH,0);
	TCODConsole::blit(battleLog,0,0,engine.screenWidth-BAR_WIDTH,LOG_HEIGHT,TCODConsole::root,0,engine.screenHeight-LOG_HEIGHT);
}

void Gui::renderBar(int x, int y, int width, const char *name,
	float value, float maxValue, const TCODColor &barColor,
	const TCODColor &backColor) {
	// fill the background
	con->setDefaultBackground(backColor);
	con->rect(x,y,width,1,false,TCOD_BKGND_SET);

	int barWidth = (int)(value / maxValue * width);
	if ( barWidth > 0 ) {
		// draw the bar
		con->setDefaultBackground(barColor);
		con->rect(x,y,barWidth,1,false,TCOD_BKGND_SET);
	}
	// print text on top of the bar
	con->setDefaultForeground(TCODColor::white);
	con->printEx(x+width/2,y,TCOD_BKGND_NONE,TCOD_CENTER,
		"%s  %g/%g", name, value, maxValue);
}

Gui::Message::Message(const char *text, const TCODColor &col) :
	text(strdup(text)),col(col) {
}

Gui::Message::~Message() {
	free(text);
}

void Gui::renderMouseLook() {
	if (! engine.map->isInFov(engine.mouse.cx, engine.mouse.cy)) {
		// if mouse is out of fov, nothing to render
		return;
	}
	char buf[128]="";
	bool first=true;
	for (Actor **it=engine.actors.begin(); it != engine.actors.end(); it++) {
		Actor *actor=*it;
		// find actors under the mouse cursor
		if (actor->x == engine.mouse.cx && actor->y == engine.mouse.cy ) {
			if (! first) {
				strcat(buf,", ");
			} else {
				first=false;
			}
			strcat(buf,actor->name);
		}
	}
	// display the list of actors under the mouse cursor
	TCODConsole::root->setDefaultForeground(TCODColor::white);
	TCODConsole::root->print(engine.mouse.cx,engine.mouse.cy-0.5f,buf);
}

void Gui::message(const TCODColor &col, const char *text, ...) {
	// build the text
	va_list ap;
	char buf[128];
	va_start(ap,text);
	vsprintf(buf,text,ap);
	va_end(ap);

	char *lineBegin=buf;
	char *lineEnd;
	do {
		// make room for the new message
		if ( log.size() == MSG_HEIGHT ) {
			Message *toRemove=log.get(0);
			log.remove(toRemove);
			delete toRemove;
		}

		// detect end of the line
		lineEnd=strchr(lineBegin,'\n');
		if ( lineEnd ) {
			*lineEnd='\0';
		}

		// add a new message to the log
		Message *msg=new Message(lineBegin, col);
		log.push(msg);

		// go to next line
		lineBegin=lineEnd+1;
	} while ( lineEnd );
}

Menu::Menu(){
    printf("menu started");
}

Menu::~Menu() {
	clear();
}

void Menu::clear() {
	items.clearAndDelete();
}

void Menu::addItem(MenuItemCode code, const char *label) {
	MenuItem *item=new MenuItem();
	item->code=code;
	item->label=label;
	items.push(item);
}

Menu::MenuItemCode Menu::pick() {
    TCODConsole::initRoot(70,50,"0xD153A53",false);
	static TCODImage img("menuimage1.png");
	int selectedItem=0;
	while( !TCODConsole::isWindowClosed() ) {
		img.blit2x(TCODConsole::root,0,0);
		int currentItem=0;
		for (MenuItem **it=items.begin(); it!=items.end(); it++) {
			if ( currentItem == selectedItem ) {
				TCODConsole::root->setDefaultForeground(TCODColor::darkBlue);
			} else {
				TCODConsole::root->setDefaultForeground(TCODColor::lightGrey);
			}
			TCODConsole::root->print(10,10+currentItem*3,(*it)->label);
			currentItem++;
		}
		TCODConsole::flush();

		// check key presses
		TCOD_key_t key;
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&key,NULL);
		switch (key.vk) {
			case TCODK_UP :
				selectedItem--;
				if (selectedItem < 0) {
					selectedItem=items.size()-1;
				}
			break;
			case TCODK_DOWN :
				selectedItem = (selectedItem + 1) % items.size();
			break;
			case TCODK_ENTER :
				return items.get(selectedItem)->code;
			default : break;
		}
	}
	return NONE;
}
