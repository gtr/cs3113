#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 2;
    int enemies_dead = 0;

    ~LevelB();

    void initialize_enemies();
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void initialize_fire();
};
