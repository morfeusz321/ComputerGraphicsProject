#include "skybox.h"
#include <iostream>

Skybox::Skybox(const std::string& shaderPath)
    : m_vao(0)
    , m_vbo(0)
{
    try {
        ShaderBuilder skyboxBuilder;
        skyboxBuilder.addStage(GL_VERTEX_SHADER, shaderPath + "skybox_vert.glsl");
        skyboxBuilder.addStage(GL_FRAGMENT_SHADER, shaderPath + "skybox_frag.glsl");
        m_shader = skyboxBuilder.build();
    } catch (ShaderLoadingException e) {
        std::cerr << "Failed to load skybox shaders: " << e.what() << std::endl;
        throw;
    }
    
    setup();
}

Skybox::~Skybox() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
    }
}

Skybox::Skybox(Skybox&& other) noexcept
    : m_vao(other.m_vao)
    , m_vbo(other.m_vbo)
    , m_shader(std::move(other.m_shader))
{
    other.m_vao = 0;
    other.m_vbo = 0;
}

Skybox& Skybox::operator=(Skybox&& other) noexcept {
    if (this != &other) {
        // Clean up existing resources
        if (m_vao != 0) {
            glDeleteVertexArrays(1, &m_vao);
        }
        if (m_vbo != 0) {
            glDeleteBuffers(1, &m_vbo);
        }
        
        // Move resources
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_shader = std::move(other.m_shader);
        
        // Null out other's resources
        other.m_vao = 0;
        other.m_vbo = 0;
    }
    return *this;
}

void Skybox::setup() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Skybox::render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const CubeMap& cubemap) {
    glDepthFunc(GL_LEQUAL);
    m_shader.bind();
    
    // Remove translation from view matrix
    glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(viewMatrix));
    
    glUniformMatrix4fv(m_shader.getUniformLocation("projection"), 1, GL_FALSE, 
        glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(m_shader.getUniformLocation("view"), 1, GL_FALSE, 
        glm::value_ptr(viewWithoutTranslation));
    
    // Bind cubemap texture
    cubemap.bind(GL_TEXTURE0);
    glUniform1i(m_shader.getUniformLocation("skybox"), 0);
    
    // Render skybox cube
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    glDepthFunc(GL_LESS);
}