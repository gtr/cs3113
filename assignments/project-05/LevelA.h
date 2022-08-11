#include "Scene.h"

class LevelA : public Scene {
public:
    int ENEMY_COUNT = 1;
    int enemies_dead = 0;
    
    ~LevelA();
    
    void initialize_enemies();
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void initialize_fire();
};
