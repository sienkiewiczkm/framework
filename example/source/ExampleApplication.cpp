#include "ExampleApplication.hpp"

#include <iostream>

#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"
#include "fw/DebugShapes.hpp"
#include "fw/TextureUtils.hpp"
#include "fw/Resources.hpp"

namespace example
{

ExampleApplication::ExampleApplication():
    _enableCameraRotations{false},
    _cameraRotationSensitivity{0.2, 0.2}
{
}

ExampleApplication::~ExampleApplication()
{
}

void ExampleApplication::onCreate()
{
    ImGuiApplication::onCreate();

    _phongEffect = std::make_shared<fw::TexturedPhongEffect>();
    _phongEffect->create();

    _universalPhongEffect = std::make_shared<fw::UniversalPhongEffect>();

    _cube = fw::createBox({1.0, 1.0, 1.0});
    _grid = std::make_shared<fw::Grid>(
        glm::ivec2{32, 32},
        glm::vec2{0.5f, 0.5f}
    );

    _frameMarker = std::make_shared<fw::FrameMarker>();

    _testTexture = fw::loadTextureFromFile(
        fw::getFrameworkResourcePath("textures/checker-base.png")
    );

    updateProjectionMatrix();
}

void ExampleApplication::onDestroy()
{
    ImGuiApplication::onDestroy();
}

void ExampleApplication::onUpdate(
    const std::chrono::high_resolution_clock::duration& deltaTime
)
{
    ImGuiApplication::onUpdate(deltaTime);

    if (ImGui::Begin("Example window"))
    {
        ImGui::Text("This is an example window");
    }
    ImGui::End();
}

void ExampleApplication::onRender()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    _phongEffect->begin();
    _phongEffect->setProjectionMatrix(_projectionMatrix);
    _phongEffect->setViewMatrix(_camera.getViewMatrix());
    _phongEffect->setModelMatrix({});
    _phongEffect->setTexture(_testTexture);
    _cube->render();
    _grid->render();
    _phongEffect->end();

    for (const auto& chunk: _frameMarker->getGeometryChunks())
    {
        _universalPhongEffect->setLightDirection({-1, 1, 0});
        _universalPhongEffect->setSolidColor(
            chunk.getMaterial()->getBaseAlbedoColor()
        );

        _universalPhongEffect->begin();
        // todo: standarize a way to change uniforms
        _universalPhongEffect->setProjectionMatrix(_projectionMatrix);
        _universalPhongEffect->setViewMatrix(_camera.getViewMatrix());
        _universalPhongEffect->setModelMatrix(chunk.getModelMatrix());
        chunk.getMesh()->render();
        _universalPhongEffect->end();
    }

    ImGuiApplication::onRender();
}

bool ExampleApplication::onMouseButton(int button, int action, int mods)
{
    if (ImGuiApplication::onMouseButton(button, action, mods)) { return true; }

    if (GLFW_MOUSE_BUTTON_LEFT == button)
    {
        _enableCameraRotations = GLFW_PRESS == action;
    }

    return true;
}

bool ExampleApplication::onMouseMove(glm::dvec2 newPosition)
{
    if (ImGuiApplication::onMouseMove(newPosition)) { return true; }

    if (_enableCameraRotations)
    {
        auto movement = getMouseMovement() * _cameraRotationSensitivity;
        _camera.rotate(movement.y, movement.x);
    }

    return true;
}

bool ExampleApplication::onScroll(double xoffset, double yoffset)
{
    if (fw::ImGuiApplication::onScroll(xoffset, yoffset))
        return true;

    const double cMinimumDistance = 1.0;
    const double cMaximumDistance = 10.0;
    const double cZoomStep = 0.5;
    auto currentDistance = _camera.getDist();
    _camera.setDist(
        std::min(
            std::max(currentDistance + cZoomStep * yoffset, cMinimumDistance),
            cMaximumDistance
        )
    );

    return true;
}

bool ExampleApplication::onResize()
{
    updateProjectionMatrix();
    return true;
}

void ExampleApplication::updateProjectionMatrix()
{
    auto windowSize = getWindowSize();
    auto aspectRatio = static_cast<float>(windowSize.x) / windowSize.y;
    _projectionMatrix = glm::perspective(45.0f, aspectRatio, 0.5f, 100.0f);
}

}
