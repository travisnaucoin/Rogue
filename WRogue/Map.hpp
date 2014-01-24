struct Tile {
    bool explored; // has the player already seen this tile ?
    char mapChar;
    bool wallCheck;
    bool emptySpace;
    bool isFloor;
    TCODColor mapCharColor;
    TCODColor highlightColor;
    TCODColor backColor;
    TCODColor backColorDark;
    Tile() : explored(false), mapChar('.'), mapCharColor(TCODColor::white), wallCheck(true), emptySpace(false), isFloor(false), backColor(TCODColor::black), backColorDark(TCODColor::black) {}
    bool getWallCheck() {
        return wallCheck;
    }
    bool getEmptySpace() {
        return emptySpace;
    }
    bool getFloor(){
        return isFloor;
    }
};

class Map : public Persistent {;
public :
    int width,height;

    Map(int width, int height);
    ~Map();
    void setMapChar(Tile &t);
    void setWallChar (int x, int y,Tile &t);
    bool isWall(int x, int y) const;
	bool isInFov(int x, int y) const;
    bool isExplored(int x, int y) const;
    bool canWalk(int x, int y) const;
    void computeFov();
    void render() const;
    void addItem(int x, int y);
    void init(bool withActors);
    void load(TCODZip &zip);
    void save(TCODZip &zip);
protected :
    Tile *tiles;
    TCODMap *map;
    long seed;
    TCODRandom *rng;
    friend class BspListener;

    void dig(int x1, int y1, int x2, int y2);
    void createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors);
    void addMonster(int x, int y);
};
