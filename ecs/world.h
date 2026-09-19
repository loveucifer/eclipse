#pragma once

// LEARNING MAP
// World is the engine's object database. An entity is only an ID; components
// hold its data; systems read that data and make the game change.
// Study CreateEntity(), Add(), Get(), and ForEach() in that order.
// Then open systems/movement_system.cpp to see a system use the database.

#include "entity.h"

#include <memory>
#include <limits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace eclipse::ecs {

class World {
private:
  struct StorageBase {
    virtual ~StorageBase() = default;
    virtual void Erase(Entity entity) = 0;
    virtual void CopyTo(Entity source, Entity target, World& world) const = 0;
    virtual bool Contains(Entity entity) const = 0;
  };

  template <typename T>
  struct Storage final : StorageBase {
    std::unordered_map<Entity, T> values;

    void Erase(Entity entity) override { values.erase(entity); }
    bool Contains(Entity entity) const override { return values.count(entity) != 0; }
    void CopyTo(Entity source, Entity target, World& world) const override {
      const auto it = values.find(source);
      if (it != values.end()) world.Add<T>(target, it->second);
    }
  };

  std::unordered_map<std::type_index, std::unique_ptr<StorageBase>> mStorages;
  std::unordered_set<Entity> mEntities;
  std::unordered_set<Entity> mPendingDestroy;
  Entity mNextEntity = 1;

  bool IsPendingDestroy(Entity entity) const {
    return mPendingDestroy.find(entity) != mPendingDestroy.end();
  }

  template <typename T>
  Storage<T>* FindStorage() {
    auto it = mStorages.find(std::type_index(typeid(T)));
    if (it == mStorages.end()) {
      return nullptr;
    }
    return static_cast<Storage<T>*>(it->second.get());
  }

  template <typename T>
  Storage<T>& GetOrCreateStorage() {
    auto key = std::type_index(typeid(T));
    auto it = mStorages.find(key);
    if (it == mStorages.end()) {
      auto storage = std::make_unique<Storage<T>>();
      auto* result = storage.get();
      mStorages.emplace(key, std::move(storage));
      return *result;
    }
    return *static_cast<Storage<T>*>(it->second.get());
  }

public:
  Entity CreateEntity() {
    if (mNextEntity == std::numeric_limits<Entity>::max()) {
      return NullEntity;
    }
    const Entity entity = mNextEntity++;
    mEntities.insert(entity);
    return entity;
  }

  bool RestoreEntity(Entity entity) {
    if (entity == NullEntity || entity == std::numeric_limits<Entity>::max() ||
        mEntities.find(entity) != mEntities.end()) {
      return false;
    }
    mEntities.insert(entity);
    if (entity >= mNextEntity) {
      mNextEntity = entity + 1;
    }
    return true;
  }

  Entity CloneFrom(const World& source, Entity entity) {
    if (!source.IsAlive(entity)) return NullEntity;
    const Entity copy = CreateEntity();
    if (copy == NullEntity) return copy;
    for (const auto& [type, storage] : source.mStorages)
      storage->CopyTo(entity, copy, *this);
    return copy;
  }

  std::vector<std::type_index> ComponentTypes(Entity entity) const {
    std::vector<std::type_index> types;
    for (const auto& [type, storage] : mStorages)
      if (storage->Contains(entity)) types.push_back(type);
    return types;
  }

  bool IsAlive(Entity entity) const {
    return mEntities.find(entity) != mEntities.end() &&
           !IsPendingDestroy(entity);
  }

  std::vector<Entity> GetEntities() const {
    return {mEntities.begin(), mEntities.end()};
  }

  void DestroyEntity(Entity entity) {
    if (mEntities.find(entity) != mEntities.end()) {
      mPendingDestroy.insert(entity);
    }
  }

  void FlushDestroyed() {
    for (Entity entity : mPendingDestroy) {
      for (auto& [type, storage] : mStorages) {
        (void)type;
        storage->Erase(entity);
      }
      mEntities.erase(entity);
    }
    mPendingDestroy.clear();
  }

  void Clear() {
    mStorages.clear();
    mEntities.clear();
    mPendingDestroy.clear();
    mNextEntity = 1;
  }

  template <typename T, typename... Args>
  T& Add(Entity entity, Args&&... args) {
    auto& storage = GetOrCreateStorage<T>();
    auto [it, inserted] = storage.values.emplace(
        entity, T(std::forward<Args>(args)...));
    if (!inserted) {
      it->second = T(std::forward<Args>(args)...);
    }
    return it->second;
  }

  template <typename T>
  T* Get(Entity entity) {
    if (!IsAlive(entity)) {
      return nullptr;
    }
    auto* storage = FindStorage<T>();
    if (!storage) {
      return nullptr;
    }
    auto it = storage->values.find(entity);
    return it == storage->values.end() ? nullptr : &it->second;
  }

  template <typename T>
  void Remove(Entity entity) {
    if (auto* storage = FindStorage<T>()) {
      storage->values.erase(entity);
    }
  }

  template <typename T>
  T* GetFirst() {
    auto* storage = FindStorage<T>();
    if (!storage) {
      return nullptr;
    }
    T* first = nullptr;
    Entity lowest = std::numeric_limits<Entity>::max();
    for (auto& [entity, component] : storage->values) {
      if (IsAlive(entity) && entity < lowest) { lowest = entity; first = &component; }
    }
    return first;
  }

  template <typename First, typename Second, typename Function>
  void ForEach(Function&& function) {
    auto* first = FindStorage<First>();
    if (!first) {
      return;
    }
    for (auto& [entity, firstComponent] : first->values) {
      if (!IsAlive(entity)) {
        continue;
      }
      auto* secondComponent = Get<Second>(entity);
      if (secondComponent) {
        function(entity, firstComponent, *secondComponent);
      }
    }
  }
};

} // namespace eclipse::ecs
