#include "imguiwindow.h"
#include "external/imgui/imgui.h"
#include "../src/engine.h"
#include "external/imgui/imgui_impl_opengl3.h"
#include "external/imgui/imgui_impl_sdl2.h"
#include <SDL_video.h>
namespace eclipse::core{
  void ImguiWindow::Create(const ImguiWindowProperties& props){
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();

      ImGuiIO& io = ImGui::GetIO();
      io.ConfigWindowsMoveFromTitleBarOnly = props.MoveFromTitleBarOnly;
      if(props.IsDockingEnabled){
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
       }
       if(props.IsViewPortEnabled){
         io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
         // when we end render we need to end it for viewport as well
       }

      
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
    ImGuiIO& io = ImGui::GetIO();
    if(io.ConfigFlags && ImGuiConfigFlags_ViewportsEnable){
    // viewpoerts work a bit differnrelyt so we have to update and render
    // platform views for viewpoert specifically and after this we think we need to like make changes to this from our props as well right so make it a sub struct for our original props ?
      auto& window = engine::Instance().GetWindow();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(window.GetSDLWindow(), window.GetSDLGLContext());
    }
  }


  bool ImguiWindow::WantCaptureMouse(){
   return ImGui::GetIO().WantCaptureMouse; 
  }

  bool ImguiWindow::WantCaptureKeyboard(){
    return ImGui::GetIO().WantCaptureKeyboard;
  }

  
}
