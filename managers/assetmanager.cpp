#include "assetmanager.h"

#include "../external/picojson.h"
#include "../src/log.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace {

using JsonObject = picojson::object;

const picojson::value* FindMember(const JsonObject& object,
                                  const std::string& name) {
  const auto it = object.find(name);
  return it == object.end() ? nullptr : &it->second;
}

bool ReadString(const JsonObject& object, const std::string& name,
                std::string& value) {
  const auto* member = FindMember(object, name);
  if (!member || !member->is<std::string>()) {
    return false;
  }
  value = member->get<std::string>();
  return true;
}

bool IsValidAssetId(const std::string& id) {
  return !id.empty() &&
         std::all_of(id.begin(), id.end(), [](unsigned char character) {
           return std::isalnum(character) || character == '.' ||
                  character == '_' || character == '-' || character == '/';
         });
}

std::string ReadTextFile(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file) {
    return {};
  }
  std::stringstream contents;
  contents << file.rdbuf();
  return contents.str();
}

} // namespace

namespace eclipse::managers {

const char* AssetTypeName(AssetType type) {
  switch (type) {
  case AssetType::Texture:
    return "texture";
  case AssetType::Shader:
    return "shader";
  case AssetType::Audio:
    return "audio";
  case AssetType::Font:
    return "font";
  }
  return "unknown";
}

bool AssetManager::Initialize(const std::filesystem::path& assetRoot,
                              const std::filesystem::path& manifest) {
  Clear();

  std::error_code error;
  mAssetRoot = std::filesystem::weakly_canonical(
      std::filesystem::absolute(assetRoot, error), error);
  if (error || !std::filesystem::is_directory(mAssetRoot)) {
    RecordIssue("", "Asset root does not exist: " + assetRoot.string());
    return false;
  }

  if (!ResolvePath(manifest, mManifestPath)) {
    RecordIssue("", "Manifest escapes the asset root: " + manifest.string());
    return false;
  }

  const bool parsed = ParseManifest(mManifestPath);
  const bool valid = Validate();
  if (parsed && valid) {
    ECLIPSE_INFO("Loaded asset manifest '{}' with {} assets",
                 mManifestPath.string(), mAssets.size());
  }
  return parsed && valid;
}

bool AssetManager::ReloadManifest() {
  if (mAssetRoot.empty() || mManifestPath.empty()) {
    RecordIssue("", "Cannot reload an asset manifest before initialization");
    return false;
  }

  ClearLoaded();
  mAssets.clear();
  mAssetIndexes.clear();
  mIssues.clear();
  const bool parsed = ParseManifest(mManifestPath);
  return parsed && Validate();
}

bool AssetManager::ParseManifest(const std::filesystem::path& manifestPath) {
  const std::string document = ReadTextFile(manifestPath);
  if (document.empty()) {
    RecordIssue("", "Unable to read asset manifest: " + manifestPath.string());
    return false;
  }

  picojson::value root;
  const std::string parseError = picojson::parse(root, document);
  if (!parseError.empty()) {
    RecordIssue("", "Invalid asset manifest JSON: " + parseError);
    return false;
  }
  if (!root.is<JsonObject>()) {
    RecordIssue("", "Asset manifest root must be an object");
    return false;
  }

  const auto& rootObject = root.get<JsonObject>();
  const auto* version = FindMember(rootObject, "version");
  if (!version || !version->is<double>() || version->get<double>() != 1.0) {
    RecordIssue("", "Asset manifest version must be 1");
    return false;
  }

  const auto* assets = FindMember(rootObject, "assets");
  if (!assets || !assets->is<picojson::array>()) {
    RecordIssue("", "Asset manifest must contain an assets array");
    return false;
  }

  bool valid = true;
  for (const auto& value : assets->get<picojson::array>()) {
    if (!value.is<JsonObject>()) {
      RecordIssue("", "Every asset entry must be an object");
      valid = false;
      continue;
    }

    const auto& object = value.get<JsonObject>();
    AssetInfo info;
    std::string type;
    std::string path;
    if (!ReadString(object, "id", info.id) || !IsValidAssetId(info.id)) {
      RecordIssue(info.id, "Asset has a missing or invalid id");
      valid = false;
      continue;
    }
    if (mAssetIndexes.find(info.id) != mAssetIndexes.end()) {
      RecordIssue(info.id, "Duplicate asset id");
      valid = false;
      continue;
    }
    if (!ReadString(object, "type", type)) {
      RecordIssue(info.id, "Asset type is required");
      valid = false;
      continue;
    }

    std::string group;
    if (ReadString(object, "group", group) && !group.empty()) {
      info.group = group;
    }

    bool entryValid = true;
    if (type == "texture") {
      info.type = AssetType::Texture;
      if (!ReadString(object, "path", path) || path.empty()) {
        RecordIssue(info.id, "Texture path is required");
        entryValid = false;
      } else {
        info.path = path;
      }
      std::string filter;
      if (ReadString(object, "filter", filter)) {
        if (filter == "nearest") {
          info.textureFilter = graphics::TextureFilter::Nearest;
        } else if (filter == "linear") {
          info.textureFilter = graphics::TextureFilter::Linear;
        } else {
          RecordIssue(info.id, "Texture filter must be nearest or linear");
          entryValid = false;
        }
      }
    } else if (type == "shader") {
      info.type = AssetType::Shader;
      std::string vertex;
      std::string fragment;
      if (!ReadString(object, "vertex", vertex) || vertex.empty() ||
          !ReadString(object, "fragment", fragment) || fragment.empty()) {
        RecordIssue(info.id, "Shader vertex and fragment paths are required");
        entryValid = false;
      } else {
        info.vertexPath = vertex;
        info.fragmentPath = fragment;
      }
    } else if (type == "audio") {
      info.type = AssetType::Audio;
      if (!ReadString(object, "path", path) || path.empty()) {
        RecordIssue(info.id, "Audio path is required");
        entryValid = false;
      } else {
        info.path = path;
      }
    } else if (type == "font") {
      info.type = AssetType::Font;
      if (!ReadString(object, "path", path) || path.empty()) {
        RecordIssue(info.id, "Font path is required");
        entryValid = false;
      } else {
        info.path = path;
      }
      std::string license;
      if (ReadString(object, "license", license) && !license.empty()) {
        info.licensePath = license;
      }
    } else {
      RecordIssue(info.id, "Unsupported asset type: " + type);
      entryValid = false;
    }

    if (!entryValid) {
      valid = false;
      continue;
    }
    mAssetIndexes.emplace(info.id, mAssets.size());
    mAssets.push_back(std::move(info));
  }

  return valid;
}

bool AssetManager::Validate() {
  bool valid = mIssues.empty();
  for (const auto& asset : mAssets) {
    const auto validateFile = [this, &asset, &valid](
                                  const std::filesystem::path& path,
                                  const char* label) {
      std::filesystem::path resolved;
      if (!ResolvePath(path, resolved)) {
        RecordIssue(asset.id, std::string(label) + " escapes the asset root");
        valid = false;
        return;
      }
      if (!std::filesystem::is_regular_file(resolved)) {
        RecordIssue(asset.id,
                    std::string(label) + " does not exist: " + path.string());
        valid = false;
      }
    };

    if (asset.type == AssetType::Shader) {
      validateFile(asset.vertexPath, "Vertex shader");
      validateFile(asset.fragmentPath, "Fragment shader");
    } else {
      validateFile(asset.path, AssetTypeName(asset.type));
      if (asset.type == AssetType::Font && !asset.licensePath.empty()) {
        validateFile(asset.licensePath, "Font license");
      }
    }
  }
  return valid;
}

bool AssetManager::HasAsset(const std::string& id) const {
  return mAssetIndexes.find(id) != mAssetIndexes.end();
}

const AssetInfo* AssetManager::FindAsset(const std::string& id) const {
  const auto it = mAssetIndexes.find(id);
  return it == mAssetIndexes.end() ? nullptr : &mAssets[it->second];
}

std::filesystem::path AssetManager::ResolveAssetPath(
    const std::string& id) const {
  const auto* asset = FindAsset(id);
  if (!asset || asset->type == AssetType::Shader) {
    return {};
  }
  std::filesystem::path resolved;
  return ResolvePath(asset->path, resolved) ? resolved
                                             : std::filesystem::path{};
}

std::shared_ptr<graphics::Texture> AssetManager::GetTexture(
    const std::string& id) {
  if (const auto cached = mTextures.find(id); cached != mTextures.end()) {
    return cached->second;
  }
  const auto* asset = FindAsset(id);
  if (!asset || asset->type != AssetType::Texture) {
    RecordIssue(id, "Unknown texture asset");
    return nullptr;
  }

  std::filesystem::path path;
  if (!ResolvePath(asset->path, path)) {
    RecordIssue(id, "Texture path escapes the asset root");
    return nullptr;
  }
  auto texture =
      std::make_shared<graphics::Texture>(path.string(), asset->textureFilter);
  if (!texture->IsLoadedFromSource()) {
    RecordIssue(id, "Texture could not be decoded: " + asset->path.string());
  }
  mTextures.emplace(id, texture);
  return texture;
}

std::shared_ptr<graphics::Shader> AssetManager::GetShader(
    const std::string& id) {
  if (const auto cached = mShaders.find(id); cached != mShaders.end()) {
    return cached->second;
  }
  const auto* asset = FindAsset(id);
  if (!asset || asset->type != AssetType::Shader) {
    RecordIssue(id, "Unknown shader asset");
    return nullptr;
  }

  std::filesystem::path vertexPath;
  std::filesystem::path fragmentPath;
  if (!ResolvePath(asset->vertexPath, vertexPath) ||
      !ResolvePath(asset->fragmentPath, fragmentPath)) {
    RecordIssue(id, "Shader path escapes the asset root");
    return nullptr;
  }
  const auto vertexSource = ReadTextFile(vertexPath);
  const auto fragmentSource = ReadTextFile(fragmentPath);
  if (vertexSource.empty() || fragmentSource.empty()) {
    RecordIssue(id, "Shader source could not be read");
    return nullptr;
  }

  auto shader =
      std::make_shared<graphics::Shader>(vertexSource, fragmentSource);
  if (!shader->IsValid()) {
    RecordIssue(id, "Shader compilation or linking failed");
    return nullptr;
  }
  mShaders.emplace(id, shader);
  return shader;
}

std::shared_ptr<core::AudioClip> AssetManager::GetAudioClip(
    const std::string& id) {
  if (const auto cached = mAudioClips.find(id); cached != mAudioClips.end()) {
    return cached->second;
  }
  const auto* asset = FindAsset(id);
  if (!asset || asset->type != AssetType::Audio) {
    RecordIssue(id, "Unknown audio asset");
    return nullptr;
  }

  std::filesystem::path path;
  if (!ResolvePath(asset->path, path)) {
    RecordIssue(id, "Audio path escapes the asset root");
    return nullptr;
  }
  auto clip = std::make_shared<core::AudioClip>(path.string());
  if (!clip->IsLoaded()) {
    RecordIssue(id, "Audio could not be decoded: " + asset->path.string());
    return nullptr;
  }
  mAudioClips.emplace(id, clip);
  return clip;
}

std::shared_ptr<graphics::Font> AssetManager::GetFont(
    const std::string& id, std::uint32_t pixelHeight) {
  const std::string key = id + "#" + std::to_string(pixelHeight);
  if (const auto cached = mFonts.find(key); cached != mFonts.end()) {
    return cached->second;
  }
  const auto* asset = FindAsset(id);
  if (!asset || asset->type != AssetType::Font) {
    RecordIssue(id, "Unknown font asset");
    return nullptr;
  }

  std::filesystem::path path;
  if (!ResolvePath(asset->path, path)) {
    RecordIssue(id, "Font path escapes the asset root");
    return nullptr;
  }
  auto font = std::make_shared<graphics::Font>(path.string(), pixelHeight);
  if (!font->IsLoaded()) {
    RecordIssue(id, "Font could not be loaded: " + asset->path.string());
    return nullptr;
  }
  mFonts.emplace(key, font);
  return font;
}

std::string AssetManager::FindTextureId(
    const std::shared_ptr<graphics::Texture>& texture) const {
  if (!texture) {
    return {};
  }
  for (const auto& [id, loaded] : mTextures) {
    const auto* asset = FindAsset(id);
    if (loaded == texture && asset && asset->type == AssetType::Texture) {
      return id;
    }
  }
  return {};
}

std::string AssetManager::FindShaderId(
    const std::shared_ptr<graphics::Shader>& shader) const {
  if (!shader) {
    return {};
  }
  for (const auto& [id, loaded] : mShaders) {
    const auto* asset = FindAsset(id);
    if (loaded == shader && asset && asset->type == AssetType::Shader) {
      return id;
    }
  }
  return {};
}

std::string AssetManager::FindFontId(
    const std::shared_ptr<graphics::Font>& font) const {
  if (!font) {
    return {};
  }
  for (const auto& [key, loaded] : mFonts) {
    if (loaded != font) {
      continue;
    }
    const auto separator = key.rfind('#');
    const std::string id = key.substr(0, separator);
    const auto* asset = FindAsset(id);
    if (asset && asset->type == AssetType::Font) {
      return id;
    }
  }
  return {};
}

std::shared_ptr<graphics::Texture> AssetManager::LoadTextureFile(
    const std::filesystem::path& path, graphics::TextureFilter filter) {
  std::filesystem::path resolved;
  if (!ResolvePath(path, resolved)) {
    RecordIssue(path.string(), "Imported texture escapes the asset root");
    return nullptr;
  }
  const std::string key = resolved.generic_string() +
                          (filter == graphics::TextureFilter::Nearest
                               ? "#nearest"
                               : "#linear");
  if (const auto cached = mTextures.find(key); cached != mTextures.end()) {
    return cached->second;
  }
  auto texture = std::make_shared<graphics::Texture>(resolved.string(), filter);
  if (!texture->IsLoadedFromSource()) {
    RecordIssue(path.string(), "Imported texture could not be decoded");
  }
  mTextures.emplace(key, texture);
  return texture;
}

void AssetManager::UnloadGroup(const std::string& group) {
  for (const auto& asset : mAssets) {
    if (asset.group != group) {
      continue;
    }
    mTextures.erase(asset.id);
    mShaders.erase(asset.id);
    mAudioClips.erase(asset.id);
    const std::string fontPrefix = asset.id + "#";
    for (auto it = mFonts.begin(); it != mFonts.end();) {
      if (it->first.compare(0, fontPrefix.size(), fontPrefix) == 0) {
        it = mFonts.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void AssetManager::UnloadUnused() {
  const auto eraseUnused = [](auto& cache) {
    for (auto it = cache.begin(); it != cache.end();) {
      if (it->second.use_count() == 1) {
        it = cache.erase(it);
      } else {
        ++it;
      }
    }
  };
  eraseUnused(mTextures);
  eraseUnused(mShaders);
  eraseUnused(mAudioClips);
  eraseUnused(mFonts);
}

void AssetManager::ClearLoaded() {
  mAudioClips.clear();
  mFonts.clear();
  mShaders.clear();
  mTextures.clear();
}

void AssetManager::Clear() {
  ClearLoaded();
  mAssets.clear();
  mAssetIndexes.clear();
  mIssues.clear();
  mManifestPath.clear();
  mAssetRoot.clear();
}

bool AssetManager::ResolvePath(const std::filesystem::path& path,
                               std::filesystem::path& resolved) const {
  if (mAssetRoot.empty() || path.empty()) {
    return false;
  }

  std::error_code error;
  const auto candidate = path.is_absolute() ? path : mAssetRoot / path;
  resolved = std::filesystem::weakly_canonical(candidate, error);
  if (error) {
    return false;
  }
  const auto relative = std::filesystem::relative(resolved, mAssetRoot, error);
  if (error || relative.empty() || relative.is_absolute()) {
    return false;
  }
  for (const auto& component : relative) {
    if (component == "..") {
      return false;
    }
  }
  return true;
}

void AssetManager::RecordIssue(const std::string& assetId,
                               const std::string& message) {
  const auto duplicate = std::find_if(
      mIssues.begin(), mIssues.end(),
      [&assetId, &message](const AssetIssue& issue) {
        return issue.assetId == assetId && issue.message == message;
      });
  if (duplicate == mIssues.end()) {
    mIssues.push_back({assetId, message});
    ECLIPSE_ERROR("Asset '{}': {}", assetId, message);
  }
}

} // namespace eclipse::managers
