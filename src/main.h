
#include "engine.h"
#include "app.h"

// to be implemeneted in client app , client returns an instance of a class
// derived fron eclipse app

eclipse::App* CreateApp();



int main(){
  
  eclipse::App* app = CreateApp();
  eclipse::engine::Instance().Run(app);
  delete app;
  // eclipse::engine& engine = eclipse::engine::Instance();
  // if(engine.Initialize()){
  //   while(true){
  //   }
  // }
  return 0;
}
