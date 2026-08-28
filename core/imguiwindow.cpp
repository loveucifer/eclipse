#include "imguiwindow.h"
#include "external/imgui/imgui.h"
#include "../src/engine.h"
#include "external/imgui/imgui_impl_opengl3.h"
#include "external/imgui/imgui_impl_sdl2.h"
namespace eclipse::core{
  void ImguiWindow::Create(){
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      auto& window = engine::Instance().GetWindow();
      // here imgui accepts 2 parameters for initlaizing which are
      // our mWindow and mGLContext but we dont have that exposed yet
      // as they are private so make a getter
      ImGui_ImplSDL2_InitForOpenGL(window.GetSDLWindow(), window.GetSDLGLContext());
      ImGui_ImplOpenGL3_Init("#version 410");
      // imgui internally uses shaders so it kinda needs to know what shaders we want to use
  }

  void ImguiWindow::Shutdown(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }

  void ImguiWindow::HandleSDLEvents(SDL_Event& event){
    ImGui_ImplSDL2_ProcessEvent(&event);
    
  }

  void ImguiWindow::BeginRender(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
  };

  void ImguiWindow::EndRender(){
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
}
