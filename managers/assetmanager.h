#pragma once

#include "../core/audioclip.h"
#include "../graphics/shader.h"
#include "../graphics/texture.h"
#include "../graphics/font.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace eclipse::managers {

enum class AssetType {
  Texture,
  Shader,
  Audio,
  Font,
};

struct AssetInfo {
  std::string id;
  AssetType type = AssetType::Texture;
  std::string group = "global";
  std::filesystem::path path;
  std::filesystem::path vertexPath;
  std::filesystem::path fragmentPath;
  std::filesystem::path licensePath;
  graphics::TextureFilter textureFilter = graphics::TextureFilter::Linear;
};

struct AssetIssue {
  std::string assetId;
  std::string message;
};

const char* AssetTypeName(AssetType type);

class AssetManager {
public:
  bool Initialize(const std::filesystem::path& assetRoot,
                  const std::filesystem::path& manifest = "manifest.json");
  bool ReloadManifest();
  bool Validate();

  bool HasAsset(const std::string& id) const;
  const AssetInfo* FindAsset(const std::string& id) const;
  std::filesystem::path ResolveAssetPath(const std::string& id) const;

  std::shared_ptr<graphics::Texture> GetTexture(const std::string& id);
  std::shared_ptr<graphics::Shader> GetShader(const std::string& id);
  std::shared_ptr<core::AudioClip> GetAudioClip(const std::string& id);
  std::shared_ptr<graphics::Font> GetFont(const std::string& id,
                                          std::uint32_t pixelHeight = 48);
  std::string FindTextureId(
      const std::shared_ptr<graphics::Texture>& texture) const;
  std::string FindShaderId(
      const std::shared_ptr<graphics::Shader>& shader) const;
  std::string FindFontId(const std::shared_ptr<graphics::Font>& font) const;

  // File loading is reserved for importers such as TMX, where referenced
  // dependencies are discovered inside another validated asset.
  std::shared_ptr<graphics::Texture> LoadTextureFile(
      const std::filesystem::path& path,
      graphics::TextureFilter filter = graphics::TextureFilter::Linear);

  void UnloadGroup(const std::string& group);
  void UnloadUnused();
  void ClearLoaded();
  void Clear();

  const std::filesystem::path& GetAssetRoot() const { return mAssetRoot; }
  const std::vector<AssetInfo>& GetAssets() const { return mAssets; }
  const std::vector<AssetIssue>& GetIssues() const { return mIssues; }

private:
  bool ParseManifest(const std::filesystem::path& manifestPath);
  bool ResolvePath(const std::filesystem::path& path,
                   std::filesystem::path& resolved) const;
  void RecordIssue(const std::string& assetId, const std::string& message);

  std::filesystem::path mAssetRoot;
  std::filesystem::path mManifestPath;
  std::vector<AssetInfo> mAssets;
  std::unordered_map<std::string, std::size_t> mAssetIndexes;
  std::vector<AssetIssue> mIssues;
  std::unordered_map<std::string, std::shared_ptr<graphics::Texture>> mTextures;
  std::unordered_map<std::string, std::shared_ptr<graphics::Shader>> mShaders;
  std::unordered_map<std::string, std::shared_ptr<core::AudioClip>> mAudioClips;
  std::unordered_map<std::string, std::shared_ptr<graphics::Font>> mFonts;
};

} // namespace eclipse::managers
