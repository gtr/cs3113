#include "Scene.h"

class LevelC : public Scene {
public:
    int ENEMY_COUNT = 3;
    int enemies_dead = 0;

    ~LevelC();

    void initialize_enemies();
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
    void initialize_fire();
};
