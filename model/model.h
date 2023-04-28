#include <string>

class Model
{
public:
    Model(std::string vertpath, std::string fragpath);
    ~Model();

    std::shared_ptr<_shader> shader;

private:
    GLuint vertbuff, uvbuff, texbuff;
};