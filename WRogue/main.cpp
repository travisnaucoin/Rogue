#include "main.hpp"

Engine engine(110,50);


int main() {
    Sound::initialise();
    engine.load();
    while ( !TCODConsole::isWindowClosed() ) {

    	engine.update();
    	engine.render();
		TCODConsole::flush();
    }
    engine.save();
    return 0;
}
