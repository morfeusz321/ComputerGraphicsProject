#pragma once

#include "mesh.h"
#include "texture.h"
#include <string>
#include <glad/glad.h>

class Box {
public:
    explicit Box(const std::string& directory);
    ~Box();

    // Move constructors
    Box(Box&& other) noexcept;
    Box& operator=(Box&& other) noexcept;

    // Disable copying
    Box(const Box&) = delete;
    Box& operator=(const Box&) = delete;

    // Bind the textures to specified texture units
    void bindColorTexture(GLenum textureUnit) const;
    void bindNormalTexture(GLenum textureUnit) const;

    // Accessor for the mesh to render
    const std::vector<GPUMesh>& getMeshes() const { return m_meshes; }

private:
    GLuint m_colorTextureHandle;
    GLuint m_normalTextureHandle;
    std::vector<GPUMesh> m_meshes;

    bool loadTexture(const std::string& filePath, GLuint& textureHandle);
};
