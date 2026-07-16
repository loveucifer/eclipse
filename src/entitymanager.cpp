#include "./entitymanager.h"

// void cleardata destorys entities whenever needed
void EntityManager::Cleardata(){
  for (auto &entity:entities){
    entity -> Destroy();
  }
} 

bool EntityManager::HasNoEntities() const {
  return entities.size() == 0;
}

void EntityManager::Update(float deltaTime){
  for (auto &entity: entities){
    entity ->Update(deltaTime);
  }
}

void EntityManager::Render(){
  for (auto &entity: entities){
    entity ->Render();
   }
}

Entity &EntityManager::AddEntity(std::string entityName){
  // to add an entity to our vector we need to push
  Entity *entity = new Entity(*this,entityName);
  entities.emplace_back(entity);
  return *entity;
}

std::vector<Entity*> EntityManager::GetEntities() const {
  return entities;
}

unsigned int EntityManager::GetEntityCount() {
  return entities.size();
}

