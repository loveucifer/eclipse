#include "physics_system.h"

#include "../components/gameplaycomponents.h"
#include "../components/rendercomponents.h"

#include <box2d/box2d.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace eclipse::systems {
namespace {

constexpr float FixedTimeStep = 1.0f / 60.0f;
constexpr int SubStepCount = 4;

b2BodyType ToBox2DBodyType(components::PhysicsBodyType type) {
  switch (type) {
  case components::PhysicsBodyType::Static:
    return b2_staticBody;
  case components::PhysicsBodyType::Kinematic:
    return b2_kinematicBody;
  case components::PhysicsBodyType::Dynamic:
    return b2_dynamicBody;
  }
  return b2_dynamicBody;
}

void HashCombine(std::size_t& seed, std::size_t value) {
  seed ^= value + 0x9e3779b9u + (seed << 6u) + (seed >> 2u);
}

template <typename T>
void HashValue(std::size_t& seed, const T& value) {
  HashCombine(seed, std::hash<T>{}(value));
}

void HashMaterial(std::size_t& seed,
                  const components::PhysicsMaterial2D& material) {
  HashValue(seed, material.density);
  HashValue(seed, material.friction);
  HashValue(seed, material.restitution);
  HashValue(seed, material.sensor);
  HashValue(seed, material.categoryBits);
  HashValue(seed, material.maskBits);
  HashValue(seed, material.groupIndex);
}

std::size_t ConfigurationHash(ecs::World& world, ecs::Entity entity,
                              const components::RigidBody2D& body) {
  std::size_t seed = 0;
  HashValue(seed, static_cast<int>(body.type));
  HashValue(seed, body.fixedRotation);
  HashValue(seed, body.bullet);
  HashValue(seed, body.enabled);
  if (const auto* box = world.Get<components::BoxCollider2D>(entity)) {
    HashValue(seed, 1);
    HashValue(seed, box->size.x);
    HashValue(seed, box->size.y);
    HashValue(seed, box->offset.x);
    HashValue(seed, box->offset.y);
    HashMaterial(seed, box->material);
  }
  if (const auto* circle = world.Get<components::CircleCollider2D>(entity)) {
    HashValue(seed, 2);
    HashValue(seed, circle->radius);
    HashValue(seed, circle->offset.x);
    HashValue(seed, circle->offset.y);
    HashMaterial(seed, circle->material);
  }
  return seed;
}

b2ShapeDef ShapeDefinition(const components::PhysicsMaterial2D& material,
                           ecs::Entity entity) {
  b2ShapeDef definition = b2DefaultShapeDef();
  definition.userData = reinterpret_cast<void*>(
      static_cast<std::uintptr_t>(entity));
  definition.density = std::max(0.0f, material.density);
  definition.material.friction = std::max(0.0f, material.friction);
  definition.material.restitution = std::clamp(material.restitution, 0.0f, 1.0f);
  definition.isSensor = material.sensor;
  definition.enableSensorEvents = true;
  definition.enableContactEvents = true;
  definition.filter.categoryBits = material.categoryBits;
  definition.filter.maskBits = material.maskBits;
  definition.filter.groupIndex = material.groupIndex;
  return definition;
}

} // namespace

struct PhysicsSystem::Impl {
  struct BodyRecord {
    b2BodyId id = b2_nullBodyId;
    std::size_t configuration = 0;
    glm::vec2 lastPosition{0.0f};
    float lastRotation = 0.0f;
    glm::vec2 lastLinearVelocity{0.0f};
    float lastAngularVelocity = 0.0f;
    std::vector<std::uint64_t> shapeIds;
  };

  b2WorldId world = b2_nullWorldId;
  glm::vec2 gravity{0.0f, 9.8f};
  float pixelsPerMeter = 100.0f;
  float accumulator = 0.0f;
  std::unordered_map<ecs::Entity, BodyRecord> bodies;
  std::unordered_map<std::uint64_t, ecs::Entity> shapes;
  std::vector<PhysicsContactEvent> events;

  Impl() { CreateWorld(); }

  ~Impl() {
    if (b2World_IsValid(world)) {
      b2DestroyWorld(world);
    }
  }

  void CreateWorld() {
    b2WorldDef definition = b2DefaultWorldDef();
    definition.gravity = {gravity.x, gravity.y};
    world = b2CreateWorld(&definition);
  }

  void Reset() {
    if (b2World_IsValid(world)) {
      b2DestroyWorld(world);
    }
    bodies.clear();
    shapes.clear();
    events.clear();
    accumulator = 0.0f;
    CreateWorld();
  }

  void DestroyBody(BodyRecord& record) {
    for (const auto shape : record.shapeIds) {
      shapes.erase(shape);
    }
    if (b2Body_IsValid(record.id)) {
      b2DestroyBody(record.id);
    }
  }

  BodyRecord CreateBody(ecs::World& ecsWorld, ecs::Entity entity,
                        components::Transform& transform,
                        components::RigidBody2D& body) {
    b2BodyDef definition = b2DefaultBodyDef();
    definition.type = ToBox2DBodyType(body.type);
    definition.position = {transform.position.x / pixelsPerMeter,
                           transform.position.y / pixelsPerMeter};
    definition.rotation = b2MakeRot(transform.rotation);
    definition.linearVelocity = {body.linearVelocity.x / pixelsPerMeter,
                                 body.linearVelocity.y / pixelsPerMeter};
    definition.angularVelocity = body.angularVelocity;
    definition.linearDamping = std::max(0.0f, body.linearDamping);
    definition.angularDamping = std::max(0.0f, body.angularDamping);
    definition.gravityScale = body.gravityScale;
    definition.fixedRotation = body.fixedRotation;
    definition.isBullet = body.bullet;
    definition.isEnabled = body.enabled;
    definition.userData = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(entity));

    BodyRecord record;
    record.id = b2CreateBody(world, &definition);
    record.configuration = ConfigurationHash(ecsWorld, entity, body);
    record.lastPosition = transform.position;
    record.lastRotation = transform.rotation;
    record.lastLinearVelocity = body.linearVelocity;
    record.lastAngularVelocity = body.angularVelocity;

    if (const auto* box = ecsWorld.Get<components::BoxCollider2D>(entity)) {
      auto shapeDefinition = ShapeDefinition(box->material, entity);
      const b2Polygon polygon = b2MakeOffsetBox(
          std::max(box->size.x, 0.01f) * 0.5f / pixelsPerMeter,
          std::max(box->size.y, 0.01f) * 0.5f / pixelsPerMeter,
          {box->offset.x / pixelsPerMeter, box->offset.y / pixelsPerMeter},
          b2MakeRot(0.0f));
      const b2ShapeId shape =
          b2CreatePolygonShape(record.id, &shapeDefinition, &polygon);
      const auto stored = b2StoreShapeId(shape);
      record.shapeIds.push_back(stored);
      shapes[stored] = entity;
    }

    if (const auto* circle = ecsWorld.Get<components::CircleCollider2D>(entity)) {
      auto shapeDefinition = ShapeDefinition(circle->material, entity);
      const b2Circle shapeData{{circle->offset.x / pixelsPerMeter,
                                circle->offset.y / pixelsPerMeter},
                               std::max(circle->radius, 0.01f) / pixelsPerMeter};
      const b2ShapeId shape =
          b2CreateCircleShape(record.id, &shapeDefinition, &shapeData);
      const auto stored = b2StoreShapeId(shape);
      record.shapeIds.push_back(stored);
      shapes[stored] = entity;
    }

    return record;
  }

  ecs::Entity EntityFor(b2ShapeId shape) const {
    const auto it = shapes.find(b2StoreShapeId(shape));
    return it == shapes.end() ? ecs::NullEntity : it->second;
  }

  void AddEvent(PhysicsContactType type, b2ShapeId firstShape,
                b2ShapeId secondShape, bool sensor) {
    const auto first = EntityFor(firstShape);
    const auto second = EntityFor(secondShape);
    if (first != ecs::NullEntity && second != ecs::NullEntity) {
      events.push_back({type, first, second, sensor});
    }
  }

  void CollectEvents() {
    const b2ContactEvents contacts = b2World_GetContactEvents(world);
    for (int index = 0; index < contacts.beginCount; ++index) {
      const auto& event = contacts.beginEvents[index];
      AddEvent(PhysicsContactType::Begin, event.shapeIdA, event.shapeIdB,
               false);
    }
    for (int index = 0; index < contacts.endCount; ++index) {
      const auto& event = contacts.endEvents[index];
      AddEvent(PhysicsContactType::End, event.shapeIdA, event.shapeIdB, false);
    }

    const b2SensorEvents sensors = b2World_GetSensorEvents(world);
    for (int index = 0; index < sensors.beginCount; ++index) {
      const auto& event = sensors.beginEvents[index];
      AddEvent(PhysicsContactType::Begin, event.sensorShapeId,
               event.visitorShapeId, true);
    }
    for (int index = 0; index < sensors.endCount; ++index) {
      const auto& event = sensors.endEvents[index];
      AddEvent(PhysicsContactType::End, event.sensorShapeId,
               event.visitorShapeId, true);
    }
  }
};

PhysicsSystem::PhysicsSystem() : mImpl(std::make_unique<Impl>()) {}
PhysicsSystem::~PhysicsSystem() = default;

void PhysicsSystem::Update(ecs::World& world, float deltaTime) {
  mImpl->events.clear();

  for (auto it = mImpl->bodies.begin(); it != mImpl->bodies.end();) {
    if (!world.IsAlive(it->first) ||
        !world.Get<components::RigidBody2D>(it->first) ||
        !world.Get<components::Transform>(it->first)) {
      mImpl->DestroyBody(it->second);
      it = mImpl->bodies.erase(it);
    } else {
      ++it;
    }
  }

  world.ForEach<components::Transform, components::RigidBody2D>(
      [&](ecs::Entity entity, components::Transform& transform,
          components::RigidBody2D& body) {
        const auto configuration = ConfigurationHash(world, entity, body);
        auto record = mImpl->bodies.find(entity);
        if (record != mImpl->bodies.end() &&
            record->second.configuration != configuration) {
          mImpl->DestroyBody(record->second);
          mImpl->bodies.erase(record);
          record = mImpl->bodies.end();
        }
        if (record == mImpl->bodies.end()) {
          record = mImpl->bodies
                       .emplace(entity, mImpl->CreateBody(world, entity,
                                                          transform, body))
                       .first;
        }

        auto& physicsBody = record->second;
        if (transform.position != physicsBody.lastPosition ||
            transform.rotation != physicsBody.lastRotation) {
          b2Body_SetTransform(
              physicsBody.id,
              {transform.position.x / mImpl->pixelsPerMeter,
               transform.position.y / mImpl->pixelsPerMeter},
              b2MakeRot(transform.rotation));
        }
        if (body.linearVelocity != physicsBody.lastLinearVelocity) {
          b2Body_SetLinearVelocity(
              physicsBody.id,
              {body.linearVelocity.x / mImpl->pixelsPerMeter,
               body.linearVelocity.y / mImpl->pixelsPerMeter});
        }
        if (body.angularVelocity != physicsBody.lastAngularVelocity) {
          b2Body_SetAngularVelocity(physicsBody.id, body.angularVelocity);
        }
        b2Body_SetLinearDamping(physicsBody.id,
                                std::max(0.0f, body.linearDamping));
        b2Body_SetAngularDamping(physicsBody.id,
                                 std::max(0.0f, body.angularDamping));
        b2Body_SetGravityScale(physicsBody.id, body.gravityScale);
      });

  mImpl->accumulator += std::clamp(deltaTime, 0.0f, 0.25f);
  while (mImpl->accumulator >= FixedTimeStep) {
    b2World_Step(mImpl->world, FixedTimeStep, SubStepCount);
    mImpl->CollectEvents();
    mImpl->accumulator -= FixedTimeStep;
  }

  world.ForEach<components::Transform, components::RigidBody2D>(
      [&](ecs::Entity entity, components::Transform& transform,
          components::RigidBody2D& body) {
        const auto record = mImpl->bodies.find(entity);
        if (record == mImpl->bodies.end()) {
          return;
        }
        const b2Vec2 position = b2Body_GetPosition(record->second.id);
        const b2Vec2 velocity = b2Body_GetLinearVelocity(record->second.id);
        transform.previousPosition = transform.position;
        transform.position = {position.x * mImpl->pixelsPerMeter,
                              position.y * mImpl->pixelsPerMeter};
        transform.rotation = b2Rot_GetAngle(
            b2Body_GetRotation(record->second.id));
        body.linearVelocity = {velocity.x * mImpl->pixelsPerMeter,
                               velocity.y * mImpl->pixelsPerMeter};
        body.angularVelocity = b2Body_GetAngularVelocity(record->second.id);
        record->second.lastPosition = transform.position;
        record->second.lastRotation = transform.rotation;
        record->second.lastLinearVelocity = body.linearVelocity;
        record->second.lastAngularVelocity = body.angularVelocity;
      });
}

void PhysicsSystem::Reset() { mImpl->Reset(); }

void PhysicsSystem::SetGravity(glm::vec2 gravity) {
  mImpl->gravity = gravity;
  b2World_SetGravity(mImpl->world, {gravity.x, gravity.y});
}

glm::vec2 PhysicsSystem::GetGravity() const { return mImpl->gravity; }

void PhysicsSystem::SetPixelsPerMeter(float pixelsPerMeter) {
  if (pixelsPerMeter > 0.0f && pixelsPerMeter != mImpl->pixelsPerMeter) {
    mImpl->pixelsPerMeter = pixelsPerMeter;
    mImpl->Reset();
  }
}

float PhysicsSystem::GetPixelsPerMeter() const {
  return mImpl->pixelsPerMeter;
}

bool PhysicsSystem::ApplyForce(ecs::Entity entity, glm::vec2 force) {
  const auto it = mImpl->bodies.find(entity);
  if (it == mImpl->bodies.end() || !b2Body_IsValid(it->second.id)) {
    return false;
  }
  b2Body_ApplyForceToCenter(
      it->second.id,
      {force.x / mImpl->pixelsPerMeter, force.y / mImpl->pixelsPerMeter}, true);
  return true;
}

bool PhysicsSystem::ApplyImpulse(ecs::Entity entity, glm::vec2 impulse) {
  const auto it = mImpl->bodies.find(entity);
  if (it == mImpl->bodies.end() || !b2Body_IsValid(it->second.id)) {
    return false;
  }
  b2Body_ApplyLinearImpulseToCenter(
      it->second.id,
      {impulse.x / mImpl->pixelsPerMeter,
       impulse.y / mImpl->pixelsPerMeter}, true);
  return true;
}

const std::vector<PhysicsContactEvent>& PhysicsSystem::GetContactEvents() const {
  return mImpl->events;
}

} // namespace eclipse::systems
