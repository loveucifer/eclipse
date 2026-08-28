#include "shader.h"
#include "../src/log.h"
#include "glad/glad.h"
#include <cstdint>
#include "helpers.h"

namespace eclipse::graphics {

shader::shader(const std::string &vertex, const std::string &fragment) {
  // this is how we start our shader
  mProgramId = glCreateProgram();ECLIPSE_CHECK_GL_ERROR;

  int compileStatus = GL_FALSE;
  char errorLog[512];

  uint32_t vertexShaderId = glCreateShader(GL_VERTEX_SHADER);ECLIPSE_CHECK_GL_ERROR;
  {
    // vertexShader//
    // we need to create a vertex shader
    // uint32_t vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    // need to copy it into opengl compatible type
    const GLchar *glSource = vertex.c_str();ECLIPSE_CHECK_GL_ERROR;
    glShaderSource(vertexShaderId, 1, &glSource, NULL);ECLIPSE_CHECK_GL_ERROR;
    glCompileShader(vertexShaderId);ECLIPSE_CHECK_GL_ERROR;
    // may or may not have worked depending on how compilation went ,
    // need 2 for both vertex and fragment to error log out
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &compileStatus);ECLIPSE_CHECK_GL_ERROR;
    // iv means integer value
    if (compileStatus != GL_TRUE) {
      glGetShaderInfoLog(vertexShaderId, sizeof(errorLog), NULL, errorLog);ECLIPSE_CHECK_GL_ERROR;
      ECLIPSE_ERROR("Vertex Shader Compilation Error {}", errorLog);ECLIPSE_CHECK_GL_ERROR;
      // if our shader failed we should delete it obviously
      // glDeleteShader(vertexShaderId);
    } else {
      glAttachShader(mProgramId, vertexShaderId);ECLIPSE_CHECK_GL_ERROR;
    }

  } // vertexShader//

  uint32_t fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);ECLIPSE_CHECK_GL_ERROR;

  if (compileStatus == GL_TRUE) {
    // fragmentShader//
    // we need to create a fragment shader
    // uint32_t fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    // need to copy it into opengl compatible type
    const GLchar *glSource = fragment.c_str();ECLIPSE_CHECK_GL_ERROR;
    glShaderSource(fragmentShaderId, 1, &glSource, NULL);ECLIPSE_CHECK_GL_ERROR;
    glCompileShader(fragmentShaderId);ECLIPSE_CHECK_GL_ERROR;
    // may or may not have worked depending on how compilation went ,
    // need 2 for both fragment and fragment to error log out
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &compileStatus);ECLIPSE_CHECK_GL_ERROR;
    // iv means integer value
    if (compileStatus != GL_TRUE) {
      glGetShaderInfoLog(fragmentShaderId, sizeof(errorLog), NULL, errorLog);ECLIPSE_CHECK_GL_ERROR;
      ECLIPSE_ERROR("Fragment Shader Compilation Error {}", errorLog);ECLIPSE_CHECK_GL_ERROR;
      // if our shader failed we should delete it obviously
      // glDeleteShader(fragmentShaderId);
      // since we are deleting it anyway regardless delete it after everything
    } else {
      glAttachShader(mProgramId, fragmentShaderId);ECLIPSE_CHECK_GL_ERROR;
    }
  } // fragmentShader//

  ECLIPSE_ASSERT(compileStatus == GL_TRUE, "Error compiling Shader");

  // link shader

  if (compileStatus == GL_TRUE) {
    glLinkProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR;
    glValidateProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR;
    glGetProgramiv(mProgramId, GL_LINK_STATUS, &compileStatus);ECLIPSE_CHECK_GL_ERROR;
    if (compileStatus != GL_TRUE) {
      glGetProgramInfoLog(mProgramId, sizeof(errorLog), NULL, errorLog);ECLIPSE_CHECK_GL_ERROR;
      ECLIPSE_ERROR("Error linking shader: { }", errorLog);
      // glDeleteProgram(mProgramId);
      mProgramId = -1;
    }
  }

  // even if we are succesfull they are linked and uploadded so we can just
  // delete it

  glDeleteShader(vertexShaderId);ECLIPSE_CHECK_GL_ERROR;
  glDeleteShader(fragmentShaderId);ECLIPSE_CHECK_GL_ERROR;
}

shader::~shader() {

  glUseProgram(0);ECLIPSE_CHECK_GL_ERROR;
  glDeleteProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR;
  // cant delete program if its in use so make it 0 then delete
}

void shader::Bind() { glUseProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR; }

void shader::UnBind() { glUseProgram(0);ECLIPSE_CHECK_GL_ERROR; }

void shader::SetUniformInt(const std::string &name, int val) {
  glUseProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR;
  glUniform1i(GetUniformLocation(name), val);ECLIPSE_CHECK_GL_ERROR;
}

void shader::SetUniformFloat(const std::string &name, float val1) {

  glUseProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR;
  glUniform1f(GetUniformLocation(name), val1);ECLIPSE_CHECK_GL_ERROR;
}
void shader::SetUniformFloat2(const std::string &name, float val1, float val2) {

  glUseProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR;
  glUniform2f(GetUniformLocation(name), val1, val2);ECLIPSE_CHECK_GL_ERROR;
}
void shader::SetUniformFloat3(const std::string &name, float val1, float val2,
                              float val3) {

  glUseProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR;
  glUniform3f(GetUniformLocation(name), val1, val2, val3);ECLIPSE_CHECK_GL_ERROR;
}
void shader::SetUniformFloat4(const std::string &name, float val1, float val2,
                              float val3, float val4) {

  glUseProgram(mProgramId);ECLIPSE_CHECK_GL_ERROR;
  glUniform4f(GetUniformLocation(name), val1, val2, val3, val4);ECLIPSE_CHECK_GL_ERROR;
}

// caching uniform locations , if we get unifroms every like say 100 things it
// will keep calling it 100 times , we call it once and cache

int shader::GetUniformLocation(const std::string &name) {
  auto it = mUniformLocations.find(name);
  if (it == mUniformLocations.end()) {
    mUniformLocations[name] = glGetUniformLocation(mProgramId, name.c_str());ECLIPSE_CHECK_GL_ERROR;
  }

  return mUniformLocations[name];
}

} // namespace eclipse::graphics
