#ifndef ENTITY_H
#define ENTITY_H
#include <vector>
#include "component.h"

class EntityManager;
class Component;

class Entity { 
  private:
    EntityManager &manager;
    bool isEntityActive;
    std::vector<Component*> components;
  public:
    std::string entityName;
    Entity(EntityManager &manager);
    Entity(EntityManager &manager,std::string name);
    void Update(float deltaTime);
    void Render();
    void Destroy();
    // getter for isEntityActive()
    bool GetIsEntityActive() const;

    template <typename T,typename... TArgs>
    T& AddComponent(TArgs&& ...args){
      T* newComponent(new T(std::forward<TArgs>(args)...));
      newComponent -> owner = this;
      components.emplace_back(newComponent);
      newComponent-> Initialize();
      return *newComponent;
    }
    // we use a template for this part and this is something that i need to write down
    // to remember what it does exactly obviously this is a template and t is type of componenet
    // we want to create , we dont know what type of componenet we want to create maybe a transfrom componenet
    // sprite componenet etc and targs is a parameter pack which can reperestn argument types such as 
    // int , double , etc m, then we go onto create a new componenet with T* new Componenet this does 2 things
    // first is that it construfct an obj with type T so that suppose we have  class with transform componenet which has public 
    // function transfrom componenet accpeitng 2 parameters of int x and y for position with this our add componenet works like
    // addcomponenet<TransformComponenet>100,50 this becomes new Transform componenet 100,50 the next thing it does it
    // it perfectly forwards our argument that we pass in, 
    // important part about " std::forward " without forwarding the orignal value category is presevered so std::forward
    // avoids unnesary copies , the next part is setting the owenre , sets the owner with this one and then emplaces back whihc
    // means it stores it and then initalizes it and then returns the pointer to a new componenet its basically a function that says
    // "Create a component, attach it to this entity, initialize it, and give it back to me."
    // https://loveucifer.github.io/posts/game-engine-ecs/  here for more 
    // 
    // 
};


////////////////////////////////////////////////
////// entity manager //////////////////////////
// we can think of entity manger as something that is used
// to manage all the entities we have or might create as we go on
// so EntityManager controls the Entities we might create and the Entity
// will have different components , for example we can think of something like this
// imagine we have a player entity , we use the entity manager to control the player who has components such as sprite , transform etc
// in this entity class we have private and public functions , for private we have a reference to the entity manager , and a bool to check whether if the entitty is actually active or not and a vector pointer to the componments we may have like transform scale spirte etc
// for public we have the name of the entity a ref to the manager with both the string and without the string , the thigns we need to do in the game loop
// such as update render and destroy and finally a getter for the active part
////////////////////////////////////////////////////
/////////////////////////////////////////////////////


#endif
