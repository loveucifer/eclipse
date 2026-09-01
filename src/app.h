#pragma once
#include "../core/window.h"
namespace eclipse{
  class App{
    public:
      App(){}
      ~App(){}

      // after creating the props for windows we need a method to pass it onto
      // our editor or any other app we might add over this one day , so we need
      // to do something to handle those properties other than hardcode them

    
      inline virtual core::WindowProperties GetWindowProperties(){return core::WindowProperties();}
      virtual void Initialize(){}
      virtual void Shutdown(){}
      virtual void Update(){}
      virtual void Render(){}
      virtual void ImGuiRender(){}      
      
    private:
  };
}
