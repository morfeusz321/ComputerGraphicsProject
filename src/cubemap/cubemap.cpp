// cubemap.cpp
#include "cubemap.h"
#include <stb/stb_image.h>
#include <iostream>

std::vector<std::string> CubeMap::face_suffixes = {
    "right.png",  // GL_TEXTURE_CUBE_MAP_POSITIVE_X
    "left.png",   // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    "top.png",    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    "bottom.png", // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    "front.png",  // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    "back.png"    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

CubeMap::CubeMap(const std::string& directory)
    : m_handle(0)
{
    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (!loadCubemapFaces(directory)) {
        std::cerr << "Failed to load cubemap textures from directory: " << directory << std::endl;
    }
}

CubeMap::~CubeMap() {
    if (m_handle != 0) {
        glDeleteTextures(1, &m_handle);
    }
}

CubeMap::CubeMap(CubeMap&& other) noexcept
    : m_handle(other.m_handle)
{
    other.m_handle = 0;
}

CubeMap& CubeMap::operator=(CubeMap&& other) noexcept {
    if (this != &other) {
        if (m_handle != 0) {
            glDeleteTextures(1, &m_handle);
        }
        m_handle = other.m_handle;
        other.m_handle = 0;
    }
    return *this;
}

void CubeMap::bind(GLenum textureUnit) const {
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_handle);
}

bool CubeMap::loadCubemapFaces(const std::string& directory) {
    stbi_set_flip_vertically_on_load(false);

    for (GLuint i = 0; i < 6; i++) {
        std::string filepath = directory + face_suffixes[i];
        int width, height, nrChannels;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
        
        if (data) {
            GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap texture failed to load at path: " << filepath << std::endl;
            stbi_image_free(data);
            return false;
        }
    }

    return true;
}