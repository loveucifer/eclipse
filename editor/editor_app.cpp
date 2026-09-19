#include "editor_app.h"

// LEARNING MAP
// This file is intentionally the most approachable example game in the repo.
// Learn these functions in order:
// 1. GetWindowProperties: choose the application window size.
// 2. Initialize: load files and create entities/components.
// 3. Update: respond to game input each frame.
// 4. ImGuiRender: build tools and debug windows.

#include "../components/gameplaycomponents.h"
#include "../components/rendercomponents.h"
#include "../input/keyboard.h"
#include "../managers/layeredtilemaploader.h"
#include "../src/engine.h"
#include "../src/log.h"

#include <external/imgui/imgui.h>
#include <external/imgui/imgui_stdlib.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>

namespace eclipse::editor {

namespace {

const char* TextureFilterName(graphics::TextureFilter filter) {
  return filter == graphics::TextureFilter::Nearest ? "Nearest" : "Linear";
}

bool DrawAssetCard(managers::AssetManager& assetManager,
                   const managers::AssetInfo& asset, bool selected,
                   const ImVec2& size) {
  ImGui::PushID(asset.id.c_str());
  const ImVec2 topLeft = ImGui::GetCursorScreenPos();
  const bool clicked = ImGui::Selectable("##asset-card", selected, 0, size);
  const bool hovered = ImGui::IsItemHovered();
  auto* draw = ImGui::GetWindowDrawList();
  const ImVec2 bottomRight{topLeft.x + size.x, topLeft.y + size.y};

  const ImU32 background = selected
                               ? IM_COL32(65, 42, 92, 255)
                               : hovered ? IM_COL32(48, 48, 58, 255)
                                         : IM_COL32(31, 31, 38, 255);
  const ImU32 border = selected ? IM_COL32(197, 116, 255, 255)
                                : IM_COL32(72, 72, 84, 255);
  draw->AddRectFilled(topLeft, bottomRight, background, 5.0f);
  draw->AddRect(topLeft, bottomRight, border, 5.0f, 0,
                selected ? 3.0f : 1.0f);

  constexpr float padding = 7.0f;
  constexpr float imageHeight = 82.0f;
  const ImVec2 imageMin{topLeft.x + padding, topLeft.y + padding};
  const ImVec2 imageMax{bottomRight.x - padding,
                        topLeft.y + padding + imageHeight};
  draw->AddRectFilled(imageMin, imageMax, IM_COL32(18, 18, 22, 255), 3.0f);

  std::shared_ptr<graphics::Texture> texture;
  if (asset.type == managers::AssetType::Texture) {
    texture = assetManager.GetTexture(asset.id);
  }
  if (texture) {
    const float sourceWidth = static_cast<float>(texture->GetWidth());
    const float sourceHeight = static_cast<float>(texture->GetHeight());
    const float availableWidth = imageMax.x - imageMin.x;
    const float availableHeight = imageMax.y - imageMin.y;
    const float scale =
        sourceWidth > 0.0f && sourceHeight > 0.0f
            ? std::min(availableWidth / sourceWidth,
                       availableHeight / sourceHeight)
            : 0.0f;
    const ImVec2 drawSize{sourceWidth * scale, sourceHeight * scale};
    const ImVec2 drawMin{imageMin.x + (availableWidth - drawSize.x) * 0.5f,
                         imageMin.y + (availableHeight - drawSize.y) * 0.5f};
    const ImVec2 drawMax{drawMin.x + drawSize.x, drawMin.y + drawSize.y};
    draw->AddImage(ImTextureID(texture->GetId()), drawMin, drawMax, {0, 1},
                   {1, 0});

    const std::string dimensions =
        std::to_string(texture->GetWidth()) + " x " +
        std::to_string(texture->GetHeight());
    draw->AddText({imageMin.x + 4.0f, imageMax.y - ImGui::GetTextLineHeight()},
                  IM_COL32(230, 230, 235, 255), dimensions.c_str());
  } else {
    const char* typeName = managers::AssetTypeName(asset.type);
    const ImVec2 typeSize = ImGui::CalcTextSize(typeName);
    draw->AddText(
        {imageMin.x + (imageMax.x - imageMin.x - typeSize.x) * 0.5f,
         imageMin.y + (imageMax.y - imageMin.y - typeSize.y) * 0.5f},
        IM_COL32(180, 180, 195, 255), typeName);
  }

  const ImVec4 textClip{topLeft.x + padding, imageMax.y + 6.0f,
                        bottomRight.x - padding, bottomRight.y - 4.0f};
  draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                {textClip.x, textClip.y}, IM_COL32(240, 240, 245, 255),
                asset.id.c_str(), nullptr, textClip.z - textClip.x, &textClip);

  if (hovered) {
    ImGui::BeginTooltip();
    ImGui::TextUnformatted(asset.id.c_str());
    ImGui::Separator();
    ImGui::Text("Type: %s", managers::AssetTypeName(asset.type));
    ImGui::Text("Group: %s", asset.group.c_str());
    if (asset.type == managers::AssetType::Shader) {
      ImGui::Text("Vertex: %s", asset.vertexPath.string().c_str());
      ImGui::Text("Fragment: %s", asset.fragmentPath.string().c_str());
    } else {
      ImGui::Text("Source: %s", asset.path.string().c_str());
    }
    ImGui::EndTooltip();
  }

  ImGui::PopID();
  return clicked;
}

} // namespace

core::WindowProperites EditorApp::GetWindowProperties() {
  core::WindowProperites props;
  props.title = "eclipseeditor";
  props.w = 1280;
  props.h = 900;
  props.wMin = 200;
  props.hMin = 200;
  props.ImGuiProps.IsDockingEnabled = true;
  props.ImGuiProps.IsViewportEnabled = true;
  props.ImGuiProps.MoveFromTitleBarOnly = true;
  return props;
}

void EditorApp::Initialize(ecs::World& world, managers::AssetManager& assets) {
  ECLIPSE_TRACE("EditorApp::Initialize()");

  if (mScenePath[0] == '\0') {
    constexpr char defaultScenePath[] = "scenes/main.scene.json";
    std::copy(std::begin(defaultScenePath), std::end(defaultScenePath),
              mScenePath.begin());
  }

  // The engine's sprite renderer expects one reusable textured quad.
  // Learn next: graphics/mesh.cpp, then graphics/render_system.cpp.
  float vertices[]{0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
                   -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f};
  uint32_t elements[]{0, 3, 1, 1, 3, 2};
  float texcoords[]{1.0f, 1.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f};
  mMesh = std::make_shared<graphics::Mesh>(
      vertices, 4, 3, texcoords, elements, 6);

  // Asset IDs are resolved through assets/manifest.json. Game code does not
  // depend on repository-relative file paths.
  mIdleTexture = assets.GetTexture("character.schoolgirl1.idle");
  mWalkTexture = assets.GetTexture("character.schoolgirl1.walk");
  mAttackTexture = assets.GetTexture("character.schoolgirl1.attack");
  mChargeTexture = assets.GetTexture("character.schoolgirl1.charge");
  mHurtTexture = assets.GetTexture("character.schoolgirl1.hurt");
  mDeadTexture = assets.GetTexture("character.schoolgirl1.dead");
  mShader = assets.GetShader("shader.sprite");
  const auto defaultFont = assets.GetFont("font.default", 32);
  const auto physicsFloorTexture = assets.GetTexture("tetris.block.i");
  const auto physicsBoxTexture = assets.GetTexture("tetris.block.t");
  const auto physicsBallTexture = assets.GetTexture("snake.bonus.blue");
  if (!mIdleTexture || !mWalkTexture || !mAttackTexture || !mChargeTexture ||
      !mHurtTexture || !mDeadTexture || !mShader || !defaultFont ||
      !physicsFloorTexture || !physicsBoxTexture || !physicsBallTexture) {
    ECLIPSE_ERROR("Editor scene cannot initialize because required assets failed");
    return;
  }

  // A factory keeps entity creation consistent. The components describe data;
  // systems later decide what that data does.
  auto createSprite = [&world, this](glm::vec2 position, glm::vec2 scale,
                                     bool controlled, int layer) {
    const auto entity = world.CreateEntity();
    world.Add<components::Transform>(
        entity, components::Transform{position, position, 0.0f, scale});
    auto& sprite = world.Add<components::SpriteRenderer>(
        entity, components::SpriteRenderer{mMesh, mShader, mIdleTexture});
    sprite.layer = layer;
    if (controlled) {
      world.Add<components::PlayerController>(entity);
    }
    return entity;
  };

  auto createNpc = [&world, this](glm::vec2 position,
                                  const std::shared_ptr<graphics::Texture>& texture,
                                  int frameCount, float size, int layer) {
    const auto entity = world.CreateEntity();
    world.Add<components::Transform>(
        entity, components::Transform{position, position, 0.0f,
                                      {size, -size}});
    auto& sprite = world.Add<components::SpriteRenderer>(
        entity, components::SpriteRenderer{mMesh, mShader, texture});
    sprite.layer = layer;
    world.Add<components::Animation>(
        entity, components::Animation{frameCount, frameCount, 1, 0.16f});
    world.Add<components::NpcBehavior>(
        entity, components::NpcBehavior{{position.x - 45.0f, position.y},
                                         {position.x + 45.0f, position.y}, 18.0f});
    return entity;
  };

  // Player data: transform, rendering, movement, collision, and animation.
  mPlayerEntity = createSprite({464.0f, 500.0f}, {112.0f, -112.0f}, true, 100);
  world.Add<components::Collider>(
      mPlayerEntity, components::Collider{{24.0f, 24.0f}, {0.0f, 0.0f}, false});
  world.Add<components::Animation>(
      mPlayerEntity, components::Animation{6, 6, 1, 0.12f});

  // Each layer is a manifest asset, so validation and packaged builds include
  // every map dependency.
  managers::LayeredTilemapLoader mapLoader;
  mapLoader.Load(
      {"map.background.layer0011", "map.background.layer0010",
       "map.background.layer0009", "map.background.layer0008",
       "map.background.layer0006", "map.background.layer0005",
       "map.background.layer0003", "map.background.layer0002",
       "map.background.layer0001", "map.background.layer0000",
       "map.background.lights0004", "map.background.lights0007"},
      world, assets, mMesh, mShader);

  // NPCs are ordinary entities with an Animation and NpcBehavior component.
  createNpc({340.0f, 470.0f}, mWalkTexture, 6, 96.0f, 110);
  createNpc({590.0f, 450.0f}, mAttackTexture, 4, 112.0f, 111);
  createNpc({520.0f, 585.0f}, mChargeTexture, 7, 64.0f, 112);
  createNpc({690.0f, 530.0f}, mHurtTexture, 2, 96.0f, 113);

  // Four invisible solid walls keep the player inside the image.
  auto createBoundary = [&world](glm::vec2 position, glm::vec2 size) {
    const auto entity = world.CreateEntity();
    world.Add<components::Transform>(entity,
                                     components::Transform{position});
    world.Add<components::Collider>(
        entity, components::Collider{size, {0.0f, 0.0f}, true});
  };
  createBoundary({-8.0f, 396.5f}, {16.0f, 793.0f});
  createBoundary({936.0f, 396.5f}, {16.0f, 793.0f});
  createBoundary({464.0f, -8.0f}, {928.0f, 16.0f});
  createBoundary({464.0f, 801.0f}, {928.0f, 16.0f});

  // The camera is an entity too. fitSize keeps the map's proportions correct.
  mCameraEntity = world.CreateEntity();
  world.Add<components::Transform>(
      mCameraEntity, components::Transform{{464.0f, 396.5f}});
  auto& camera = world.Add<components::Camera>(mCameraEntity);
  camera.fitSize = {928.0f, 793.0f};
  camera.fitScale = 1.25f;

  const auto textEntity = world.CreateEntity();
  world.Add<components::Name>(textEntity, components::Name{"Engine text"});
  world.Add<components::Transform>(
      textEntity, components::Transform{{28.0f, 52.0f}, {28.0f, 52.0f},
                                        0.0f, {1.0f, -1.0f}});
  components::TextRenderer label;
  label.font = defaultFont;
  label.shader = mShader;
  label.text = "Eclipse text rendering";
  label.color = {0.85f, 0.7f, 1.0f};
  label.layer = 1000;
  world.Add<components::TextRenderer>(textEntity, std::move(label));

  // A visible Box2D sandbox: dynamic blocks and a ball fall onto this platform.
  auto createPhysicsBox = [&world, this](
                              const char* name, glm::vec2 position,
                              glm::vec2 size,
                              const std::shared_ptr<graphics::Texture>& texture,
                              components::PhysicsBodyType type,
                              float angularVelocity = 0.0f) {
    const auto entity = world.CreateEntity();
    world.Add<components::Name>(entity, components::Name{name});
    world.Add<components::Transform>(
        entity, components::Transform{position, position, 0.0f,
                                      {size.x, -size.y}});
    auto& sprite = world.Add<components::SpriteRenderer>(
        entity, components::SpriteRenderer{mMesh, mShader, texture});
    sprite.layer = 900;
    components::RigidBody2D body;
    body.type = type;
    body.angularVelocity = angularVelocity;
    world.Add<components::RigidBody2D>(entity, body);
    components::BoxCollider2D collider;
    collider.size = size;
    collider.material.restitution = 0.2f;
    world.Add<components::BoxCollider2D>(entity, collider);
  };

  createPhysicsBox("Box2D floor", {750.0f, 350.0f}, {280.0f, 24.0f},
                   physicsFloorTexture,
                   components::PhysicsBodyType::Static);
  createPhysicsBox("Box2D block 1", {690.0f, 75.0f}, {44.0f, 44.0f},
                   physicsBoxTexture,
                   components::PhysicsBodyType::Dynamic, 0.8f);
  createPhysicsBox("Box2D block 2", {750.0f, 125.0f}, {52.0f, 52.0f},
                   physicsBoxTexture,
                   components::PhysicsBodyType::Dynamic, -0.5f);
  createPhysicsBox("Box2D block 3", {790.0f, 45.0f}, {36.0f, 36.0f},
                   physicsBoxTexture,
                   components::PhysicsBodyType::Dynamic, 1.2f);

  const auto ball = world.CreateEntity();
  world.Add<components::Name>(ball, components::Name{"Box2D ball"});
  world.Add<components::Transform>(
      ball, components::Transform{{850.0f, 95.0f}, {850.0f, 95.0f}, 0.0f,
                                  {40.0f, -40.0f}});
  auto& ballSprite = world.Add<components::SpriteRenderer>(
      ball, components::SpriteRenderer{mMesh, mShader, physicsBallTexture});
  ballSprite.layer = 900;
  world.Add<components::RigidBody2D>(ball);
  components::CircleCollider2D ballCollider;
  ballCollider.radius = 20.0f;
  ballCollider.material.restitution = 0.65f;
  world.Add<components::CircleCollider2D>(ball, ballCollider);

  const auto physicsText = world.CreateEntity();
  world.Add<components::Name>(physicsText,
                              components::Name{"Box2D demo label"});
  world.Add<components::Transform>(
      physicsText, components::Transform{{620.0f, 390.0f}, {620.0f, 390.0f},
                                         0.0f, {1.0f, -1.0f}});
  components::TextRenderer physicsLabel;
  physicsLabel.font = defaultFont;
  physicsLabel.shader = mShader;
  physicsLabel.text = "Box2D live demo - F5 to reset";
  physicsLabel.color = {0.45f, 0.85f, 1.0f};
  physicsLabel.scale = 0.55f;
  physicsLabel.layer = 1000;
  world.Add<components::TextRenderer>(physicsText, std::move(physicsLabel));

  // A small fixed demo grid makes the algorithm visible before an NPC uses it.
  // Learn next: systems/pathfinding.cpp, especially cost and parent arrays.
  mDemoGrid.width = 20;
  mDemoGrid.height = 14;
  mDemoGrid.blocked.assign(mDemoGrid.width * mDemoGrid.height, false);
  for (int x = 2; x < 18; ++x) {
    mDemoGrid.blocked[3 * mDemoGrid.width + x] = x != 4;
    mDemoGrid.blocked[6 * mDemoGrid.width + x] = x != 15;
    mDemoGrid.blocked[9 * mDemoGrid.width + x] = x != 7;
  }
  for (int y = 4; y < 9; ++y) {
    mDemoGrid.blocked[y * mDemoGrid.width + 11] = true;
  }
  mDemoFrames = systems::AStarPathfinder::FindPathTrace(
      mDemoGrid, {1, 1}, {18, 12});
}

void EditorApp::Update(ecs::World& world, float deltaTime) {
  if (!mDemoFrames.empty()) {
    mDemoElapsed += deltaTime;
    if (mDemoElapsed >= 0.08f) {
      mDemoElapsed = 0.0f;
      if (mDemoFrame + 1 < mDemoFrames.size()) {
        ++mDemoFrame;
      }
    }
  }
  // Animation and movement are handled by engine systems. This function only
  // chooses which player animation should be shown.
  if (Engine::Instance().GetSceneSource() != "main") return;
  auto* sprite = world.Get<components::SpriteRenderer>(mPlayerEntity);
  auto* transform = world.Get<components::Transform>(mPlayerEntity);
  auto* animation = world.Get<components::Animation>(mPlayerEntity);
  if (!sprite || !transform || !animation) {
    return;
  }

  std::shared_ptr<graphics::Texture> texture = mIdleTexture;
  int frameCount = 6;
  float frameSize = 128.0f;
  if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_LEFT) ||
      input::keyboard::Key(input::ECLIPSE_INPUT_KEY_A)) {
    mFacingRight = false;
  } else if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_RIGHT) ||
             input::keyboard::Key(input::ECLIPSE_INPUT_KEY_D)) {
    mFacingRight = true;
  }
  if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_SPACE)) {
    texture = mAttackTexture;
    frameCount = 4;
  } else if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_C)) {
    texture = mChargeTexture;
    frameCount = 7;
    frameSize = 64.0f;
  } else if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_H)) {
    texture = mHurtTexture;
    frameCount = 2;
  } else if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_K)) {
    texture = mDeadTexture;
    frameCount = 2;
  } else if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_LEFT) ||
             input::keyboard::Key(input::ECLIPSE_INPUT_KEY_RIGHT) ||
             input::keyboard::Key(input::ECLIPSE_INPUT_KEY_UP) ||
             input::keyboard::Key(input::ECLIPSE_INPUT_KEY_DOWN) ||
             input::keyboard::Key(input::ECLIPSE_INPUT_KEY_A) ||
             input::keyboard::Key(input::ECLIPSE_INPUT_KEY_D) ||
             input::keyboard::Key(input::ECLIPSE_INPUT_KEY_W) ||
             input::keyboard::Key(input::ECLIPSE_INPUT_KEY_S)) {
    texture = mWalkTexture;
  }

  if (sprite->texture != texture) {
    sprite->texture = texture;
    animation->currentFrame = 0;
    animation->elapsed = 0.0f;
  }
  animation->frameCount = frameCount;
  animation->columns = frameCount;
  animation->rows = 1;
  transform->scale = {mFacingRight ? frameSize : -frameSize, -frameSize};
}

void EditorApp::OnSceneLoaded(ecs::World& world, managers::AssetManager& assets) {
  mPlayerEntity = mCameraEntity = mSelectedEntity = ecs::NullEntity;
  auto ids = world.GetEntities(); std::sort(ids.begin(), ids.end());
  for (auto id : ids) {
    if (!mPlayerEntity && world.Get<components::PlayerController>(id)) mPlayerEntity = id;
    if (!mCameraEntity && world.Get<components::Camera>(id)) mCameraEntity = id;
  }
  if (Engine::Instance().GetSceneSource() == "main") {
    mIdleTexture = assets.GetTexture("character.schoolgirl1.idle");
    mWalkTexture = assets.GetTexture("character.schoolgirl1.walk");
    mAttackTexture = assets.GetTexture("character.schoolgirl1.attack");
    mChargeTexture = assets.GetTexture("character.schoolgirl1.charge");
    mHurtTexture = assets.GetTexture("character.schoolgirl1.hurt");
    mDeadTexture = assets.GetTexture("character.schoolgirl1.dead");
  }
}

void EditorApp::Shutdown() {
  // Release GPU objects before the OpenGL window is destroyed.
  mIdleTexture.reset();
  mWalkTexture.reset();
  mAttackTexture.reset();
  mChargeTexture.reset();
  mHurtTexture.reset();
  mDeadTexture.reset();
  mShader.reset();
  mMesh.reset();
}

void EditorApp::ImGuiRender() {
  // This is editor-only UI. A finished game can replace this with game menus.
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
  auto& engine = Engine::Instance();
  auto& gameState = engine.GetGameState();
  const bool commandDown = ImGui::GetIO().KeyCtrl || ImGui::GetIO().KeySuper;
  if (mScenePath[0] != '\0' && commandDown &&
      ImGui::IsKeyPressed(ImGuiKey_S, false)) {
    if (engine.SaveScene(mScenePath.data())) {
      mSceneStatus = "Saved " + std::string(mScenePath.data());
    } else {
      mSceneStatus = engine.GetSceneManager().LastError();
    }
  }

  ImGui::Begin("Scene");
  ImGui::TextWrapped("Current: %s", engine.GetSceneSource().c_str());
  ImGui::InputText("File", mScenePath.data(), mScenePath.size());
  if (ImGui::Button("Save") && mScenePath[0] != '\0') {
    if (engine.SaveScene(mScenePath.data())) {
      mSceneStatus = "Saved " + std::string(mScenePath.data());
    } else {
      mSceneStatus = engine.GetSceneManager().LastError();
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Load") && mScenePath[0] != '\0') {
    if (engine.LoadSceneFile(mScenePath.data())) {
      mSceneStatus = "Loaded " + std::string(mScenePath.data());
      if (!engine.GetWorld().IsAlive(mSelectedEntity)) {
        mSelectedEntity = ecs::NullEntity;
      }
    } else {
      mSceneStatus = engine.GetSceneManager().LastError();
    }
  }
  ImGui::SameLine();
  ImGui::TextDisabled("Ctrl/Cmd+S");
  if (!mSceneStatus.empty()) {
    ImGui::TextWrapped("%s", mSceneStatus.c_str());
  }
  ImGui::End();

  ImGui::Begin("Game State");
  const auto state = gameState.Current();
  const char* stateName = state == managers::GameState::Playing ? "Playing"
                             : state == managers::GameState::Paused ? "Paused"
                             : state == managers::GameState::Menu ? "Menu"
                             : state == managers::GameState::GameOver ? "Game Over"
                                                                       : "Boot";
  ImGui::Text("State: %s", stateName);
  if (state == managers::GameState::Playing && ImGui::Button("Pause"))
    gameState.Set(managers::GameState::Paused);
  if (state == managers::GameState::Paused && ImGui::Button("Resume"))
    gameState.Set(managers::GameState::Playing);
  if (state != managers::GameState::Menu && ImGui::Button("Main Menu"))
    gameState.Set(managers::GameState::Menu);
  if (state == managers::GameState::Menu && ImGui::Button("Start Game"))
    gameState.Set(managers::GameState::Playing);
  if (state == managers::GameState::Playing && ImGui::Button("Game Over"))
    gameState.Set(managers::GameState::GameOver);
  if (state == managers::GameState::GameOver && ImGui::Button("Restart")) {
    engine.LoadScene("main");
    gameState.Set(managers::GameState::Playing);
  }
  ImGui::End();

  ImGui::SetNextWindowSize({760.0f, 720.0f}, ImGuiCond_FirstUseEver);
  ImGui::Begin("Assets");
  auto& assetManager = engine.GetAssetManager();
  ImGui::TextDisabled("Asset root: %s",
                      assetManager.GetAssetRoot().string().c_str());
  ImGui::SetNextItemWidth(-1.0f);
  ImGui::InputTextWithHint("##asset-search", "Search asset ID or path",
                           mAssetFilter.data(), mAssetFilter.size());
  const std::string assetFilter = mAssetFilter.data();

  if (mSelectedAssetId.empty() && !assetManager.GetAssets().empty()) {
    mSelectedAssetId = assetManager.GetAssets().front().id;
  }

  constexpr float cardWidth = 148.0f;
  constexpr float cardHeight = 132.0f;
  const float listHeight =
      std::max(260.0f, ImGui::GetContentRegionAvail().y * 0.52f);
  if (ImGui::BeginChild("AssetCards", {0.0f, listHeight}, true)) {
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const int columnCount = std::max(
        1, static_cast<int>((ImGui::GetContentRegionAvail().x + spacing) /
                            (cardWidth + spacing)));
    int visibleIndex = 0;
    for (const auto& asset : assetManager.GetAssets()) {
      const std::string sourceText =
          asset.type == managers::AssetType::Shader
              ? asset.vertexPath.string() + " " + asset.fragmentPath.string()
              : asset.path.string();
      if (!assetFilter.empty() &&
          asset.id.find(assetFilter) == std::string::npos &&
          sourceText.find(assetFilter) == std::string::npos) {
        continue;
      }
      if (visibleIndex % columnCount != 0) {
        ImGui::SameLine();
      }
      if (DrawAssetCard(assetManager, asset,
                        mSelectedAssetId == asset.id,
                        {cardWidth, cardHeight})) {
        mSelectedAssetId = asset.id;
      }
      ++visibleIndex;
    }
    if (visibleIndex == 0) {
      ImGui::TextDisabled("No assets match \"%s\"", assetFilter.c_str());
    }
  }
  ImGui::EndChild();

  if (const auto* asset = assetManager.FindAsset(mSelectedAssetId)) {
    ImGui::SeparatorText("Selected asset");
    ImGui::TextColored({0.78f, 0.48f, 1.0f, 1.0f}, "%s",
                       asset->id.c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton("Copy ID")) {
      ImGui::SetClipboardText(asset->id.c_str());
    }
    ImGui::Text("Type: %s    Group: %s",
                managers::AssetTypeName(asset->type), asset->group.c_str());

    if (asset->type == managers::AssetType::Shader) {
      ImGui::TextWrapped("Vertex source: %s",
                         asset->vertexPath.string().c_str());
      ImGui::TextWrapped("Fragment source: %s",
                         asset->fragmentPath.string().c_str());
    } else {
      ImGui::TextWrapped("Manifest path: %s", asset->path.string().c_str());
      const auto resolvedPath = assetManager.ResolveAssetPath(asset->id);
      if (!resolvedPath.empty()) {
        ImGui::TextWrapped("Resolved file: %s",
                           resolvedPath.string().c_str());
      }
    }

    if (asset->type == managers::AssetType::Texture) {
      const auto texture = assetManager.GetTexture(asset->id);
      if (texture) {
        ImGui::Text("Image: %u x %u px    Filter: %s", texture->GetWidth(),
                    texture->GetHeight(),
                    TextureFilterName(texture->GetTextureFilter()));
        const float sourceWidth = static_cast<float>(texture->GetWidth());
        const float sourceHeight = static_cast<float>(texture->GetHeight());
        const float previewWidth =
            std::min(sourceWidth, ImGui::GetContentRegionAvail().x);
        const float previewHeight =
            sourceWidth > 0.0f
                ? std::min(previewWidth * sourceHeight / sourceWidth, 260.0f)
                : 0.0f;
        if (previewWidth > 0.0f && previewHeight > 0.0f) {
          ImGui::Image(ImTextureID(texture->GetId()),
                       {previewWidth, previewHeight}, {0, 1}, {1, 0});
        }
      }
    } else if (asset->type == managers::AssetType::Audio) {
      const auto clip = assetManager.GetAudioClip(asset->id);
      if (clip) {
        ImGui::Text("Duration: %.2f seconds", clip->GetDurationSeconds());
      }
    }
  }

  const auto& assetIssues = assetManager.GetIssues();
  if (!assetIssues.empty() &&
      ImGui::CollapsingHeader("Asset problems",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::TextColored({1.0f, 0.35f, 0.35f, 1.0f}, "%zu problem(s)",
                       assetIssues.size());
    for (const auto& issue : assetIssues) {
      ImGui::BulletText("%s: %s", issue.assetId.c_str(),
                        issue.message.c_str());
    }
  }
  ImGui::End();

  ImGui::Begin("Entities");
  auto& world = engine.GetWorld();
  if (ImGui::Button("Create entity")) {
    mSelectedEntity = world.CreateEntity();
    world.Add<components::Name>(mSelectedEntity);
    world.Add<components::Transform>(mSelectedEntity);
  }
  ImGui::SameLine();
  if (ImGui::Button("Duplicate") && world.IsAlive(mSelectedEntity)) {
    const auto original = mSelectedEntity;
    mSelectedEntity = world.CloneFrom(world, original);
    if (auto* camera = world.Get<components::Camera>(mSelectedEntity); camera && camera->followTarget == original)
      camera->followTarget = mSelectedEntity;
  }
  ImGui::SameLine();
  if (ImGui::Button("Delete") && world.IsAlive(mSelectedEntity)) {
    const auto deleted = mSelectedEntity;
    for (auto id : world.GetEntities())
      if (auto* camera = world.Get<components::Camera>(id); camera && camera->followTarget == deleted)
        camera->followTarget = ecs::NullEntity;
    world.DestroyEntity(deleted); world.FlushDestroyed(); mSelectedEntity = ecs::NullEntity;
  }
  ImGui::InputText("Prefab path", &mPrefabPath);
  if (ImGui::Button("Save selected prefab")) {
    mSceneStatus = engine.GetSceneManager().SavePrefab(mPrefabPath, world, mSelectedEntity, assetManager)
        ? "Prefab saved" : engine.GetSceneManager().LastError();
  }
  ImGui::SameLine();
  if (ImGui::Button("Instantiate prefab")) {
    mSelectedEntity = engine.GetSceneManager().Instantiate(mPrefabPath, world, assetManager);
    if (!mSelectedEntity) mSceneStatus = engine.GetSceneManager().LastError();
  }
  auto entities = world.GetEntities(); std::sort(entities.begin(), entities.end());
  for (const auto entity : entities) {
    const auto* name = world.Get<components::Name>(entity);
    const std::string label = (name ? name->value : "Entity") + " " + std::to_string(entity);
    if (ImGui::Selectable(label.c_str(), mSelectedEntity == entity))
      mSelectedEntity = entity;
  }
  if (world.IsAlive(mSelectedEntity)) {
    ImGui::Separator();
    if (auto* name = world.Get<components::Name>(mSelectedEntity)) {
      ImGui::InputText("Name", &name->value);
    }
    if (auto* transform = world.Get<components::Transform>(mSelectedEntity)) {
      ImGui::Text("Transform");
      ImGui::DragFloat2("Position", &transform->position.x, 1.0f);
      ImGui::DragFloat2("Scale", &transform->scale.x, 0.1f, 0.01f,
                        1000.0f);
      ImGui::DragFloat("Rotation", &transform->rotation, 0.01f);
    }
    if (auto* text = world.Get<components::TextRenderer>(mSelectedEntity)) {
      ImGui::Text("TextRenderer");
      ImGui::InputTextMultiline("Text", &text->text);
      ImGui::ColorEdit3("Color", &text->color.x);
      ImGui::DragFloat("Text scale", &text->scale, 0.05f, 0.05f, 20.0f);
      ImGui::InputInt("Text layer", &text->layer);
      if (ImGui::SmallButton("Remove TextRenderer")) {
        world.Remove<components::TextRenderer>(mSelectedEntity);
      }
    } else if (ImGui::SmallButton("Add TextRenderer")) {
      components::TextRenderer text;
      text.font = assetManager.GetFont("font.default", 32);
      text.shader = assetManager.GetShader("shader.sprite");
      text.text = "Text";
      world.Add<components::TextRenderer>(mSelectedEntity, std::move(text));
    }

    if (auto* body = world.Get<components::RigidBody2D>(mSelectedEntity)) {
      ImGui::Text("RigidBody2D");
      int type = static_cast<int>(body->type);
      const char* types[] = {"Static", "Kinematic", "Dynamic"};
      if (ImGui::Combo("Body type", &type, types, 3)) {
        body->type = static_cast<components::PhysicsBodyType>(type);
      }
      ImGui::DragFloat2("Linear velocity", &body->linearVelocity.x, 1.0f);
      ImGui::DragFloat("Angular velocity", &body->angularVelocity, 0.05f);
      ImGui::DragFloat("Linear damping", &body->linearDamping, 0.05f, 0.0f);
      ImGui::DragFloat("Angular damping", &body->angularDamping, 0.05f, 0.0f);
      ImGui::DragFloat("Gravity scale", &body->gravityScale, 0.05f);
      ImGui::Checkbox("Fixed rotation", &body->fixedRotation);
      ImGui::Checkbox("Bullet", &body->bullet);
      ImGui::Checkbox("Physics enabled", &body->enabled);
      if (ImGui::SmallButton("Remove RigidBody2D")) {
        world.Remove<components::RigidBody2D>(mSelectedEntity);
      }
    } else if (ImGui::SmallButton("Add RigidBody2D")) {
      world.Add<components::RigidBody2D>(mSelectedEntity);
    }

    const auto drawMaterial = [](components::PhysicsMaterial2D& material) {
      ImGui::DragFloat("Density", &material.density, 0.05f, 0.0f);
      ImGui::DragFloat("Friction", &material.friction, 0.05f, 0.0f);
      ImGui::DragFloat("Restitution", &material.restitution, 0.05f, 0.0f,
                       1.0f);
      ImGui::Checkbox("Sensor", &material.sensor);
      ImGui::InputScalar("Category bits", ImGuiDataType_U32,
                         &material.categoryBits);
      ImGui::InputScalar("Mask bits", ImGuiDataType_U32,
                         &material.maskBits);
      ImGui::InputInt("Group index", &material.groupIndex);
    };

    if (auto* box = world.Get<components::BoxCollider2D>(mSelectedEntity)) {
      ImGui::PushID("BoxCollider2D");
      ImGui::Text("BoxCollider2D");
      ImGui::DragFloat2("Physics box size", &box->size.x, 1.0f, 0.01f);
      ImGui::DragFloat2("Physics box offset", &box->offset.x, 1.0f);
      drawMaterial(box->material);
      if (ImGui::SmallButton("Remove BoxCollider2D")) {
        world.Remove<components::BoxCollider2D>(mSelectedEntity);
      }
      ImGui::PopID();
    } else if (ImGui::SmallButton("Add BoxCollider2D")) {
      world.Add<components::BoxCollider2D>(mSelectedEntity);
    }

    if (auto* circle =
            world.Get<components::CircleCollider2D>(mSelectedEntity)) {
      ImGui::PushID("CircleCollider2D");
      ImGui::Text("CircleCollider2D");
      ImGui::DragFloat("Physics circle radius", &circle->radius, 1.0f,
                       0.01f);
      ImGui::DragFloat2("Physics circle offset", &circle->offset.x, 1.0f);
      drawMaterial(circle->material);
      if (ImGui::SmallButton("Remove CircleCollider2D")) {
        world.Remove<components::CircleCollider2D>(mSelectedEntity);
      }
      ImGui::PopID();
    } else if (ImGui::SmallButton("Add CircleCollider2D")) {
      world.Add<components::CircleCollider2D>(mSelectedEntity);
    }
  }
  ImGui::End();

  // This canvas is intentionally black so the purple route is easy to see.
  // The same path vector can later drive an NPC's movement target by target.
  ImGui::SetNextWindowSize(ImVec2{620.0f, 500.0f}, ImGuiCond_FirstUseEver);
  if (ImGui::Begin("A* Pathfinding Demo")) {
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const float cell = std::min(
        available.x / static_cast<float>(mDemoGrid.width),
        available.y / static_cast<float>(mDemoGrid.height));
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    auto* draw = ImGui::GetWindowDrawList();
    const ImVec2 canvasEnd = {origin.x + cell * mDemoGrid.width,
                              origin.y + cell * mDemoGrid.height};
    draw->AddRectFilled(origin, canvasEnd, IM_COL32(3, 3, 8, 255));

    for (int y = 0; y < mDemoGrid.height; ++y) {
      for (int x = 0; x < mDemoGrid.width; ++x) {
        const ImVec2 min = {origin.x + x * cell + 1.0f,
                            origin.y + y * cell + 1.0f};
        const ImVec2 max = {origin.x + (x + 1) * cell - 1.0f,
                            origin.y + (y + 1) * cell - 1.0f};
        if (mDemoGrid.IsBlocked(x, y)) {
          draw->AddRectFilled(min, max, IM_COL32(35, 35, 48, 255));
        } else {
          draw->AddRect(min, max, IM_COL32(20, 20, 35, 255));
        }
      }
    }

    const auto& demoFrame = mDemoFrames[mDemoFrame];
    for (const auto cellPosition : demoFrame.visited) {
      const ImVec2 min = {origin.x + cellPosition.x * cell + 3.0f,
                          origin.y + cellPosition.y * cell + 3.0f};
      const ImVec2 max = {origin.x + (cellPosition.x + 1) * cell - 3.0f,
                          origin.y + (cellPosition.y + 1) * cell - 3.0f};
      draw->AddRectFilled(min, max, IM_COL32(70, 30, 110, 255));
    }
    for (const auto cellPosition : demoFrame.frontier) {
      const ImVec2 min = {origin.x + cellPosition.x * cell + 4.0f,
                          origin.y + cellPosition.y * cell + 4.0f};
      const ImVec2 max = {origin.x + (cellPosition.x + 1) * cell - 4.0f,
                          origin.y + (cellPosition.y + 1) * cell - 4.0f};
      draw->AddRectFilled(min, max, IM_COL32(80, 120, 220, 255));
    }
    for (const auto cellPosition : demoFrame.path) {
      const ImVec2 min = {origin.x + cellPosition.x * cell + 3.0f,
                          origin.y + cellPosition.y * cell + 3.0f};
      const ImVec2 max = {origin.x + (cellPosition.x + 1) * cell - 3.0f,
                          origin.y + (cellPosition.y + 1) * cell - 3.0f};
      draw->AddRectFilled(min, max, IM_COL32(170, 65, 255, 255));
    }

    const auto drawMarker = [&](glm::ivec2 position, ImU32 color) {
      const ImVec2 min = {origin.x + position.x * cell + 2.0f,
                          origin.y + position.y * cell + 2.0f};
      const ImVec2 max = {origin.x + (position.x + 1) * cell - 2.0f,
                          origin.y + (position.y + 1) * cell - 2.0f};
      draw->AddRectFilled(min, max, color);
    };
    drawMarker({1, 1}, IM_COL32(60, 220, 120, 255));
    drawMarker({18, 12}, IM_COL32(255, 80, 150, 255));
    ImGui::Dummy({cell * mDemoGrid.width, cell * mDemoGrid.height});
    ImGui::Text("Blue: frontier   Violet: visited   Purple: final route");
    if (demoFrame.finished) {
      ImGui::Text("Path found: %zu steps", demoFrame.path.size());
    } else {
      ImGui::Text("Searching...");
    }
  }
  ImGui::End();

  // The GameView shows the engine framebuffer inside an editor window.
  ImGui::SetNextWindowSize(ImVec2{1000.0f, 760.0f}, ImGuiCond_Always);
  if (ImGui::Begin("GameView")) {
    auto& window = engine.GetWindow();
    const auto windowSize = window.GetSize();
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const float aspect = static_cast<float>(windowSize.x) /
                         static_cast<float>(windowSize.y);
    ImVec2 size = available;
    if (available.x / available.y > aspect)
      size.x = available.y * aspect;
    else
      size.y = available.x / aspect;
    const ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos({cursor.x + (available.x - size.x) * 0.5f,
                         cursor.y + (available.y - size.y) * 0.5f});
    ImGui::Image(ImTextureID(window.GetFrameBuffer()->GetTextureId()), size,
                 {0, 1}, {1, 0});
  }
  ImGui::End();
}

} // namespace eclipse::editor
