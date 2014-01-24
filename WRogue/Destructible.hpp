class Destructible : public Persistent {
public :
	float maxHp; // maximum health points
	float hp; // current health points
	float mana;
	float maxMana;
	float food;
	float maxFood;
	float defense; // hit points deflected
	const char *corpseName; // the actor's name once dead/destroyed

	Destructible(float maxHp, float maxMana, float maxFood, float defense, const char *corpseName);
	~Destructible();
	float heal(float amount);
	inline bool isDead() { return hp <= 0; }
	float takeDamage(Actor *owner, float damage);
	virtual void die(Actor *owner);
	void load(TCODZip &zip);
    void save(TCODZip &zip);
    static Destructible *create(TCODZip &zip);
protected :
    enum DestructibleType {
        MONSTER,PLAYER
    };

};

class MonsterDestructible : public Destructible {
public :
	MonsterDestructible(float maxHp, float defense, const char *corpseName);
	void die(Actor *owner);
    void save(TCODZip &zip);
};

class PlayerDestructible : public Destructible {
public :
	PlayerDestructible(float maxHp, float maxMana, float maxFood, float defense, const char *corpseName);
	void die(Actor *owner);
    void save(TCODZip &zip);
};
