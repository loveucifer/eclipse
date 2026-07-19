#ifndef ENTITY_H
#define ENTITY_H
#include <vector>
#include <map>
#include "constants.h"
#include "component.h"

class EntityManager;
class Component;

class Entity { 
  private:
    EntityManager &manager;
    bool isEntityActive;
    std::vector<Component*> components;
    std::map<const std::type_info*,Component*> componentTypeMap;
  public:
    std::string entityName;
    LayerType layer;
    Entity(EntityManager &manager);
    Entity(EntityManager &manager,std::string name,LayerType layer);
    void ListAllComponents() const;
    void Update(float deltaTime);
    void Render();
    void Destroy();

    template <typename T>
    bool HasComponent() const{
      return componentTypeMap.find(&typeid(T)) != componentTypeMap.end();
    }

    // few things to note this is not really as fast as one might hope to but
    // for most parts we do not care and lets instead focus on how this works
    // typeid(T) returns a reference to std::type info object basically a small object the compiler genreates once per distinct type ., so &typeid is the addressss of the per type singleton obj so it works as a map key , so when we clal the has component t its justasking if this entitys map have an entry whos e key is the address of Ts type info ,
    // more explanation jeach entity in our game has different parts attached to it as
    // we talked like sprite position etc they are componenets
    // and each entity imagine has a filling cabinet called componenet typemap
    // everytime we add something we file this under some label say something like
    // health or position or whavter, so its like take this new componenet , throw it undre some filing cabinet under its " matching " type and has componenet just goes to the filing cabinet and chekcs if its there or not

    // getter for isEntityActive()
    bool GetIsEntityActive() const;

    template <typename T,typename... TArgs>
    T& AddComponent(TArgs&& ...args){
      T* newComponent(new T(std::forward<TArgs>(args)...));
      newComponent -> owner = this;
      components.emplace_back(newComponent);
      componentTypeMap[&typeid(*newComponent)] = newComponent;
      newComponent-> Initialize();
      return *newComponent;
    }

    template <typename T> T *GetComponent() {
      return static_cast<T*>(componentTypeMap[&typeid(T)]);
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
