#include "main.h"
#include "../graphics/mesh.h"
#include "../graphics/shader.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "app.h"
#include "engine.h"
#include "external/imgui/imgui.h"
#include "log.h"
#include <cstdint>
#include <memory>
#include "../graphics/framebuffer.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"
using namespace eclipse;

class Editor : public eclipse::App {

private:
  std::shared_ptr<eclipse::graphics::mesh> mMesh;
  std::shared_ptr<eclipse::graphics::shader> mShader;

  float xKeyOffset = 0.f;
  float yKeyOffset = 0.f;
  float keySpeed = 0.01f;


  glm::vec2 mRectPos,mRectSize;


public:
  // now that we have created our windowproperties and implemented it in our
  // regular window.create we should make it override here

  core::WindowProperties GetWindowProperties() override {
    core::WindowProperties props;
    props.w = 800;
    props.h = 600;
    props.title = "Eclipse Editor";
    props.ImGuiProps.IsViewPortEnabled = true;
    props.ImGuiProps.IsDockingEnabled = true;
    return props;
  }
  void Initialize() override {

    ECLIPSE_TRACE("Editor:: Initialize()");
    float vertices[]{// -0.5f  ,-0.5f  ,0.f,
                     // 0.f    ,0.5f   ,0.f,
                     // 0.5f   ,-0.5f  ,0.f

                     0.5f,  0.5f,
                     0.f,

                     0.5f,  -0.5f,
                     0.f,

                     -0.5f, -0.5f,
                     0.f,

                     -0.5f, 0.5f,
                     0.f

    };

    uint32_t elements[]{0, 3, 1, 1, 3, 2};

    mMesh = std::make_shared<eclipse::graphics::mesh>(&vertices[0], 4, 3,
                                                      &elements[0], 6);

    const char *vertexShader = R"(
            #version 410 core
            layout(location = 0) in vec3 position;
            out vec3 vertexpos;
            uniform mat4 model = mat4(1.0);
            uniform vec2 offset = vec2 (0.5);
            void main(){
              vertexpos = position + vec3(offset , 0);
              gl_Position = model * vec4(position,1.0);
            }
         )";

    const char *fragmentShader = R"(
            #version 410 core
            out vec4 outColor;
            in vec3 vertexpos;
            uniform vec3 color = vec3(0.0);
            void main(){
              outColor = vec4(vertexpos,1.0);
            }
         )";

    mShader = std::make_shared<eclipse::graphics::shader>(vertexShader,
                                                          fragmentShader);

    mRectPos = glm::vec2(0.f);
    mRectSize = glm::vec2(1.f);
  }

  void Update() override {

    ECLIPSE_TRACE("Editor:: Update()");

    auto windowSize =engine::Instance().GetWindow().GetSize();

    // engine::Instance().GetWindow().GetSize(windowWidth, windowHeight);
    float xNormal = (float)input::mouse::X() / (float)windowSize.x;
    float yNormal =
        (float)(windowSize.y- input::mouse::Y()) / (float)windowSize.y;



    if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_LEFT)) {
      xKeyOffset -= keySpeed;
    }
    if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_RIGHT)) {
      xKeyOffset += keySpeed;
    }
    if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_UP)) {
      yKeyOffset += keySpeed;
    }
    if (input::keyboard::Key(input::ECLIPSE_INPUT_KEY_DOWN)) {
      yKeyOffset -= keySpeed;
    }

    mShader->SetUniformFloat3("color", 1, 0, 0);
    // Mouse controls the color gradient; keyboard controls the rectangle position.
    mShader->SetUniformFloat2("offset", xNormal, yNormal);


    glm::mat4 model = glm::mat4(1.f);
    model = glm::translate(model, {mRectPos.x,mRectPos.y,0.f});
    model = glm::scale(model, {mRectSize.x,mRectSize.y,0.f});
    mShader->SetUniformMat4("model", model);

    ECLIPSE_TRACE("{},{}", windowSize.x, windowSize.y);
    ECLIPSE_TRACE("X: {}, Y: {}, {}{}{}{}{}", input::mouse::X(),
                  input::mouse::Y(),

                  input::mouse::Button(input::ECLIPSE_INPUT_MOUSE_LEFT),

                  input::mouse::Button(input::ECLIPSE_INPUT_MOUSE_RIGHT),

                  input::mouse::Button(input::ECLIPSE_INPUT_MOUSE_MIDDLE),

                  input::mouse::Button(input::ECLIPSE_INPUT_MOUSE_X1),

                  input::mouse::Button(input::ECLIPSE_INPUT_MOUSE_X2));
  }

  void Render() override {

    ECLIPSE_TRACE("Editor:: Render()");
    auto rc =
        std::make_unique<graphics::rendercommands::RenderMesh>(mMesh, mShader);
    engine::Instance().GetRenderManager().Submit(std::move(rc));
  }

  void ImGuiRender() override {

    // dock to window edge
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    // note
    
    if (ImGui::Begin("Rect Pos")) {
      ImGui::DragFloat2("Rect Pos", glm::value_ptr(mRectPos), 0.01f);
    }
    ImGui::End();

    if (ImGui::Begin("Rect Size")) {
      ImGui::DragFloat2("Rect Size", glm::value_ptr(mRectSize), 0.01f);
    }

    ImGui::End();

    if (ImGui::Begin("GameView")) {

      auto &window = engine::Instance().GetWindow();
      ImVec2 size = {480,360};
      ImVec2 uv0 ={0,1};
      ImVec2 uv1 ={1,0};
      ImGui::Image(
        
        ImTextureID(window.GetFrameBuffer()->GetTextureId()),size,uv0,uv1
      );
    }
    ImGui::End();
  }

  void Shutdown() override { ECLIPSE_TRACE("Editor:: Shutdown()"); }
};

eclipse::App *CreateApp() { return new Editor(); }
