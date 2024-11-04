#include "box.h"
#include <stb/stb_image.h>
#include <iostream>

Box::Box(const std::string& directory)
    : m_colorTextureHandle(0), m_normalTextureHandle(0)
{
    // Load the box mesh
    m_meshes = GPUMesh::loadMeshGPU(directory + "/box.obj");

    // Generate and load textures
    glGenTextures(1, &m_colorTextureHandle);
    glGenTextures(1, &m_normalTextureHandle);

    if (!loadTexture(directory + "/box_base_color.png", m_colorTextureHandle)) {
        std::cerr << "Failed to load color texture from directory: " << directory << std::endl;
    }

    if (!loadTexture(directory + "/box_normal.png", m_normalTextureHandle)) {
        std::cerr << "Failed to load normal texture from directory: " << directory << std::endl;
    }
}

Box::~Box() {
    if (m_colorTextureHandle != 0) {
        glDeleteTextures(1, &m_colorTextureHandle);
    }
    if (m_normalTextureHandle != 0) {
        glDeleteTextures(1, &m_normalTextureHandle);
    }
}

Box::Box(Box&& other) noexcept
    : m_colorTextureHandle(other.m_colorTextureHandle),
    m_normalTextureHandle(other.m_normalTextureHandle),
    m_meshes(std::move(other.m_meshes))
{
    other.m_colorTextureHandle = 0;
    other.m_normalTextureHandle = 0;
}

Box& Box::operator=(Box&& other) noexcept {
    if (this != &other) {
        if (m_colorTextureHandle != 0) {
            glDeleteTextures(1, &m_colorTextureHandle);
        }
        if (m_normalTextureHandle != 0) {
            glDeleteTextures(1, &m_normalTextureHandle);
        }

        m_colorTextureHandle = other.m_colorTextureHandle;
        m_normalTextureHandle = other.m_normalTextureHandle;
        m_meshes = std::move(other.m_meshes);

        other.m_colorTextureHandle = 0;
        other.m_normalTextureHandle = 0;
    }
    return *this;
}

bool Box::loadTexture(const std::string& filePath, GLuint& textureHandle) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        return true;
    }
    else {
        std::cerr << "Failed to load texture at path: " << filePath << std::endl;
        stbi_image_free(data);
        return false;
    }
}

void Box::bindColorTexture(GLenum textureUnit) const {
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_colorTextureHandle);
}

void Box::bindNormalTexture(GLenum textureUnit) const {
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_normalTextureHandle);
}
