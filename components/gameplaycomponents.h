#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <string>

namespace eclipse::components {

struct Name {
  std::string value = "Entity";
  template <class F> void Fields(F visit) { visit("value", value); }
};

struct PlayerController {
  float speed = 250.0f;
};

struct Collider {
  glm::vec2 size{1.0f};
  glm::vec2 offset{0.0f};
  bool isStatic = false;
};

enum class PhysicsBodyType : int {
  Static = 0,
  Kinematic = 1,
  Dynamic = 2,
};

struct RigidBody2D {
  PhysicsBodyType type = PhysicsBodyType::Dynamic;
  glm::vec2 linearVelocity{0.0f};
  float angularVelocity = 0.0f;
  float linearDamping = 0.0f;
  float angularDamping = 0.0f;
  float gravityScale = 1.0f;
  bool fixedRotation = false;
  bool bullet = false;
  bool enabled = true;
};

struct PhysicsMaterial2D {
  float density = 1.0f;
  float friction = 0.6f;
  float restitution = 0.0f;
  bool sensor = false;
  std::uint32_t categoryBits = 1;
  std::uint32_t maskBits = 0xFFFFFFFFu;
  int groupIndex = 0;
};

struct BoxCollider2D {
  glm::vec2 size{1.0f};
  glm::vec2 offset{0.0f};
  PhysicsMaterial2D material;
};

struct CircleCollider2D {
  float radius = 0.5f;
  glm::vec2 offset{0.0f};
  PhysicsMaterial2D material;
};

struct Animation {
  int frameCount = 1;
  int columns = 1;
  int rows = 1;
  float frameDuration = 0.1f;
  float elapsed = 0.0f;
  int currentFrame = 0;
};

struct NpcBehavior {
  glm::vec2 patrolStart{0.0f};
  glm::vec2 patrolEnd{0.0f};
  float speed = 20.0f;
  bool movingToEnd = true;
};

} // namespace eclipse::components
