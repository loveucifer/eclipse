#pragma once

#include "../ecs/world.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace eclipse::systems {

enum class PhysicsContactType {
  Begin,
  End,
};

struct PhysicsContactEvent {
  PhysicsContactType type = PhysicsContactType::Begin;
  ecs::Entity first = ecs::NullEntity;
  ecs::Entity second = ecs::NullEntity;
  bool sensor = false;
};

class PhysicsSystem {
public:
  PhysicsSystem();
  ~PhysicsSystem();

  PhysicsSystem(const PhysicsSystem&) = delete;
  PhysicsSystem& operator=(const PhysicsSystem&) = delete;

  void Update(ecs::World& world, float deltaTime);
  void Reset();

  void SetGravity(glm::vec2 gravity);
  glm::vec2 GetGravity() const;
  void SetPixelsPerMeter(float pixelsPerMeter);
  float GetPixelsPerMeter() const;

  bool ApplyForce(ecs::Entity entity, glm::vec2 force);
  bool ApplyImpulse(ecs::Entity entity, glm::vec2 impulse);

  const std::vector<PhysicsContactEvent>& GetContactEvents() const;

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

} // namespace eclipse::systems
