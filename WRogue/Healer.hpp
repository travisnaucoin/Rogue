class Healer : public Pickable {
public :
    float amount; // how many hp

    Healer(float amount);
    bool use(Actor *owner, Actor *wearer);
};
