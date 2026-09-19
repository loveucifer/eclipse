#include "render_system.h"

// LEARNING MAP
// Rendering happens in two stages: collect visible sprite data, then submit
// draw commands. Follow Render() to see how Transform becomes a model matrix,
// how layer controls draw order, and how RenderMeshTextured finally draws it.

#include "../components/rendercomponents.h"
#include "../graphics/rendercommands.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <vector>

namespace eclipse::systems {

void RenderSystem::Render(ecs::World& world,
                          managers::RenderManager& renderManager) {
  auto* camera = world.GetFirst<components::Camera>();
  if (!camera) {
    return;
  }

  struct RenderItem {
    int layer;
    std::shared_ptr<graphics::Mesh> mesh;
    std::shared_ptr<graphics::Shader> shader;
    std::shared_ptr<graphics::Texture> texture;
    glm::mat4 model;
    glm::vec2 uvOffset;
    glm::vec2 uvScale;
    glm::vec3 color;
  };
  std::vector<RenderItem> items;

  world.ForEach<components::Transform, components::SpriteRenderer>(
      [&items](ecs::Entity, components::Transform& transform,
               components::SpriteRenderer& sprite) {
        if (!sprite.mesh || !sprite.shader || !sprite.texture) {
          return;
        }

        glm::mat4 model(1.0f);
        model = glm::translate(
            model, glm::vec3(transform.position.x, transform.position.y, 0.0f));
        model = glm::rotate(model, transform.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(
            model, glm::vec3(transform.scale.x, transform.scale.y, 1.0f));

        items.push_back({sprite.layer, sprite.mesh, sprite.shader, sprite.texture,
                         model, sprite.uvOffset, sprite.uvScale,
                         glm::vec3(1.0f)});
      });

  world.ForEach<components::Transform, components::TextRenderer>(
      [&items](ecs::Entity, components::Transform& transform,
               components::TextRenderer& text) {
        if (!text.font || !text.font->IsLoaded() || !text.shader ||
            !text.shader->IsValid()) {
          return;
        }

        glm::mat4 parent(1.0f);
        parent = glm::translate(
            parent, glm::vec3(transform.position.x, transform.position.y, 0.0f));
        parent = glm::rotate(parent, transform.rotation,
                             glm::vec3(0.0f, 0.0f, 1.0f));
        parent = glm::scale(
            parent, glm::vec3(transform.scale * text.scale, 1.0f));

        float penX = 0.0f;
        float penY = 0.0f;
        const auto& space = text.font->GetGlyph(' ');
        for (const unsigned char character : text.text) {
          if (character == '\r') {
            continue;
          }
          if (character == '\n') {
            penX = 0.0f;
            penY += text.font->GetLineHeight();
            continue;
          }
          if (character == '\t') {
            penX += space.advance * 4.0f;
            continue;
          }

          const auto& glyph = text.font->GetGlyph(character);
          if (glyph.size.x > 0.0f && glyph.size.y > 0.0f) {
            const glm::vec2 center{
                penX + glyph.bearing.x + glyph.size.x * 0.5f,
                penY - glyph.bearing.y + glyph.size.y * 0.5f};
            glm::mat4 model = glm::translate(
                parent, glm::vec3(center.x, center.y, 0.0f));
            model = glm::scale(
                model, glm::vec3(glyph.size.x, glyph.size.y, 1.0f));
            items.push_back({text.layer, text.font->GetMesh(), text.shader,
                             text.font->GetAtlas(), model, glyph.uvOffset,
                             glyph.uvScale, text.color});
          }
          penX += glyph.advance;
        }
      });

  std::stable_sort(items.begin(), items.end(),
                   [](const RenderItem& left, const RenderItem& right) {
                     return left.layer < right.layer;
                   });

  for (const auto& item : items) {
    renderManager.Submit(
        std::make_unique<graphics::rendercommands::RenderMeshTextured>(
            item.mesh, item.shader, item.texture, item.model, camera->view,
            camera->projection, item.uvOffset, item.uvScale, item.color));
  }
}

} // namespace eclipse::systems
