#include "GUI.h"

static const PANEL_HEIGHT = 7;
static const BAR_WIDTH = 20;

GUI::GUI()
{
    //ctor
    con = new TCODConsole(engine.screenWidth,PANEL_HEIGHT);
}

GUI::~GUI()
{
    //dtor
    delete con;
}

void GUI::render(){
    con->setDefaultBackground();
    con->clear();
    renderBar(1,1,BAR_WIDTH,"Life:",engine->player->destructible->hp,
              engine->player->destructible->maxHp,TCODColor::darkerCrimson,
              TCODColor::darkAzure);
    TCODConsole::blit(con,0,0,engine->screenWidth,PANEL_HEIGHT,TCODConsole::root,0,
                      engine.screenHeight-PANEL_HEIGHT);
}

void Gui::renderBar(int x, int y, int width, const char *name,
    float value, float maxValue, const TCODColor &barColor,
    const TCODColor &backColor) {
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
        "%s : %g/%g", name, value, maxValue);
}

