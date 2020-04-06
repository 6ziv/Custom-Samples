#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H
#include <string>
#include <QOpenGLFunctions>
class TextureLoader
{
public:
    static GLuint CreateTextureFromPngFile(std::string fileName);
private:
    inline static std::map<std::string,GLuint> loaded_textures;
};
#endif
