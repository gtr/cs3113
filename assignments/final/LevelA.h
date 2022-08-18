#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 1;
    int enemies_dead = 0;

    float speed = 5.0f;
    float ledge = 0.0f;
    bool start = true;

    ~LevelA();
    
    void initialize_gliders();
    void initialize_shooters();
    void initialize_boss();
    void initialize_enemies();

    void initialize_player();
    void initialize_rocks();
    void initialize_coins();
    void initialize_fire();
    void initialize_map();
    
    void shoot_fire();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
