#include "Scene.h"

class Menu : public Scene {
public:
    int ENEMY_COUNT = 1;
    GLuint txt ;

    ~Menu();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
