#pragma once

#include "../graphics/mesh.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"
#include "../graphics/font.h"
#include "../ecs/entity.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace eclipse::components {

struct Transform {
  glm::vec2 position{0.0f};
  glm::vec2 previousPosition{0.0f};
  float rotation = 0.0f;
  glm::vec2 scale{1.0f};
};

struct SpriteRenderer {
  std::shared_ptr<graphics::Mesh> mesh;
  std::shared_ptr<graphics::Shader> shader;
  std::shared_ptr<graphics::Texture> texture;
  glm::vec2 uvOffset{0.0f};
  glm::vec2 uvScale{1.0f};
  int layer = 0;
};

struct TextRenderer {
  std::shared_ptr<graphics::Font> font;
  std::shared_ptr<graphics::Shader> shader;
  std::string text;
  glm::vec3 color{1.0f};
  float scale = 1.0f;
  int layer = 0;
};

struct Camera {
  glm::mat4 view{1.0f};
  glm::mat4 projection{1.0f};
  float zoom = 1.0f;
  glm::vec2 fitSize{0.0f};
  float fitScale = 1.0f;
  ecs::Entity followTarget = ecs::NullEntity;
};

} // namespace eclipse::components
