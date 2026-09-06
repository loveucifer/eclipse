#pragma once
#include "../glm/glm.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
namespace eclipse::graphics {
class shader {
public:
  shader(const std::string &vertex, const std::string &fragment);
  ~shader();

  void Bind();
  void UnBind();

  void SetUniformInt(const std::string &name, int val);

  void SetUniformFloat(const std::string &name, float val1);

  void SetUniformFloat2(const std::string &name, float val1, float val2);
  void SetUniformFloat2(const std::string& name, const glm::vec2& val);

  void SetUniformFloat3(const std::string &name, float val1, float val2,
                        float val3);
  void SetUniformFloat3(const std::string& name, const glm::vec3& val);

  void SetUniformFloat4(const std::string &name, float val1, float val2,
                        float val3, float val4);
  void SetUniformFloat4(const std::string& name, const glm::vec4& val);

  void SetUniformMat3(const std::string& name , const glm::mat3& mat);
  void SetUniformMat4(const std::string& name , const glm::mat4& mat);
private:

  int GetUniformLocation(const std::string& name);
  std::unordered_map<std::string, int>mUniformLocations;
  uint32_t mProgramId;
};
} // namespace eclipse::graphics
