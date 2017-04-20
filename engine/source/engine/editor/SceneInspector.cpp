#include "engine/editor/SceneInspector.hpp"

#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#include "imgui.h"
#include "fw/components/EntityInfo.hpp"
#include "fw/components/Transform.hpp"
#include "fw/rendering/Light.hpp"
#include "fw/rendering/AreaLight.hpp"
#include "fwui/ImGuiExtensions.hpp"
#include "fw/models/RenderMesh.hpp"

namespace ee
{

SceneInspector::SceneInspector(
    fw::VirtualFilesystem& vfs,
    std::shared_ptr<fw::ITextureManagerWithInspection> textureManager,
    std::shared_ptr<fw::StaticModelFactory> staticModelFactory,
    entityx::EntityManager* entityManager
):
    _entityManager{entityManager},
    _vfs{vfs},
    _textureManager{textureManager},
    _staticModelFactory{staticModelFactory},
    _renderMeshEditor{_vfs, _staticModelFactory},
    _materialEditor{_vfs, _textureManager}
{
}

SceneInspector::~SceneInspector()
{
}

void SceneInspector::show()
{
    if (!_inspectorEnabled) { return; }

    if (!ImGui::Begin(
        "Scene inspector",
        &_inspectorEnabled,
        ImGuiWindowFlags_MenuBar
    ))
    {
        ImGui::End();
        return;
    }

    showMenu();
    showEntitiesList();
    showComponentsList();

    ImGui::End();
}

void SceneInspector::showMenu()
{
    const bool isEntitySelected = _selectedEntity.valid();

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Entities"))
        {
            if (ImGui::BeginMenu("New"))
            {
                if (ImGui::MenuItem("Empty entity"))
                {
                    createEmptyEntity();
                }

                if (ImGui::MenuItem("Light"))
                {
                    createLight();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Camera"))
                {
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Delete", nullptr, false, isEntitySelected))
            {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Components", isEntitySelected))
        {
            if (ImGui::MenuItem("Area light"))
            {
                attachAreaLight();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void SceneInspector::showEntitiesList()
{
    ImGui::BeginChild("SceneInspectorLeftPane", ImVec2(150, 0), true);

    auto selectedEntity = _selectedEntity;

    _entityManager->each<fw::EntityInfo>(
        [&selectedEntity](entityx::Entity entity, fw::EntityInfo& info)
        {
            std::stringstream nodeName;
            nodeName << info.getName();
            nodeName << "#" << std::to_string(entity.id().index());

            if (ImGui::Selectable(
                info.getName().c_str(),
                entity == selectedEntity
            ))
            {
                selectedEntity = entity;
            }
        }
    );

    _selectedEntity = selectedEntity;

    ImGui::EndChild();
    ImGui::SameLine();
}

void SceneInspector::showComponentsList()
{
    // todo: this should be more abstract, it is the way it is for simplicity
    if (!_selectedEntity.valid()) { return; }
    ImGui::BeginChild("SceneInspectorRightPane");

    showEntityInfoComponent();
    showTransformComponent();
    showLightComponent();
    showAreaLightComponent();
    showMeshComponent();
    showMaterialComponent();

    ImGui::EndChild();
}

void SceneInspector::showEntityInfoComponent()
{
    if (!_selectedEntity.has_component<fw::EntityInfo>()) { return; }
    auto entityInfo = _selectedEntity.component<fw::EntityInfo>();

    auto entityName = entityInfo->getName();
    if (ImGui::InputText("Name", entityName))
    {
        entityInfo->setName(entityName);
    }
}

void SceneInspector::showTransformComponent()
{
    if (!_selectedEntity.has_component<fw::Transform>()) { return; }
    if (!ImGui::CollapsingHeader("Transform")) { return; }
    auto transform = _selectedEntity.component<fw::Transform>();

    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat orientation;

    glm::decompose(
        transform->getTransform(),
        scale,
        orientation,
        translation,
        skew,
        perspective
    );

    ImGui::DragFloat3("Position", glm::value_ptr(translation), 0.1f);
    ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f, 0.001f, 1000.0f);

    if (ImGui::Button("Reset scale"))
    {
        scale = {1.0f, 1.0f, 1.0f};
    }

    ImGui::SameLine();

    if (ImGui::Button("Make uniform (clone X)"))
    {
        scale = {scale.x, scale.x, scale.x};
    }

    auto newTranslationMtx = glm::translate({}, translation);
    auto newScalingMtx = glm::scale({}, scale);

    transform->setTransform(newTranslationMtx * newScalingMtx);
}

void SceneInspector::showLightComponent()
{
    if (_selectedEntity.has_component<fw::Light>())
    {
        auto light = _selectedEntity.component<fw::Light>();
        if (ImGui::CollapsingHeader("Light"))
        {
            _lightEditor.showEmbeddedFor(*light);
        }
    }
}

void SceneInspector::showAreaLightComponent()
{
    if (_selectedEntity.has_component<fw::AreaLight>())
    {
        auto light = _selectedEntity.component<fw::AreaLight>();
        if (ImGui::CollapsingHeader("Area light"))
        {
            _areaLightEditor.showEmbeddedFor(*light);
        }
    }
}

void SceneInspector::showMeshComponent()
{
    if (_selectedEntity.has_component<fw::RenderMesh>())
    {
        auto renderMesh = _selectedEntity.component<fw::RenderMesh>();
        if (ImGui::CollapsingHeader("Mesh"))
        {
            _renderMeshEditor.showEmbeddedFor(*renderMesh);
        }
    }
}

void SceneInspector::showMaterialComponent()
{
    if (_selectedEntity.has_component<fw::Material>())
    {
        auto material = _selectedEntity.component<fw::Material>();
        if (ImGui::CollapsingHeader("Material"))
        {
            _materialEditor.showEmbeddedFor(*material);
        }
    }
}

void SceneInspector::createEmptyEntity()
{
    auto entity = _entityManager->create();
    entity.assign<fw::EntityInfo>("Empty");
    entity.assign<fw::Transform>();
}

void SceneInspector::createLight()
{
    auto entity = _entityManager->create();
    entity.assign<fw::EntityInfo>("Light");
    entity.assign<fw::Transform>();
    entity.assign<fw::Light>();
}

void SceneInspector::attachAreaLight()
{
    _selectedEntity.assign<fw::AreaLight>();
}

}
