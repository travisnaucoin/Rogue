class Pickable : public Persistent {
public :
	bool pick(Actor *owner, Actor *wearer);
	virtual bool use(Actor *owner, Actor *wearer);
	static Pickable *create (TCODZip &zip);
protected :
    enum PickableType {
        HEALER,LIGHTNING_BOLT,CONFUSER,FIREBALL
    };
};

class Healer : public Pickable {
public :
	float amount; // how many hp

	Healer(float amount);
	bool use(Actor *owner, Actor *wearer);
	void load(TCODZip &zip);
    void save(TCODZip &zip);
};

class LightningBolt: public Pickable {
public :
    float cost;
    float range,damage;
    LightningBolt(float range, float damage, float cost);
    bool use(Actor *owner, Actor *wearer);
    void effect(int x, int y, Actor *a);
    void load(TCODZip &zip);
    void save(TCODZip &zip);
};

class FireBolt: public LightningBolt{
public:
    FireBolt(float range, float damage, float cost);
    bool use (Actor *owner, Actor *wearer);
    void effect(int x, int y);
    void save(TCODZip &zip);

};
