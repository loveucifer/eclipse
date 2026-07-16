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
    // TODO write about what this does 
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
