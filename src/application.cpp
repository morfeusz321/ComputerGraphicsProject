#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "bezier/bezier.h"
#include "skybox/skybox.h"
#include "cubemap/cubemap.h"

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
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

class Application
{
public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024), OpenGLVersion::GL41), m_texture(RESOURCE_ROOT "resources/checkerboard.png"), m_cubemap(RESOURCE_ROOT "resources/cubemap/"), m_camera_front(&m_window, glm::vec3(18, 3, 22), glm::vec3(-1, 0, -1)), m_camera_top(&m_window, glm::vec3(5, 20, -8), glm::vec3(0, -1, 1)), m_activeCamera(m_camera_front), m_skybox(RESOURCE_ROOT "shaders/"), m_bezierPath({0.0f, 0.0f, 0.0f}, {5.0f, 5.0f, 0.0f}, {10.0f, -5.0f, 0.0f}, {15.0f, 0.0f, 0.0f})
    {
        m_window.registerKeyCallback([this](int key, int scancode, int action, int mods)
                                     {
            if (action == GLFW_PRESS)
                onKeyPressed(key, mods);
            else if (action == GLFW_RELEASE)
                onKeyReleased(key, mods); });
        m_window.registerMouseMoveCallback(std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        m_window.registerMouseButtonCallback([this](int button, int action, int mods)
                                             {
            if (action == GLFW_PRESS)
                onMouseClicked(button, mods);
            else if (action == GLFW_RELEASE)
                onMouseReleased(button, mods); });

        m_wheel_pair = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/wheel.obj");
        m_tank_body = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/tank_body.obj");
        m_gun = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/gun.obj");
        m_turret = GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/turret.obj");

        try
        {
            ShaderBuilder defaultBuilder;
            defaultBuilder.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();

            ShaderBuilder shadowBuilder;
            shadowBuilder.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/shadow_vert.glsl");
            shadowBuilder.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/shadow_frag.glsl");
            m_shadowShader = shadowBuilder.build();

            ShaderBuilder environmentBuilder;
            environmentBuilder.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/environment_vert.glsl");
            environmentBuilder.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/environment_frag.glsl");
            m_environmentShader = environmentBuilder.build();
        }
        catch (ShaderLoadingException e)
        {
            std::cerr << e.what() << std::endl;
        }

        resetSimulation();
    }

    void update()
    {
        const float deltaTime = 0.016f; // Assuming a fixed timestep of 16ms

        while (!m_window.shouldClose())
        {
            m_window.updateInput();
            m_activeCamera.updateInput();

            ImGui::SetNextWindowSize(ImVec2(800, 1000), ImGuiCond_FirstUseEver);
            ImGui::Begin("User Controls");
            if (ImGui::Button("Reset Simulation"))
            {
                resetSimulation();
            }
            ImGui::Separator();
            ImGui::Text("Material Properties");
            ImGui::Checkbox("Use material if no texture", &m_useMaterial);
            ImGui::Checkbox("Use Diffuse (Kd)", &useKd);
            if (useKd)
            {
                ImGui::ColorEdit3("Diffuse Color (Kd)", glm::value_ptr(kd));
            }
            ImGui::Checkbox("Use Specular (Ks)", &useKs);
            if (useKs)
            {
                ImGui::ColorEdit3("Specular Color (Ks)", glm::value_ptr(ks));
            }
            ImGui::Checkbox("Use Shininess", &useShininess);
            if (useShininess)
            {
                ImGui::SliderFloat("Shininess", &shininess, 1.0f, 128.0f);
            }
            ImGui::Checkbox("Use Roughness", &useRoughness);
            if (useRoughness)
            {
                ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
            }

            ImGui::Separator();
            ImGui::Text("Tank Controls");

            ImGui::Checkbox("Drive Tank", &driveTank);
            if (driveTank)
            {
                ImGui::SliderFloat("Speed Tank", &tankSpeedAlongCurve, 0.01f, 0.3f, "%.3f");
            }

            ImGui::Separator();
            ImGui::Text("Turret Controls");
            if (!animateTurret)
            {
                ImGui::SliderInt("Angle Turret", &turretRotationAngle, -30, 30);
            }
            else
            {
                ImGui::SliderFloat("Speed Turret", &turretRotationSpeed, 1.0f, 10.0f);
            }
            ImGui::Checkbox("Animate Turret Movement", &animateTurret);

            ImGui::Separator();
            ImGui::Text("Bazooka Controls");
            if (!animateGun)
            {
                ImGui::SliderFloat("Angle Bazooka", &gunTiltAngle, -20.0f, 20.0f);
            }
            else
            {
                ImGui::SliderFloat("Speed Bazooka", &gunTiltSpeed, 0.5f, 5.0f);
            }
            ImGui::Checkbox("Animate Bazooka Tilt", &animateGun);
            ImGui::Separator();
            ImGui::Text("Environment Mapping");
            ImGui::Checkbox("Use Environment Mapping", &m_useEnvironmentMapping);
            if (m_useEnvironmentMapping)
            {
                ImGui::SliderFloat("Reflectivity", &m_reflectivity, 0.0f, 1.0f);
            }
            ImGui::End();

            if (animateTurret && !turretAngleManuallySet)
            {
                animateTurretRotation();
            }

            if (animateGun && !gunAngleManuallySet)
            {
                animateGunTilt();
            }

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            if (driveTank) {
                currentT += tankSpeedAlongCurve * deltaTime;
                if (currentT > 1.0f)
                    currentT = 0.0f;

                tankCoords = m_bezierPath.calculatePosition(currentT);
                float distanceTraveled = tankSpeedAlongCurve * deltaTime * glm::distance(m_bezierPath.getP0(), m_bezierPath.getP3());
                rotationAngleWheels += (distanceTraveled / radiusWheels);
            }


            renderTank();
            m_window.swapBuffers();
        }
    }

    void animateTurretRotation()
    {
        turretRotationAngle += turretDirection * turretRotationSpeed;
        if (turretRotationAngle >= 30)
        {
            turretDirection = -1;
        }
        else if (turretRotationAngle <= -30)
        {
            turretDirection = 1;
        }
    }

    void animateGunTilt()
    {
        gunTiltAngle += gunDirection * gunTiltSpeed;
        if (gunTiltAngle >= 20.0f)
        {
            gunDirection = -1;
        }
        else if (gunTiltAngle <= -20.0f)
        {
            gunDirection = 1;
        }
    }

    glm::vec3 calculateRectanglePath(float t)
    {
        float segmentT = fmod(t * 4.0f, 1.0f); // normalized within each segment
        int segment = int(t * 4.0f);

        switch (segment)
        {
        case 0:
            return glm::mix(rectP0, rectP1, segmentT); // bottom edge
        case 1:
            return glm::mix(rectP1, rectP2, segmentT); // right edge
        case 2:
            return glm::mix(rectP2, rectP3, segmentT); // top edge
        case 3:
            return glm::mix(rectP3, rectP0, segmentT); // left edge back to origin
        default:
            return rectP0;
        }
    }

    // overrides the above for smooth transitions around the edges (look in both sides since its an armour tank)
    glm::vec3 calculateRectanglePath(float t, glm::vec3 &outDirection)
    {
        float segmentT = fmod(t * 4.0f, 1.0f); // normalized within each segment
        int segment = int(t * 4.0f);

        // for smooth transitions
        segmentT = glm::smoothstep(0.1f, 0.9f, segmentT);

        glm::vec3 currentPos, nextPos;

        switch (segment)
        {
        case 0:
            currentPos = glm::mix(rectP0, rectP1, segmentT);
            nextPos = glm::mix(rectP0, rectP1, fmod(segmentT + 0.01f, 1.0f));
            break;
        case 1:
            currentPos = glm::mix(rectP1, rectP2, segmentT);
            nextPos = glm::mix(rectP1, rectP2, fmod(segmentT + 0.01f, 1.0f));
            break;
        case 2:
            currentPos = glm::mix(rectP2, rectP3, segmentT);
            nextPos = glm::mix(rectP2, rectP3, fmod(segmentT + 0.01f, 1.0f));
            break;
        case 3:
            currentPos = glm::mix(rectP3, rectP0, segmentT);
            nextPos = glm::mix(rectP3, rectP0, fmod(segmentT + 0.01f, 1.0f));
            break;
        default:
            currentPos = rectP0;
            nextPos = rectP1;
            break;
        }
        outDirection = glm::normalize(nextPos - currentPos);

        return currentPos;
    }

    void renderTank()
    {
        m_skybox.render(m_projectionMatrix, m_activeCamera.viewMatrix(), m_cubemap);
        glm::vec3 tankDirection;
        tankCoords = calculateRectanglePath(currentT, tankDirection);

        // target angle using atan2 and normalize it within [-pi, pi]
        float targetAngle = atan2(-tankDirection.z, tankDirection.x);

        // Smoothly interpolate the current angle to the target angle with short-angle interpolation
        static float currentAngle = targetAngle;
        float deltaAngle = targetAngle - currentAngle;

        if (deltaAngle > 0.5 * glm::pi<float>())
        {
            deltaAngle -= 2.0f * glm::pi<float>();
        }
        else if (deltaAngle < 0.5 * -glm::pi<float>())
        {
            deltaAngle += 2.0f * glm::pi<float>();
        }
        currentAngle += deltaAngle * 0.1f; // 0.1f to control the smoothness of the rotation

        // initial rotation to adjust for the model's -z forward direction in Blender
        glm::mat4 initialAlignment = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // dynamic rotation to align with the smoothly interpolated angle
        glm::mat4 dynamicRotation = glm::rotate(glm::mat4(1.0f), currentAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 tankBodyMatrix = glm::translate(glm::mat4(1.0f), tankCoords) * dynamicRotation * initialAlignment;
        renderMesh(m_tank_body, tankBodyMatrix);

        std::vector<glm::vec3> wheel_offsets = {{-0.7f, -1.6f, -3.5f}, {-0.7f, -1.6f, -0.5f}, {-0.7f, -1.6f, 2.5f}};

        for (const glm::vec3 &offset : wheel_offsets)
        {
            glm::mat4 wheelPositionMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(offset.x, offset.y, offset.z));
            glm::mat4 wheelRotationMatrix = glm::rotate(wheelPositionMatrix, glm::radians(rotationAngleWheels), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 finalWheelMatrix = tankBodyMatrix * wheelRotationMatrix;
            renderMesh(m_wheel_pair, finalWheelMatrix);
        }
        glm::mat4 turretRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians((float)turretRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 turretModelMatrix = tankBodyMatrix * turretRotationMatrix;
        renderMesh(m_turret, turretModelMatrix);

        glm::mat4 gunModelMatrix = glm::rotate(turretModelMatrix, glm::radians(gunTiltAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        renderMesh(m_gun, gunModelMatrix);
    }

    void resetSimulation()
    {
        tankCoords = glm::vec3(0.0f);
        currentT = 0.0f;
        driveTank = false;
        tankSpeedAlongCurve = 0.01f;
        rotationAngleWheels = 0.0f;
        turretRotationAngle = 0.0f;
        turretRotationSpeed = 1.0f;
        gunTiltAngle = 0.0f;
        gunTiltSpeed = 1.0f;
        turretDirection = 1;
        gunDirection = 1;
        turretAngleManuallySet = false;
        gunAngleManuallySet = false;
        animateTurret = false;
        animateGun = false;
    }

    void onKeyPressed(int key, int mods)
    {
        if (key == GLFW_KEY_1)
        {
            m_activeCamera = m_camera_front;
        }
        else if (key == GLFW_KEY_2)
        {
            m_activeCamera = m_camera_top;
        }
    }

    void onKeyReleased(int key, int mods)
    {
        std::cout << "Key released: " << key << std::endl;
    }

    void onMouseMove(const glm::dvec2 &cursorPos)
    {
        std::cout << "Mouse at position: " << cursorPos.x << " " << cursorPos.y << std::endl;
    }

    void onMouseClicked(int button, int mods)
    {
        std::cout << "Pressed mouse button: " << button << std::endl;
    }

    void onMouseReleased(int button, int mods)
    {
        std::cout << "Released mouse button: " << button << std::endl;
    }

    void renderMesh(std::vector<GPUMesh> &meshes, const glm::mat4 &modelMatrix)
    {
        const glm::mat4 mvpMatrix = m_projectionMatrix * m_activeCamera.viewMatrix() * modelMatrix;
        const glm::mat3 normalModelMatrix = glm::inverseTranspose(glm::mat3(modelMatrix));

        glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

        for (GPUMesh &mesh : meshes)
        {
            m_environmentShader.bind();

            // Set matrices
            glUniformMatrix4fv(m_environmentShader.getUniformLocation("mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniformMatrix4fv(m_environmentShader.getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix3fv(m_environmentShader.getUniformLocation("normalModelMatrix"), 1, GL_FALSE, glm::value_ptr(normalModelMatrix));

            // Set lighting uniforms
            glUniform3fv(m_environmentShader.getUniformLocation("lightPos"), 1, glm::value_ptr(lightPos));
            glUniform3fv(m_environmentShader.getUniformLocation("lightColor"), 1, glm::value_ptr(lightColor));
            glUniform3fv(m_environmentShader.getUniformLocation("viewPos"), 1, glm::value_ptr(m_activeCamera.cameraPos()));

            // Set material properties
            glUniform1i(m_environmentShader.getUniformLocation("useKd"), useKd);
            if (useKd)
                glUniform3fv(m_environmentShader.getUniformLocation("kd"), 1, glm::value_ptr(kd));

            glUniform1i(m_environmentShader.getUniformLocation("useKs"), useKs);
            if (useKs)
                glUniform3fv(m_environmentShader.getUniformLocation("ks"), 1, glm::value_ptr(ks));

            glUniform1i(m_environmentShader.getUniformLocation("useShininess"), useShininess);
            if (useShininess)
                glUniform1f(m_environmentShader.getUniformLocation("shininess"), shininess);

            glUniform1i(m_environmentShader.getUniformLocation("useEnvironmentMapping"), m_useEnvironmentMapping);
            glUniform1f(m_environmentShader.getUniformLocation("reflectivity"), m_reflectivity);

            // Bind textures
            if (mesh.hasTextureCoords())
            {
                m_texture.bind(GL_TEXTURE0);
                glUniform1i(m_environmentShader.getUniformLocation("colorMap"), 0);
                glUniform1i(m_environmentShader.getUniformLocation("hasTexCoords"), GL_TRUE);
            }
            else
            {
                glUniform1i(m_environmentShader.getUniformLocation("hasTexCoords"), GL_FALSE);
            }
            m_texture.bind(GL_TEXTURE0);
            glUniform1i(m_environmentShader.getUniformLocation("colorMap"), 0);
            glUniform1i(m_environmentShader.getUniformLocation("hasTexCoords"), GL_TRUE);
            // Bind cubemap
            m_cubemap.bind(GL_TEXTURE1);
            glUniform1i(m_environmentShader.getUniformLocation("cubemap"), 1);

            // Render the mesh
            mesh.draw(m_environmentShader);
        }
    }

private:
    Window m_window;
    Shader m_defaultShader;
    Shader m_shadowShader;

    // EnvMapping
    CubeMap m_cubemap;
    Skybox m_skybox;
    Shader m_environmentShader;
    bool m_useEnvironmentMapping{true};
    float m_reflectivity{0.5f};

    std::vector<GPUMesh> m_wheel_pair;
    std::vector<GPUMesh> m_tank_body;
    std::vector<GPUMesh> m_gun;
    std::vector<GPUMesh> m_turret;

    Texture m_texture;
    BezierPath m_bezierPath;
    bool m_useMaterial{true};
    bool driveTank{false};
    bool animateTurret{false};
    bool animateGun{false};
    float tankSpeedAlongCurve{0.01f};
    float rotationAngleWheels{0.0f};
    float radiusWheels{0.02f};
    int turretRotationAngle{0};
    float turretRotationSpeed{1.0f};
    float gunTiltAngle{0.0f};
    float gunTiltSpeed{1.0f};
    int turretDirection{1};
    int gunDirection{1};
    bool turretAngleManuallySet{false};
    bool gunAngleManuallySet{false};
    glm::vec3 tankCoords{0.0f};
    float currentT = 0.0f;
    bool userOverrideKd{false};
    bool userOverrideKs{false};

    // Rectangle control points
    glm::vec3 rectP0 = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rectP1 = glm::vec3(10.0f, 0.0f, 0.0f);
    glm::vec3 rectP2 = glm::vec3(10.0f, 0.0f, 10.0f);
    glm::vec3 rectP3 = glm::vec3(0.0f, 0.0f, 10.0f);

    // Material properties
    glm::vec3 kd{0.8f, 0.8f, 0.8f}; // Diffuse color
    glm::vec3 ks{0.1, 0.1f, 0.1f};  // Specular color
    float shininess{0.0f};          // Shininess for specular reflection
    float roughness{0.5f};          // Roughness for surface roughness

    // Toggles for material properties
    bool useKd{true};
    bool useKs{true};
    bool useShininess{true};
    bool useRoughness{true};

    glm::mat4 m_projectionMatrix = glm::perspective(glm::radians(80.0f), 1.0f, 0.1f, 30.0f);
    glm::mat4 m_modelMatrix{1.0f};

    Camera m_camera_front;
    Camera m_camera_top;
    Camera m_activeCamera;
};

int main()
{
    Application app;
    app.update();
    return 0;
}