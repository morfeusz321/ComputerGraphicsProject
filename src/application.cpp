//#include "Image.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"
// Always include window first (because it includes glfw, which includes GL, which needs to be included AFTER glew).
// Can't wait for modules to fix this stuff...
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/window.h>
#include <functional>
#include <iostream>
#include <vector>

class Application {
public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024), OpenGLVersion::GL41)
        , m_texture(RESOURCE_ROOT "resources/checkerboard.png")
        , m_camera_front(&m_window, glm::vec3(20, 3, 20), glm::vec3(-1, 0, -1)) // camera for front view
        , m_camera_top(&m_window, glm::vec3(0, 15, -10), glm::vec3(0, -1, 1))  // camera for top view
        , m_activeCamera(m_camera_front)
    {
        m_window.registerKeyCallback([this](int key, int scancode, int action, int mods) {
            if (action == GLFW_PRESS)
                onKeyPressed(key, mods);
            else if (action == GLFW_RELEASE)
                onKeyReleased(key, mods);
            });
        m_window.registerMouseMoveCallback(std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        m_window.registerMouseButtonCallback([this](int button, int action, int mods) {
            if (action == GLFW_PRESS)
                onMouseClicked(button, mods);
            else if (action == GLFW_RELEASE)
                onMouseReleased(button, mods);
            });

        // Load meshes for tank body, wheels, gun, and turret
        m_wheel_pair = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/wheel.obj");
        m_tank_body = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/tank_body.obj");
        m_gun = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/gun.obj");
        m_turret = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/turret.obj");

        try {
            ShaderBuilder defaultBuilder;
            defaultBuilder.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();

            ShaderBuilder shadowBuilder;
            shadowBuilder.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/shadow_vert.glsl");
            shadowBuilder.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "Shaders/shadow_frag.glsl");
            m_shadowShader = shadowBuilder.build();
        }
        catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }

        resetSimulation();
    }

    void update() {
        int dummyInteger = 0; // Initialized to 0
        const float deltaTime = 0.016f; // Assuming a fixed timestep of 16ms 

        while (!m_window.shouldClose()) {
            m_window.updateInput();
            m_activeCamera.updateInput();

            ImGui::Begin("Window");
            if (ImGui::Button("Reset Simulation")) {
                resetSimulation();
            }
            ImGui::Separator();
            ImGui::Text("Tank Controls");

            ImGui::Checkbox("Drive Tank", &driveTank);
            if (driveTank) {
                ImGui::SliderFloat("Tank Speed", &tankSpeed, 0.01f, 1.0f, "%.2f");
            }

            ImGui::Text("Turret Controls");
            ImGui::SliderInt("Turret Angle", &turretRotationAngle, -45, 45);
            ImGui::Checkbox("Animate Turret Movement", &animateTurret);
            if (animateTurret) {
                ImGui::SliderFloat("Turret Rotation Speed", &turretRotationSpeed, 1.0f, 10.0f);
            }

            ImGui::Text("Bazooka Controls");
            ImGui::Checkbox("Animate Bazooka Tilt", &animateGun);
            if (animateGun) {
                ImGui::SliderFloat("Bazooka Tilt Speed", &gunTiltSpeed, 0.5f, 5.0f);
            }
            ImGui::SliderFloat("Gun Tilt Angle", &gunTiltAngle, -20.0f, 20.0f);

            ImGui::Separator();
            ImGui::Text("Texture Controls");
            ImGui::Checkbox("Use material if no texture", &m_useMaterial);
            ImGui::End();

            if (animateTurret && !turretAngleManuallySet) {
                animateTurretRotation();
            }

            if (animateGun && !gunAngleManuallySet) {
                animateGunTilt();
            }

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            if (driveTank) {
                float distanceTraveled = tankSpeed * deltaTime; // Distance = speed * time
                tankCoords += glm::vec3(0.0f, 0.0f, distanceTraveled);

                // Update the wheel rotation based on the distance traveled
                rotationAngleWheels += (distanceTraveled / radiusWheels);
            }

            renderTank();
            m_window.swapBuffers();
        }
    }

    void animateTurretRotation() {
        turretRotationAngle += turretDirection * turretRotationSpeed;
        if (turretRotationAngle >= 30) {
            turretDirection = -1;
        }
        else if (turretRotationAngle <= -30) {
            turretDirection = 1;
        }
    }

    void animateGunTilt() {
        gunTiltAngle += gunDirection * gunTiltSpeed;
        if (gunTiltAngle >= 20.0f) {
            gunDirection = -1;
        }
        else if (gunTiltAngle <= -20.0f) {
            gunDirection = 1;
        }
    }

    void renderTank() {
        glm::mat4 tankBodyMatrix = glm::translate(glm::mat4(1.0f), tankCoords);
        renderMesh(m_tank_body, tankBodyMatrix);

        std::vector<float> z_offsets = { -5.0f, -2.3f, 0.0f };
        for (float z_offset : z_offsets) {
            glm::mat4 wheelPositionMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, z_offset));
            glm::mat4 wheelRotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngleWheels, glm::vec3(1.0f, 0.0f, 0.0f));

            glm::mat4 finalWheelMatrix = tankBodyMatrix * wheelPositionMatrix * wheelRotationMatrix;

            renderMesh(m_wheel_pair, finalWheelMatrix);
        }

        glm::mat4 turretModelMatrix = glm::translate(tankBodyMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        turretModelMatrix = glm::rotate(turretModelMatrix, glm::radians((float)turretRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        renderMesh(m_turret, turretModelMatrix);

        glm::mat4 gunModelMatrix = glm::translate(turretModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        gunModelMatrix = glm::rotate(gunModelMatrix, glm::radians(gunTiltAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        renderMesh(m_gun, gunModelMatrix);
    }



    void resetSimulation() {
        tankCoords = glm::vec3(0.0f);
        driveTank = false;
        animateTurret = false;
        animateGun = false;
        tankSpeed = 0.05f;
        rotationAngleWheels = 0.0f;
        turretRotationAngle = 0;
        turretRotationSpeed = 1.0f;
        gunTiltAngle = 0.0f;
        gunTiltSpeed = 1.0f;
        turretDirection = 1;
        gunDirection = 1;
        turretAngleManuallySet = false;
        gunAngleManuallySet = false;
    }

    void onKeyPressed(int key, int mods) {
        if (key == GLFW_KEY_1) {
            m_activeCamera = m_camera_front;
        }
        else if (key == GLFW_KEY_2) {
            m_activeCamera = m_camera_top;
        }
    }

    void onKeyReleased(int key, int mods) {
        std::cout << "Key released: " << key << std::endl;
    }

    void onMouseMove(const glm::dvec2& cursorPos) {
        std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
    }

    void onMouseClicked(int button, int mods) {
        std::cout << "Pressed mouse button: " << button << std::endl;
    }

    void onMouseReleased(int button, int mods) {
        std::cout << "Released mouse button: " << button << std::endl;
    }

    void renderMesh(std::vector<GPUMesh>& meshes, const glm::mat4& modelMatrix) {
        const glm::mat4 mvpMatrix = m_projectionMatrix * m_activeCamera.viewMatrix() * modelMatrix;
        const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(modelMatrix));

        for (GPUMesh& mesh : meshes) {
            m_defaultShader.bind();
            glUniformMatrix4fv(m_defaultShader.getUniformLocation("mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix3fv(m_defaultShader.getUniformLocation("normalModelMatrix"), 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

            if (mesh.hasTextureCoords()) {
                m_texture.bind(GL_TEXTURE0);
                glUniform1i(m_defaultShader.getUniformLocation("colorMap"), 0);
                glUniform1i(m_defaultShader.getUniformLocation("hasTexCoords"), GL_TRUE);
                glUniform1i(m_defaultShader.getUniformLocation("useMaterial"), m_useMaterial);
            }
            else {
                glUniform1i(m_defaultShader.getUniformLocation("hasTexCoords"), GL_FALSE);
                glUniform1i(m_defaultShader.getUniformLocation("useMaterial"), m_useMaterial);
            }
            mesh.draw(m_defaultShader);
        }
    }

private:
    Window m_window;
    Shader m_defaultShader;
    Shader m_shadowShader;

    std::vector<GPUMesh> m_wheel_pair;
    std::vector<GPUMesh> m_tank_body;
    std::vector<GPUMesh> m_gun;
    std::vector<GPUMesh> m_turret;

    Texture m_texture;
    bool m_useMaterial{ true };
    bool driveTank{ false };
    bool animateTurret{ false };
    bool animateGun{ false };
    float tankSpeed{ 0.05f };
    float rotationAngleWheels{ 0.0f };
    float radiusWheels{ 0.05f };
    int turretRotationAngle{ 0 };
    float turretRotationSpeed{ 1.0f };
    float gunTiltAngle{ 0.0f };
    float gunTiltSpeed{ 1.0f };
    int turretDirection{ 1 };
    int gunDirection{ 1 };
    bool turretAngleManuallySet{ false };
    bool gunAngleManuallySet{ false };
    glm::vec3 tankCoords{ 0.0f };

    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 0.1f, 30.0f);
    glm::mat4 m_modelMatrix{ 1.0f };

    Camera m_camera_front;
    Camera m_camera_top;
    Camera m_activeCamera;
};

int main() {
    Application app;
    app.update();
    return 0;
}
