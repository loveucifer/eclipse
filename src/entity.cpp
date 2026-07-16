#include "./entity.h"

Entity::Entity(EntityManager &manager): manager(manager){
  this -> isEntityActive = true;
}

Entity::Entity(EntityManager &manager, std::string name): manager(manager), isEntityActive(),entityName(name){
    this -> isEntityActive = true;
} 

void Entity::Update(float deltaTime){
  // we have to loop all the componenets of the entities
  for (auto &component: components){
    component -> Update(deltaTime);
  }
}

void Entity::Render(){
  for (auto &component: components){
    component -> Render();
  }
}

void Entity::Destroy(){
  this -> isEntityActive = false;  
}

bool Entity::GetIsEntityActive() const {
  return this -> isEntityActive;
}

/////////////////////////////////////////////////////////////
////////// rendering the components ////////////////////////
// for each of the entity we have to update and render we just use
// auto for whatver componenet we may end up withj and just update it and
// then finally render it //////////////////////////////////////
////////////////////////////////////////////////////////////////
