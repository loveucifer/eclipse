#include "shader.h"

#include <glm/gtc/type_ptr.hpp>
#include "../src/log.h"

#include <glad/glad.h>

#include <array>

namespace {

GLuint CompileShader(GLenum type, const std::string& source,
                     const char* stageName) {
  (void)stageName;
  const GLuint shader = glCreateShader(type);
  const GLchar* sourcePointer = source.c_str();
  glShaderSource(shader, 1, &sourcePointer, nullptr);
  glCompileShader(shader);

  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_TRUE) {
    return shader;
  }

  std::array<char, 1024> log{};
  glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr,
                     log.data());
  ECLIPSE_ERROR("{} shader compilation failed: {}", stageName, log.data());
  glDeleteShader(shader);
  return 0;
}

} // namespace

namespace eclipse::graphics {

Shader::Shader(const std::string& vertex, const std::string& fragment) {
  const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertex, "Vertex");
  if (vertexShader == 0) {
    return;
  }

  const GLuint fragmentShader =
      CompileShader(GL_FRAGMENT_SHADER, fragment, "Fragment");
  if (fragmentShader == 0) {
    glDeleteShader(vertexShader);
    return;
  }

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLint linked = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked != GL_TRUE) {
    std::array<char, 1024> log{};
    glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr,
                        log.data());
    ECLIPSE_ERROR("Shader program linking failed: {}", log.data());
    glDeleteProgram(program);
    return;
  }

  mProgramId = program;
}

Shader::~Shader() {
  if (mProgramId != 0) {
    glDeleteProgram(mProgramId);
  }
}

void Shader::Bind() const {
  if (mProgramId != 0) {
    glUseProgram(mProgramId);
  }
}

void Shader::Unbind() const { glUseProgram(0); }

void Shader::SetUniformInt(const std::string& name, int value) {
  Bind();
  glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniformFloat(const std::string& name, float value) {
  Bind();
  glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniformFloat2(const std::string& name, float value1,
                              float value2) {
  Bind();
  glUniform2f(GetUniformLocation(name), value1, value2);
}

void Shader::SetUniformFloat3(const std::string& name, float value1,
                              float value2, float value3) {
  Bind();
  glUniform3f(GetUniformLocation(name), value1, value2, value3);
}

void Shader::SetUniformFloat4(const std::string& name, float value1,
                              float value2, float value3, float value4) {
  Bind();
  glUniform4f(GetUniformLocation(name), value1, value2, value3, value4);
}

void Shader::SetUniformFloat2(const std::string& name,
                              const glm::vec2& value) {
  SetUniformFloat2(name, value.x, value.y);
}

void Shader::SetUniformFloat3(const std::string& name,
                              const glm::vec3& value) {
  SetUniformFloat3(name, value.x, value.y, value.z);
}

void Shader::SetUniformFloat4(const std::string& name,
                              const glm::vec4& value) {
  SetUniformFloat4(name, value.x, value.y, value.z, value.w);
}

void Shader::SetUniformMat3(const std::string& name,
                            const glm::mat3& matrix) {
  Bind();
  glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE,
                     glm::value_ptr(matrix));
}

void Shader::SetUniformMat4(const std::string& name,
                            const glm::mat4& matrix) {
  Bind();
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE,
                     glm::value_ptr(matrix));
}

int Shader::GetUniformLocation(const std::string& name) {
  const auto existing = mUniformLocations.find(name);
  if (existing != mUniformLocations.end()) {
    return existing->second;
  }
  const int location = glGetUniformLocation(mProgramId, name.c_str());
  mUniformLocations.emplace(name, location);
  return location;
}

} // namespace eclipse::graphics
