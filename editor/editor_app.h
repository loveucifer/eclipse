#pragma once

// LEARNING MAP
// This class is the sample game/editor built on top of the engine.
// Start with Initialize() in editor_app.cpp to see how assets become entities.
// Then follow Update() for input and ImGuiRender() for the editor windows.

#include "../src/app.h"
#include "../ecs/entity.h"
#include "../graphics/mesh.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"
#include "../systems/pathfinding.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace eclipse::editor {

class EditorApp final : public App {
public:
  core::WindowProperites GetWindowProperties() override;
  void Initialize(ecs::World& world, managers::AssetManager& assets) override;
  void OnSceneLoaded(ecs::World& world, managers::AssetManager& assets) override;
  void Update(ecs::World& world, float deltaTime) override;
  void Shutdown() override;
  void ImGuiRender() override;

private:
  // These are shared GPU objects. Entities only keep references to them.
  std::shared_ptr<graphics::Mesh> mMesh;
  std::shared_ptr<graphics::Shader> mShader;
  std::shared_ptr<graphics::Texture> mIdleTexture;
  std::shared_ptr<graphics::Texture> mWalkTexture;
  std::shared_ptr<graphics::Texture> mAttackTexture;
  std::shared_ptr<graphics::Texture> mChargeTexture;
  std::shared_ptr<graphics::Texture> mHurtTexture;
  std::shared_ptr<graphics::Texture> mDeadTexture;

  bool mFacingRight = true;
  ecs::Entity mPlayerEntity = ecs::NullEntity;
  ecs::Entity mCameraEntity = ecs::NullEntity;
  ecs::Entity mSelectedEntity = ecs::NullEntity;
  systems::PathGrid mDemoGrid;
  std::vector<systems::PathSearchFrame> mDemoFrames;
  std::size_t mDemoFrame = 0;
  std::array<char, 128> mAssetFilter{};
  std::array<char, 256> mScenePath{};
  std::string mSelectedAssetId;
  std::string mSceneStatus;
  std::string mPrefabPath = "prefabs/object.json";
  float mDemoElapsed = 0.0f;
};

} // namespace eclipse::editor
