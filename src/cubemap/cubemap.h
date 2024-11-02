// cubemap.h
#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>

class CubeMap {
public:
    CubeMap(const std::string& directory);
    ~CubeMap();

    // Prevent copying
    CubeMap(const CubeMap&) = delete;
    CubeMap& operator=(const CubeMap&) = delete;

    // Allow moving
    CubeMap(CubeMap&&) noexcept;
    CubeMap& operator=(CubeMap&&) noexcept;

    void bind(GLenum textureUnit) const;
    GLuint getHandle() const { return m_handle; }

private:
    GLuint m_handle;
    bool loadCubemapFaces(const std::string& directory);
    static std::vector<std::string> face_suffixes;
};