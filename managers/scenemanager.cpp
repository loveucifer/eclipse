#include "scenemanager.h"

#include "../components/gameplaycomponents.h"
#include "../components/rendercomponents.h"
#include "../core/jsondata.h"
#include "../external/picojson.h"
#include "../graphics/mesh.h"
#include "../src/log.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <utility>

namespace {

using JsonArray = picojson::array;
using JsonObject = picojson::object;

picojson::value Vec2(glm::vec2 value) {
  JsonArray array;
  array.emplace_back(static_cast<double>(value.x));
  array.emplace_back(static_cast<double>(value.y));
  return picojson::value(array);
}

picojson::value Vec3(glm::vec3 value) {
  return picojson::value(JsonArray{picojson::value(value.x),
                                   picojson::value(value.y),
                                   picojson::value(value.z)});
}

const picojson::value* FindMember(const JsonObject& object,
                                  const std::string& name) {
  const auto it = object.find(name);
  return it == object.end() ? nullptr : &it->second;
}

bool ReadObject(const JsonObject& object, const std::string& name,
                const JsonObject*& result) {
  const auto* value = FindMember(object, name);
  if (!value || !value->is<JsonObject>()) {
    return false;
  }
  result = &value->get<JsonObject>();
  return true;
}

bool ReadString(const JsonObject& object, const std::string& name,
                std::string& result) {
  const auto* value = FindMember(object, name);
  if (!value || !value->is<std::string>()) {
    return false;
  }
  result = value->get<std::string>();
  return true;
}

bool ReadNumber(const JsonObject& object, const std::string& name,
                double& result) {
  const auto* value = FindMember(object, name);
  if (!value || !value->is<double>() || !std::isfinite(value->get<double>())) {
    return false;
  }
  result = value->get<double>();
  return true;
}

bool ReadFloat(const JsonObject& object, const std::string& name,
               float& result) {
  double value = 0.0;
  if (!ReadNumber(object, name, value) ||
      value < -std::numeric_limits<float>::max() ||
      value > std::numeric_limits<float>::max()) {
    return false;
  }
  result = static_cast<float>(value);
  return true;
}

bool ReadInt(const JsonObject& object, const std::string& name, int& result) {
  double value = 0.0;
  if (!ReadNumber(object, name, value) || std::floor(value) != value ||
      value < std::numeric_limits<int>::min() ||
      value > std::numeric_limits<int>::max()) {
    return false;
  }
  result = static_cast<int>(value);
  return true;
}

bool ReadUint32(const JsonObject& object, const std::string& name,
                std::uint32_t& result) {
  double value = 0.0;
  if (!ReadNumber(object, name, value) || std::floor(value) != value ||
      value < 0.0 || value > std::numeric_limits<std::uint32_t>::max()) {
    return false;
  }
  result = static_cast<std::uint32_t>(value);
  return true;
}

bool ReadEntity(const JsonObject& object, const std::string& name,
                eclipse::ecs::Entity& result) {
  double value = 0.0;
  if (!ReadNumber(object, name, value) || std::floor(value) != value ||
      value < 0.0 ||
      value > std::numeric_limits<eclipse::ecs::Entity>::max()) {
    return false;
  }
  result = static_cast<eclipse::ecs::Entity>(value);
  return true;
}

bool ReadBool(const JsonObject& object, const std::string& name,
              bool& result) {
  const auto* value = FindMember(object, name);
  if (!value || !value->is<bool>()) {
    return false;
  }
  result = value->get<bool>();
  return true;
}

bool ReadVec2(const JsonObject& object, const std::string& name,
              glm::vec2& result) {
  const auto* value = FindMember(object, name);
  if (!value || !value->is<JsonArray>()) {
    return false;
  }
  const auto& array = value->get<JsonArray>();
  if (array.size() != 2 || !array[0].is<double>() ||
      !array[1].is<double>() || !std::isfinite(array[0].get<double>()) ||
      !std::isfinite(array[1].get<double>())) {
    return false;
  }
  const double x = array[0].get<double>();
  const double y = array[1].get<double>();
  if (x < -std::numeric_limits<float>::max() ||
      x > std::numeric_limits<float>::max() ||
      y < -std::numeric_limits<float>::max() ||
      y > std::numeric_limits<float>::max()) {
    return false;
  }
  result = {static_cast<float>(x), static_cast<float>(y)};
  return true;
}

bool ReadVec3(const JsonObject& object, const std::string& name,
              glm::vec3& result) {
  const auto* value = FindMember(object, name);
  if (!value || !value->is<JsonArray>()) {
    return false;
  }
  const auto& array = value->get<JsonArray>();
  if (array.size() != 3 || !array[0].is<double>() ||
      !array[1].is<double>() || !array[2].is<double>()) {
    return false;
  }
  for (const auto& entry : array) {
    if (!std::isfinite(entry.get<double>()) ||
        std::abs(entry.get<double>()) > std::numeric_limits<float>::max()) {
      return false;
    }
  }
  result = {static_cast<float>(array[0].get<double>()),
            static_cast<float>(array[1].get<double>()),
            static_cast<float>(array[2].get<double>())};
  return true;
}

std::shared_ptr<eclipse::graphics::Mesh> CreateSpriteMesh() {
  float vertices[]{0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
                   -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f};
  std::uint32_t elements[]{0, 3, 1, 1, 3, 2};
  float texcoords[]{1.0f, 1.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f};
  return std::make_shared<eclipse::graphics::Mesh>(
      vertices, 4, 3, texcoords, elements, 6);
}

picojson::value PhysicsMaterial(
    const eclipse::components::PhysicsMaterial2D& material) {
  JsonObject value;
  value["density"] = picojson::value(material.density);
  value["friction"] = picojson::value(material.friction);
  value["restitution"] = picojson::value(material.restitution);
  value["sensor"] = picojson::value(material.sensor);
  value["categoryBits"] = picojson::value(
      static_cast<double>(material.categoryBits));
  value["maskBits"] = picojson::value(
      static_cast<double>(material.maskBits));
  value["groupIndex"] = picojson::value(
      static_cast<double>(material.groupIndex));
  return picojson::value(value);
}

bool ReadPhysicsMaterial(const JsonObject& parent, const std::string& name,
                         eclipse::components::PhysicsMaterial2D& material) {
  const JsonObject* value = nullptr;
  if (!ReadObject(parent, name, value) ||
      !ReadFloat(*value, "density", material.density) ||
      !ReadFloat(*value, "friction", material.friction) ||
      !ReadFloat(*value, "restitution", material.restitution) ||
      !ReadBool(*value, "sensor", material.sensor) ||
      !ReadUint32(*value, "categoryBits", material.categoryBits) ||
      !ReadUint32(*value, "maskBits", material.maskBits) ||
      !ReadInt(*value, "groupIndex", material.groupIndex)) {
    return false;
  }
  return material.density >= 0.0f && material.friction >= 0.0f &&
         material.restitution >= 0.0f && material.restitution <= 1.0f;
}

} // namespace

namespace eclipse::managers {

bool SceneManager::KnownComponent(const std::string& name) const {
  bool known = name == "transform" || name == "spriteRenderer" ||
      name == "camera" || name == "playerController" || name == "collider" ||
      name == "animation" || name == "npcBehavior" || name == "name" ||
      name == "textRenderer" || name == "rigidBody2D" ||
      name == "boxCollider2D" || name == "circleCollider2D";
  for (const auto& codec : mCodecs) known = known || codec.name == name;
  return known;
}

void SceneManager::Register(const std::string& name, SceneLoader loader) {
  mLoaders[name] = std::move(loader);
}

bool SceneManager::Load(const std::string& name, ecs::World& world,
                        AssetManager& assets) {
  const auto it = mLoaders.find(name);
  if (it == mLoaders.end() || !it->second) {
    mLastError = "Unknown scene: " + name;
    ECLIPSE_ERROR("Unable to load unknown scene '{}'", name);
    return false;
  }

  world.Clear();
  if (!mCurrent.empty()) {
    assets.UnloadGroup(mCurrent);
  }
  it->second(world, assets);
  mCurrent = name;
  mLastError.clear();
  ECLIPSE_INFO("Loaded scene '{}'", name);
  return true;
}

bool SceneManager::SaveFile(const std::filesystem::path& path,
                            ecs::World& world, const AssetManager& assets) {
  picojson::value document;
  if (!ToJson(world, assets, document)) return false;
  try { core::json::Write(path, document); return true; }
  catch (const std::exception& e) { mLastError = e.what(); return false; }
}

bool SceneManager::ToJson(ecs::World& world, const AssetManager& assets,
                          picojson::value& result, ecs::Entity onlyEntity) {
  mLastError.clear();
  if (onlyEntity && !world.IsAlive(onlyEntity)) {
    mLastError = "Cannot serialize a missing entity";
    return false;
  }

  JsonObject root;
  root["version"] = picojson::value(1.0);
  JsonArray entities;
  auto entityIds = world.GetEntities();
  std::sort(entityIds.begin(), entityIds.end());
  std::vector<std::type_index> knownTypes{typeid(components::Transform), typeid(components::SpriteRenderer),
    typeid(components::Camera), typeid(components::PlayerController), typeid(components::Collider),
    typeid(components::Animation), typeid(components::NpcBehavior), typeid(components::Name),
    typeid(components::TextRenderer), typeid(components::RigidBody2D),
    typeid(components::BoxCollider2D), typeid(components::CircleCollider2D)};
  for (const auto& codec : mCodecs) knownTypes.push_back(codec.type);

  for (const auto entity : entityIds) {
    if (!world.IsAlive(entity) || (onlyEntity && onlyEntity != entity)) {
      continue;
    }
    JsonObject entry;
    for (const auto& type : world.ComponentTypes(entity)) {
      if (std::find(knownTypes.begin(), knownTypes.end(), type) == knownTypes.end()) {
        mLastError = "Entity " + std::to_string(entity) + " has an unregistered component: " + type.name();
        return false;
      }
    }
    entry["id"] = picojson::value(static_cast<double>(entity));
    JsonObject components;

    if (const auto* transform = world.Get<components::Transform>(entity)) {
      JsonObject value;
      value["position"] = Vec2(transform->position);
      value["previousPosition"] = Vec2(transform->previousPosition);
      value["rotation"] = picojson::value(transform->rotation);
      value["scale"] = Vec2(transform->scale);
      components["transform"] = picojson::value(value);
    }
    if (const auto* sprite = world.Get<components::SpriteRenderer>(entity)) {
      const std::string textureId = assets.FindTextureId(sprite->texture);
      const std::string shaderId = assets.FindShaderId(sprite->shader);
      if (textureId.empty() || shaderId.empty()) {
        mLastError = "Entity " + std::to_string(entity) +
                     " uses a sprite without manifest asset IDs";
        return false;
      }
      JsonObject value;
      value["texture"] = picojson::value(textureId);
      value["shader"] = picojson::value(shaderId);
      value["uvOffset"] = Vec2(sprite->uvOffset);
      value["uvScale"] = Vec2(sprite->uvScale);
      value["layer"] = picojson::value(static_cast<double>(sprite->layer));
      components["spriteRenderer"] = picojson::value(value);
    }
    if (const auto* text = world.Get<components::TextRenderer>(entity)) {
      const std::string fontId = assets.FindFontId(text->font);
      const std::string shaderId = assets.FindShaderId(text->shader);
      if (fontId.empty() || shaderId.empty()) {
        mLastError = "Entity " + std::to_string(entity) +
                     " uses text without manifest font and shader IDs";
        return false;
      }
      JsonObject value;
      value["font"] = picojson::value(fontId);
      value["pixelHeight"] = picojson::value(
          static_cast<double>(text->font->GetPixelHeight()));
      value["shader"] = picojson::value(shaderId);
      value["text"] = picojson::value(text->text);
      value["color"] = Vec3(text->color);
      value["scale"] = picojson::value(text->scale);
      value["layer"] = picojson::value(static_cast<double>(text->layer));
      components["textRenderer"] = picojson::value(value);
    }
    if (const auto* camera = world.Get<components::Camera>(entity)) {
      JsonObject value;
      value["zoom"] = picojson::value(camera->zoom);
      value["fitSize"] = Vec2(camera->fitSize);
      value["fitScale"] = picojson::value(camera->fitScale);
      value["followTarget"] =
          picojson::value(static_cast<double>(onlyEntity ? ecs::NullEntity : camera->followTarget));
      components["camera"] = picojson::value(value);
    }
    if (const auto* controller = world.Get<components::PlayerController>(entity)) {
      JsonObject value;
      value["speed"] = picojson::value(controller->speed);
      components["playerController"] = picojson::value(value);
    }
    if (const auto* collider = world.Get<components::Collider>(entity)) {
      JsonObject value;
      value["size"] = Vec2(collider->size);
      value["offset"] = Vec2(collider->offset);
      value["isStatic"] = picojson::value(collider->isStatic);
      components["collider"] = picojson::value(value);
    }
    if (const auto* body = world.Get<components::RigidBody2D>(entity)) {
      JsonObject value;
      value["type"] = picojson::value(static_cast<double>(body->type));
      value["linearVelocity"] = Vec2(body->linearVelocity);
      value["angularVelocity"] = picojson::value(body->angularVelocity);
      value["linearDamping"] = picojson::value(body->linearDamping);
      value["angularDamping"] = picojson::value(body->angularDamping);
      value["gravityScale"] = picojson::value(body->gravityScale);
      value["fixedRotation"] = picojson::value(body->fixedRotation);
      value["bullet"] = picojson::value(body->bullet);
      value["enabled"] = picojson::value(body->enabled);
      components["rigidBody2D"] = picojson::value(value);
    }
    if (const auto* box = world.Get<components::BoxCollider2D>(entity)) {
      JsonObject value;
      value["size"] = Vec2(box->size);
      value["offset"] = Vec2(box->offset);
      value["material"] = PhysicsMaterial(box->material);
      components["boxCollider2D"] = picojson::value(value);
    }
    if (const auto* circle = world.Get<components::CircleCollider2D>(entity)) {
      JsonObject value;
      value["radius"] = picojson::value(circle->radius);
      value["offset"] = Vec2(circle->offset);
      value["material"] = PhysicsMaterial(circle->material);
      components["circleCollider2D"] = picojson::value(value);
    }
    if (const auto* animation = world.Get<components::Animation>(entity)) {
      JsonObject value;
      value["frameCount"] = picojson::value(static_cast<double>(animation->frameCount));
      value["columns"] = picojson::value(static_cast<double>(animation->columns));
      value["rows"] = picojson::value(static_cast<double>(animation->rows));
      value["frameDuration"] = picojson::value(animation->frameDuration);
      value["elapsed"] = picojson::value(animation->elapsed);
      value["currentFrame"] = picojson::value(static_cast<double>(animation->currentFrame));
      components["animation"] = picojson::value(value);
    }
    if (const auto* npc = world.Get<components::NpcBehavior>(entity)) {
      JsonObject value;
      value["patrolStart"] = Vec2(npc->patrolStart);
      value["patrolEnd"] = Vec2(npc->patrolEnd);
      value["speed"] = picojson::value(npc->speed);
      value["movingToEnd"] = picojson::value(npc->movingToEnd);
      components["npcBehavior"] = picojson::value(value);
    }
    if (const auto* name = world.Get<components::Name>(entity)) {
      components["name"] = core::json::Encode(*name);
    }

    try {
      for (const auto& codec : mCodecs) {
        picojson::value value;
        if (codec.write(world, entity, value)) components[codec.name] = value;
      }
    } catch (const std::exception& e) { mLastError = e.what(); return false; }
    entry["components"] = picojson::value(components);
    entities.emplace_back(entry);
  }
  root["entities"] = picojson::value(entities);

  result = picojson::value(root);
  try { core::json::ValidateNumbers(result); }
  catch (const std::exception& e) { mLastError = e.what(); return false; }
  return true;
}

bool SceneManager::LoadFile(const std::filesystem::path& path,
                            ecs::World& world, AssetManager& assets) {
  try { return FromJson(core::json::Read(path), world, assets); }
  catch (const std::exception& e) { mLastError = e.what(); return false; }
}

bool SceneManager::FromJson(const picojson::value& rootValue, ecs::World& world,
                            AssetManager& assets) {
  mLastError.clear();
  const auto fail = [this](const std::string& message) {
    mLastError = message;
    ECLIPSE_ERROR("Unable to load scene: {}", message);
    return false;
  };
  if (!rootValue.is<JsonObject>()) {
    return fail("Scene root must be an object");
  }
  const auto& root = rootValue.get<JsonObject>();
  double version = 0.0;
  if (!ReadNumber(root, "version", version) || version != 1.0) {
    return fail("Scene version must be 1");
  }
  const auto* entitiesValue = FindMember(root, "entities");
  if (!entitiesValue || !entitiesValue->is<JsonArray>()) {
    return fail("Scene must contain an entities array");
  }

  ecs::World loadedWorld;
  std::shared_ptr<graphics::Mesh> spriteMesh;
  for (const auto& entityValue : entitiesValue->get<JsonArray>()) {
    if (!entityValue.is<JsonObject>()) {
      return fail("Every entity must be an object");
    }
    const auto& entry = entityValue.get<JsonObject>();
    ecs::Entity entity = ecs::NullEntity;
    const JsonObject* componentValues = nullptr;
    if (!ReadEntity(entry, "id", entity) || entity == ecs::NullEntity ||
        !ReadObject(entry, "components", componentValues) ||
        !loadedWorld.RestoreEntity(entity)) {
      return fail("Entity has an invalid or duplicate id/components object");
    }
    for (const auto& [key, value] : *componentValues)
      if (!KnownComponent(key)) return fail("Unknown component: " + key);

    if (const auto* value = FindMember(*componentValues, "transform")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) + " has an invalid transform");
      }
      const auto& object = value->get<JsonObject>();
      components::Transform component;
      if (!ReadVec2(object, "position", component.position) ||
          !ReadVec2(object, "previousPosition", component.previousPosition) ||
          !ReadFloat(object, "rotation", component.rotation) ||
          !ReadVec2(object, "scale", component.scale)) {
        return fail("Entity " + std::to_string(entity) + " has an invalid transform");
      }
      loadedWorld.Add<components::Transform>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "spriteRenderer")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) + " has an invalid sprite renderer");
      }
      const auto& object = value->get<JsonObject>();
      std::string textureId;
      std::string shaderId;
      glm::vec2 uvOffset;
      glm::vec2 uvScale;
      int layer = 0;
      if (!ReadString(object, "texture", textureId) ||
          !ReadString(object, "shader", shaderId) ||
          !ReadVec2(object, "uvOffset", uvOffset) ||
          !ReadVec2(object, "uvScale", uvScale) ||
          !ReadInt(object, "layer", layer)) {
        return fail("Entity " + std::to_string(entity) + " has invalid sprite fields");
      }
      auto texture = assets.GetTexture(textureId);
      auto shader = assets.GetShader(shaderId);
      if (!texture || !texture->IsLoadedFromSource() || !shader ||
          !shader->IsValid()) {
        return fail("Entity " + std::to_string(entity) +
                    " references unavailable sprite assets");
      }
      if (!spriteMesh) {
        spriteMesh = CreateSpriteMesh();
      }
      components::SpriteRenderer component{spriteMesh, shader, texture};
      component.uvOffset = uvOffset;
      component.uvScale = uvScale;
      component.layer = layer;
      loadedWorld.Add<components::SpriteRenderer>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "textRenderer")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) +
                    " has an invalid text renderer");
      }
      const auto& object = value->get<JsonObject>();
      std::string fontId;
      std::string shaderId;
      std::string textValue;
      int pixelHeight = 0;
      int layer = 0;
      float scale = 1.0f;
      glm::vec3 color{1.0f};
      if (!ReadString(object, "font", fontId) ||
          !ReadInt(object, "pixelHeight", pixelHeight) || pixelHeight <= 0 ||
          !ReadString(object, "shader", shaderId) ||
          !ReadString(object, "text", textValue) ||
          !ReadVec3(object, "color", color) ||
          !ReadFloat(object, "scale", scale) || scale <= 0.0f ||
          !ReadInt(object, "layer", layer)) {
        return fail("Entity " + std::to_string(entity) +
                    " has invalid text fields");
      }
      auto font = assets.GetFont(fontId, static_cast<std::uint32_t>(pixelHeight));
      auto shader = assets.GetShader(shaderId);
      if (!font || !font->IsLoaded() || !shader || !shader->IsValid()) {
        return fail("Entity " + std::to_string(entity) +
                    " references unavailable text assets");
      }
      components::TextRenderer component;
      component.font = std::move(font);
      component.shader = std::move(shader);
      component.text = std::move(textValue);
      component.color = color;
      component.scale = scale;
      component.layer = layer;
      loadedWorld.Add<components::TextRenderer>(entity, std::move(component));
    }
    if (const auto* value = FindMember(*componentValues, "camera")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) + " has an invalid camera");
      }
      const auto& object = value->get<JsonObject>();
      components::Camera component;
      if (!ReadFloat(object, "zoom", component.zoom) ||
          !ReadVec2(object, "fitSize", component.fitSize) ||
          !ReadFloat(object, "fitScale", component.fitScale) ||
          !ReadEntity(object, "followTarget", component.followTarget)) {
        return fail("Entity " + std::to_string(entity) + " has invalid camera fields");
      }
      loadedWorld.Add<components::Camera>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "playerController")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) + " has an invalid player controller");
      }
      components::PlayerController component;
      if (!ReadFloat(value->get<JsonObject>(), "speed", component.speed)) {
        return fail("Entity " + std::to_string(entity) + " has an invalid player speed");
      }
      loadedWorld.Add<components::PlayerController>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "collider")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) + " has an invalid collider");
      }
      const auto& object = value->get<JsonObject>();
      components::Collider component;
      if (!ReadVec2(object, "size", component.size) ||
          !ReadVec2(object, "offset", component.offset) ||
          !ReadBool(object, "isStatic", component.isStatic)) {
        return fail("Entity " + std::to_string(entity) + " has invalid collider fields");
      }
      loadedWorld.Add<components::Collider>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "rigidBody2D")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) +
                    " has an invalid rigid body");
      }
      const auto& object = value->get<JsonObject>();
      components::RigidBody2D component;
      int type = 0;
      if (!ReadInt(object, "type", type) || type < 0 || type > 2 ||
          !ReadVec2(object, "linearVelocity", component.linearVelocity) ||
          !ReadFloat(object, "angularVelocity", component.angularVelocity) ||
          !ReadFloat(object, "linearDamping", component.linearDamping) ||
          !ReadFloat(object, "angularDamping", component.angularDamping) ||
          !ReadFloat(object, "gravityScale", component.gravityScale) ||
          !ReadBool(object, "fixedRotation", component.fixedRotation) ||
          !ReadBool(object, "bullet", component.bullet) ||
          !ReadBool(object, "enabled", component.enabled)) {
        return fail("Entity " + std::to_string(entity) +
                    " has invalid rigid body fields");
      }
      component.type = static_cast<components::PhysicsBodyType>(type);
      loadedWorld.Add<components::RigidBody2D>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "boxCollider2D")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) +
                    " has an invalid physics box");
      }
      const auto& object = value->get<JsonObject>();
      components::BoxCollider2D component;
      if (!ReadVec2(object, "size", component.size) ||
          component.size.x <= 0.0f || component.size.y <= 0.0f ||
          !ReadVec2(object, "offset", component.offset) ||
          !ReadPhysicsMaterial(object, "material", component.material)) {
        return fail("Entity " + std::to_string(entity) +
                    " has invalid physics box fields");
      }
      loadedWorld.Add<components::BoxCollider2D>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "circleCollider2D")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) +
                    " has an invalid physics circle");
      }
      const auto& object = value->get<JsonObject>();
      components::CircleCollider2D component;
      if (!ReadFloat(object, "radius", component.radius) ||
          component.radius <= 0.0f ||
          !ReadVec2(object, "offset", component.offset) ||
          !ReadPhysicsMaterial(object, "material", component.material)) {
        return fail("Entity " + std::to_string(entity) +
                    " has invalid physics circle fields");
      }
      loadedWorld.Add<components::CircleCollider2D>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "animation")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) + " has an invalid animation");
      }
      const auto& object = value->get<JsonObject>();
      components::Animation component;
      if (!ReadInt(object, "frameCount", component.frameCount) ||
          !ReadInt(object, "columns", component.columns) ||
          !ReadInt(object, "rows", component.rows) ||
          !ReadFloat(object, "frameDuration", component.frameDuration) ||
          !ReadFloat(object, "elapsed", component.elapsed) ||
          !ReadInt(object, "currentFrame", component.currentFrame)) {
        return fail("Entity " + std::to_string(entity) + " has invalid animation fields");
      }
      loadedWorld.Add<components::Animation>(entity, component);
    }
    if (const auto* value = FindMember(*componentValues, "npcBehavior")) {
      if (!value->is<JsonObject>()) {
        return fail("Entity " + std::to_string(entity) + " has invalid NPC behavior");
      }
      const auto& object = value->get<JsonObject>();
      components::NpcBehavior component;
      if (!ReadVec2(object, "patrolStart", component.patrolStart) ||
          !ReadVec2(object, "patrolEnd", component.patrolEnd) ||
          !ReadFloat(object, "speed", component.speed) ||
          !ReadBool(object, "movingToEnd", component.movingToEnd)) {
        return fail("Entity " + std::to_string(entity) + " has invalid NPC fields");
      }
      loadedWorld.Add<components::NpcBehavior>(entity, component);
    }
    try {
      if (const auto* value = FindMember(*componentValues, "name")) {
        components::Name component;
        core::json::Decode(*value, component);
        loadedWorld.Add<components::Name>(entity, component);
      }
      for (const auto& codec : mCodecs)
        if (const auto* value = FindMember(*componentValues, codec.name)) codec.read(loadedWorld, entity, *value);
    } catch (const std::exception& e) { return fail(e.what()); }
  }

  for (const auto entity : loadedWorld.GetEntities()) {
    const auto* camera = loadedWorld.Get<components::Camera>(entity);
    if (camera && camera->followTarget != ecs::NullEntity &&
        !loadedWorld.IsAlive(camera->followTarget)) {
      return fail("Entity " + std::to_string(entity) +
                  " camera follows a missing entity");
    }
  }

  world = std::move(loadedWorld);
  return true;
}

bool SceneManager::SavePrefab(const std::filesystem::path& path, ecs::World& world,
                              ecs::Entity entity, const AssetManager& assets) {
  picojson::value document;
  if (!entity || !ToJson(world, assets, document, entity)) return false;
  try { core::json::Write(path, document); return true; }
  catch (const std::exception& e) { mLastError = e.what(); return false; }
}

ecs::Entity SceneManager::Instantiate(const std::filesystem::path& path,
                                      ecs::World& world, AssetManager& assets) {
  ecs::World prefab;
  if (!LoadFile(path, prefab, assets)) return ecs::NullEntity;
  const auto entities = prefab.GetEntities();
  if (entities.size() != 1) {
    mLastError = "A prefab must contain exactly one entity";
    return ecs::NullEntity;
  }
  return world.CloneFrom(prefab, entities.front());
}

} // namespace eclipse::managers
