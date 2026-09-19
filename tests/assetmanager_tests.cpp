#include "../managers/assetmanager.h"
#include "../managers/scenemanager.h"
#include "../components/gameplaycomponents.h"
#include "../components/rendercomponents.h"
#include "../input/actions.h"
#include "../input/keyboard.h"
#include "../systems/physics_system.h"

#include <chrono>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>

namespace {

int failures = 0;

void Check(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    ++failures;
  }
}

std::filesystem::path MakeTemporaryDirectory() {
  const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto path = std::filesystem::temp_directory_path() /
                    ("eclipse-assets-" + std::to_string(suffix));
  std::filesystem::create_directories(path);
  return path;
}

void WriteManifest(const std::filesystem::path& root,
                   const std::string& document) {
  std::ofstream file(root / "manifest.json");
  file << document;
}

void WriteTestWav(const std::filesystem::path& path) {
  constexpr std::array<std::uint8_t, 52> wav{
      'R', 'I', 'F', 'F', 44,  0,   0,   0,   'W', 'A', 'V', 'E', 'f',
      'm', 't', ' ',  16,   0,   0,   0,   1,   0,   1,   0,   64,  31,
      0,   0,   128,  62,   0,   0,   2,   0,   16,  0,   'd', 'a', 't',
      'a', 8,   0,    0,    0,   0,   232, 3,   24,  252, 0,   0};
  std::ofstream file(path, std::ios::binary);
  file.write(reinterpret_cast<const char*>(wav.data()),
             static_cast<std::streamsize>(wav.size()));
}

void TestProjectManifest() {
  eclipse::managers::AssetManager assets;
  Check(assets.Initialize("assets"), "project manifest should validate");
  Check(assets.GetIssues().empty(), "project manifest should have no issues");
  Check(assets.HasAsset("shader.sprite"), "sprite shader should be registered");
  Check(assets.HasAsset("character.schoolgirl1.idle"),
        "player idle texture should be registered");
  Check(assets.HasAsset("font.default"),
        "default font should be registered");
  const auto* font = assets.FindAsset("font.default");
  Check(font && font->type == eclipse::managers::AssetType::Font,
        "font manifests should preserve their asset type");

  const auto resolved =
      assets.ResolveAssetPath("character.schoolgirl1.idle");
  Check(std::filesystem::is_regular_file(resolved),
        "texture ID should resolve to an existing file");
  Check(assets.ResolveAssetPath("shader.sprite").empty(),
        "shader should not resolve as a single-path asset");
}

void TestDuplicateIdsAreRejected() {
  const auto root = MakeTemporaryDirectory();
  std::ofstream(root / "one.png").put('x');
  WriteManifest(
      root,
      R"({"version":1,"assets":[
        {"id":"duplicate","type":"texture","path":"one.png"},
        {"id":"duplicate","type":"texture","path":"one.png"}
      ]})");

  eclipse::managers::AssetManager assets;
  Check(!assets.Initialize(root), "duplicate IDs should invalidate a manifest");
  Check(!assets.GetIssues().empty(), "duplicate IDs should report an issue");
  std::filesystem::remove_all(root);
}

void TestPathsCannotEscapeRoot() {
  const auto root = MakeTemporaryDirectory();
  WriteManifest(
      root,
      R"({"version":1,"assets":[
        {"id":"escape","type":"texture","path":"../outside.png"}
      ]})");

  eclipse::managers::AssetManager assets;
  Check(!assets.Initialize(root), "escaping paths should invalidate a manifest");
  Check(assets.ResolveAssetPath("escape").empty(),
        "escaping paths should never resolve");
  std::filesystem::remove_all(root);
}

void TestAudioIsDecodedAndCached() {
  const auto root = MakeTemporaryDirectory();
  WriteTestWav(root / "tone.wav");
  WriteManifest(
      root,
      R"({"version":1,"assets":[
        {"id":"audio.tone","type":"audio","path":"tone.wav"}
      ]})");

  eclipse::managers::AssetManager assets;
  Check(assets.Initialize(root), "audio manifest should validate");
  const auto first = assets.GetAudioClip("audio.tone");
  const auto second = assets.GetAudioClip("audio.tone");
  Check(first && first->IsLoaded(), "WAV data should decode");
  Check(first == second, "audio clips should be cached by asset ID");
  Check(first && !first->GetData().empty(), "decoded audio should contain data");
  Check(first && first->GetFormat().freq == 48000,
        "audio should convert to the engine sample rate");
  std::filesystem::remove_all(root);
}

void TestUnknownTypesAreRejected() {
  const auto root = MakeTemporaryDirectory();
  WriteManifest(
      root,
      R"({"version":1,"assets":[
        {"id":"bad","type":"model","path":"model.obj"}
      ]})");

  eclipse::managers::AssetManager assets;
  Check(!assets.Initialize(root), "unknown asset types should be rejected");
  Check(!assets.HasAsset("bad"), "invalid entries should not be registered");
  std::filesystem::remove_all(root);
}

void TestActionBindings() {
  using eclipse::input::Action;
  using eclipse::input::BindKey;
  using eclipse::input::ClearActionBindings;
  using eclipse::input::GetActionBindings;
  using eclipse::input::ResetActionBindings;
  using eclipse::input::UnbindKey;

  ResetActionBindings();
  Check(GetActionBindings(Action::MoveUp).size() == 2,
        "movement actions should have keyboard and arrow defaults");
  Check(GetActionBindings(Action::Jump).size() == 1,
        "jump should have a default binding");

  ClearActionBindings(Action::Jump);
  Check(GetActionBindings(Action::Jump).empty(),
        "action bindings should be clearable");
  Check(BindKey(Action::Jump, eclipse::input::ECLIPSE_INPUT_KEY_J),
        "valid keys should be bindable");
  Check(BindKey(Action::Jump, eclipse::input::ECLIPSE_INPUT_KEY_J) &&
            GetActionBindings(Action::Jump).size() == 1,
        "duplicate bindings should not be added twice");
  Check(!BindKey(Action::Jump, -1), "invalid keys should be rejected");
  Check(UnbindKey(Action::Jump, eclipse::input::ECLIPSE_INPUT_KEY_J) &&
            GetActionBindings(Action::Jump).empty(),
        "individual keys should be removable");
  Check(!UnbindKey(Action::Jump, eclipse::input::ECLIPSE_INPUT_KEY_J),
        "removing a missing binding should report failure");

  ResetActionBindings();
}

void TestPhysics() {
  eclipse::ecs::World world;

  const auto falling = world.CreateEntity();
  world.Add<eclipse::components::Transform>(
      falling, eclipse::components::Transform{{0.0f, 0.0f}});
  world.Add<eclipse::components::RigidBody2D>(falling);
  world.Add<eclipse::components::CircleCollider2D>(
      falling, eclipse::components::CircleCollider2D{10.0f});

  const auto sensor = world.CreateEntity();
  world.Add<eclipse::components::Transform>(
      sensor, eclipse::components::Transform{{0.0f, 100.0f}});
  eclipse::components::RigidBody2D sensorBody;
  sensorBody.type = eclipse::components::PhysicsBodyType::Static;
  world.Add<eclipse::components::RigidBody2D>(sensor, sensorBody);
  eclipse::components::BoxCollider2D sensorShape;
  sensorShape.size = {200.0f, 10.0f};
  sensorShape.material.sensor = true;
  world.Add<eclipse::components::BoxCollider2D>(sensor, sensorShape);

  const auto floor = world.CreateEntity();
  world.Add<eclipse::components::Transform>(
      floor, eclipse::components::Transform{{0.0f, 200.0f}});
  eclipse::components::RigidBody2D floorBody;
  floorBody.type = eclipse::components::PhysicsBodyType::Static;
  world.Add<eclipse::components::RigidBody2D>(floor, floorBody);
  world.Add<eclipse::components::BoxCollider2D>(
      floor, eclipse::components::BoxCollider2D{{1000.0f, 20.0f}});

  eclipse::systems::PhysicsSystem physics;
  bool sensorBegan = false;
  bool sensorEnded = false;
  bool contactBegan = false;
  for (int frame = 0; frame < 240; ++frame) {
    physics.Update(world, 1.0f / 60.0f);
    for (const auto& event : physics.GetContactEvents()) {
      sensorBegan = sensorBegan ||
          (event.type == eclipse::systems::PhysicsContactType::Begin &&
           event.sensor &&
           (event.first == sensor || event.second == sensor));
      sensorEnded = sensorEnded ||
          (event.type == eclipse::systems::PhysicsContactType::End &&
           event.sensor &&
           (event.first == sensor || event.second == sensor));
      contactBegan = contactBegan ||
          (event.type == eclipse::systems::PhysicsContactType::Begin &&
           !event.sensor &&
           (event.first == floor || event.second == floor));
    }
  }

  const auto* transform = world.Get<eclipse::components::Transform>(falling);
  Check(transform && transform->position.y > 175.0f &&
            transform->position.y < 185.0f,
        "Box2D bodies should settle on static geometry in engine units");
  Check(sensorBegan, "Box2D sensors should produce begin events");
  Check(sensorEnded, "Box2D sensors should produce end events");
  Check(contactBegan, "Box2D contacts should produce begin events");
  Check(physics.ApplyImpulse(falling, {0.0f, -100.0f}),
        "physics impulses should be applicable to active bodies");
  const float settledY = transform ? transform->position.y : 0.0f;
  physics.Update(world, 1.0f / 60.0f);
  transform = world.Get<eclipse::components::Transform>(falling);
  Check(transform && transform->position.y < settledY,
        "physics impulses should affect simulation velocity");
}

void TestSceneRoundTrip() {
  const auto root = MakeTemporaryDirectory();
  const auto scenePath = root / "round-trip.scene.json";
  eclipse::ecs::World source;
  Check(source.RestoreEntity(2), "scene test should restore entity 2");
  Check(source.RestoreEntity(7), "scene test should restore entity 7");

  eclipse::components::Transform transform;
  transform.position = {12.5f, -4.0f};
  transform.previousPosition = {10.0f, -4.0f};
  transform.rotation = 0.75f;
  transform.scale = {2.0f, 3.0f};
  source.Add<eclipse::components::Transform>(2, transform);
  source.Add<eclipse::components::PlayerController>(
      2, eclipse::components::PlayerController{321.0f});
  source.Add<eclipse::components::Name>(
      2, eclipse::components::Name{"Player"});
  source.Add<eclipse::components::Collider>(
      2, eclipse::components::Collider{{4.0f, 5.0f}, {1.0f, 2.0f}, true});
  eclipse::components::RigidBody2D rigidBody;
  rigidBody.type = eclipse::components::PhysicsBodyType::Kinematic;
  rigidBody.linearVelocity = {10.0f, 20.0f};
  rigidBody.fixedRotation = true;
  source.Add<eclipse::components::RigidBody2D>(2, rigidBody);
  eclipse::components::BoxCollider2D physicsBox;
  physicsBox.size = {20.0f, 30.0f};
  physicsBox.material.friction = 0.25f;
  physicsBox.material.categoryBits = 4;
  source.Add<eclipse::components::BoxCollider2D>(2, physicsBox);
  source.Add<eclipse::components::Animation>(
      2, eclipse::components::Animation{8, 4, 2, 0.2f, 0.1f, 3});
  source.Add<eclipse::components::NpcBehavior>(
      7, eclipse::components::NpcBehavior{{1.0f, 2.0f}, {8.0f, 9.0f},
                                          42.0f, false});
  eclipse::components::Camera camera;
  camera.zoom = 1.5f;
  camera.fitSize = {640.0f, 360.0f};
  camera.fitScale = 1.25f;
  camera.followTarget = 2;
  source.Add<eclipse::components::Camera>(7, camera);
  eclipse::components::CircleCollider2D physicsCircle;
  physicsCircle.radius = 7.0f;
  physicsCircle.material.sensor = true;
  source.Add<eclipse::components::CircleCollider2D>(7, physicsCircle);

  eclipse::managers::AssetManager assets;
  eclipse::managers::SceneManager scenes;
  Check(scenes.SaveFile(scenePath, source, assets),
        "scene should save data components");

  eclipse::ecs::World loaded;
  Check(scenes.LoadFile(scenePath, loaded, assets),
        "saved scene should load");
  Check(loaded.IsAlive(2) && loaded.IsAlive(7),
        "scene should preserve entity IDs");
  const auto* loadedTransform =
      loaded.Get<eclipse::components::Transform>(2);
  Check(loadedTransform && loadedTransform->position.x == 12.5f &&
            loadedTransform->scale.y == 3.0f,
        "scene should preserve transforms");
  const auto* loadedCollider =
      loaded.Get<eclipse::components::Collider>(2);
  Check(loadedCollider && loadedCollider->isStatic &&
            loadedCollider->offset.y == 2.0f,
        "scene should preserve colliders");
  const auto* loadedBody = loaded.Get<eclipse::components::RigidBody2D>(2);
  const auto* loadedBox = loaded.Get<eclipse::components::BoxCollider2D>(2);
  const auto* loadedCircle = loaded.Get<eclipse::components::CircleCollider2D>(7);
  Check(loadedBody &&
            loadedBody->type == eclipse::components::PhysicsBodyType::Kinematic &&
            loadedBody->fixedRotation && loadedBody->linearVelocity.y == 20.0f,
        "scene should preserve rigid bodies");
  Check(loadedBox && loadedBox->size.y == 30.0f &&
            loadedBox->material.categoryBits == 4,
        "scene should preserve physics boxes and filters");
  Check(loadedCircle && loadedCircle->radius == 7.0f &&
            loadedCircle->material.sensor,
        "scene should preserve physics circles and sensors");
  const auto* loadedName = loaded.Get<eclipse::components::Name>(2);
  Check(loadedName && loadedName->value == "Player",
        "scene should preserve entity names");
  const auto* loadedAnimation =
      loaded.Get<eclipse::components::Animation>(2);
  Check(loadedAnimation && loadedAnimation->currentFrame == 3 &&
            loadedAnimation->columns == 4,
        "scene should preserve animations");
  const auto* loadedCamera = loaded.Get<eclipse::components::Camera>(7);
  Check(loadedCamera && loadedCamera->followTarget == 2 &&
            loadedCamera->fitSize.x == 640.0f,
        "scene should preserve cameras and entity references");
  Check(loaded.CreateEntity() == 8,
        "new IDs should continue after restored scene IDs");

  std::ofstream(scenePath) << "{not valid json";
  Check(!scenes.LoadFile(scenePath, loaded, assets),
        "invalid scene JSON should be rejected");
  Check(loaded.IsAlive(2) && loaded.IsAlive(7),
        "failed scene loads should preserve the active world");
  std::filesystem::remove_all(root);
}

} // namespace

int main() {
  TestProjectManifest();
  TestDuplicateIdsAreRejected();
  TestPathsCannotEscapeRoot();
  TestUnknownTypesAreRejected();
  TestAudioIsDecodedAndCached();
  TestActionBindings();
  TestPhysics();
  TestSceneRoundTrip();
  if (failures != 0) {
    std::cerr << failures << " test(s) failed\n";
    return 1;
  }
  std::cout << "All engine tests passed\n";
  return 0;
}
