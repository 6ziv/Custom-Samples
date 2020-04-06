#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "textureloader.hpp"
#include <stb_image.h>
#include "apputils.hpp"
#include "openglhelper.hpp"
GLuint TextureLoader::CreateTextureFromPngFile(std::string filename)
{
    if(loaded_textures.find(filename)!=loaded_textures.cend()){
        return loaded_textures[filename];
    }
    GLuint textureId;
    int width, height, channels;
    unsigned char* png;
    AppUtils::FileContent data;
    if(!AppUtils::readFileContent(filename, data))
        return 0;
    png = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(data.data()),
        static_cast<int>(data.size()),
        &width,&height,&channels,STBI_rgb_alpha);

    OpenGLHelper::get()->glGenTextures(1, &textureId);
    OpenGLHelper::get()->glBindTexture(GL_TEXTURE_2D, textureId);
    OpenGLHelper::get()->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, png);
    OpenGLHelper::get()->glGenerateMipmap(GL_TEXTURE_2D);
    OpenGLHelper::get()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    OpenGLHelper::get()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    OpenGLHelper::get()->glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(png);
    data.clear();
    loaded_textures[filename]=textureId;
    return textureId;
}
