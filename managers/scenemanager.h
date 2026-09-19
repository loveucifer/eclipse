#pragma once

#include "assetmanager.h"
#include "../ecs/world.h"
#include "../external/picojson.h"
#include "../core/jsondata.h"

#include <functional>
#include <filesystem>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace eclipse::managers {

class SceneManager {
public:
  using SceneLoader = std::function<void(ecs::World&, AssetManager&)>;

  void Register(const std::string& name, SceneLoader loader);
  bool Load(const std::string& name, ecs::World& world, AssetManager& assets);
  bool SaveFile(const std::filesystem::path& path, ecs::World& world,
                const AssetManager& assets);
  bool LoadFile(const std::filesystem::path& path, ecs::World& world,
                AssetManager& assets);
  bool ToJson(ecs::World& world, const AssetManager& assets, picojson::value& result,
              ecs::Entity onlyEntity = ecs::NullEntity);
  bool FromJson(const picojson::value& document, ecs::World& world, AssetManager& assets);
  bool SavePrefab(const std::filesystem::path& path, ecs::World& world,
                  ecs::Entity entity, const AssetManager& assets);
  ecs::Entity Instantiate(const std::filesystem::path& path, ecs::World& world,
                           AssetManager& assets);
  // Custom data components expose Fields(visitor) for generic JSON encoding.
  template<class T> bool RegisterComponent(const std::string& name) {
    if (name.empty() || KnownComponent(name)) { mLastError = "Invalid or registered component name: " + name; return false; }
    mCodecs.push_back({name, std::type_index(typeid(T)),
      [](ecs::World& world, ecs::Entity id, picojson::value& out) {
        if (auto* c = world.Get<T>(id)) { out = core::json::Encode(*c); return true; }
        return false;
      },
      [](ecs::World& world, ecs::Entity id, const picojson::value& in) {
        T c{}; core::json::Decode(in, c); world.Add<T>(id, c);
      }});
    return true;
  }
  const std::string& Current() const { return mCurrent; }
  const std::string& LastError() const { return mLastError; }

private:
  bool KnownComponent(const std::string& name) const;
  struct Codec {
    std::string name;
    std::type_index type;
    std::function<bool(ecs::World&, ecs::Entity, picojson::value&)> write;
    std::function<void(ecs::World&, ecs::Entity, const picojson::value&)> read;
  };
  std::vector<Codec> mCodecs;
  std::unordered_map<std::string, SceneLoader> mLoaders;
  std::string mCurrent;
  std::string mLastError;
};

} // namespace eclipse::managers
