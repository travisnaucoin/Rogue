class GUI
{
    public:
        GUI();
       ~GUI();
       void render();
    protected:
        TCODConsole *con;

        void renderBar(int x, int y, int width, const char *name,
        float value, float maxValue, const TCODColor &barColor,
        const TCODColor &backColor);
};

