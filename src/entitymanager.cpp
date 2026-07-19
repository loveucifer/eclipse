#include "./entitymanager.h"
#include <iostream>
#include "entity.h"

// void cleardata destorys entities whenever needed
void EntityManager::Cleardata(){
  for (auto &entity:entities){
    entity -> Destroy();
  }
}

void EntityManager::ListAllEntities() const {
  unsigned int i = 0;
  for (auto &entity : entities) {
    std::cout << "Entity [ " << i << " ]" << entity->entityName << std::endl;
    entity->ListAllComponents();
    i++;
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
  // for (auto &entity: entities){
  //   entity ->Render();
  //  }

    for (int layerNumber = 0; layerNumber < NUM_LAYER ; layerNumber++){
    for (auto &entity: GetEntitiesByLayer(static_cast<LayerType>(layerNumber))){
        entity ->Render();
      
    }
  }
  
}

std::vector<Entity*> EntityManager::GetEntitiesByLayer(LayerType layer) const{
  std::vector<Entity*> selectedEntities;
  for (auto& entity:entities){
    if (entity->layer == layer){
      selectedEntities.emplace_back(entity);
    }
  }
  return selectedEntities;
}


Entity &EntityManager::AddEntity(std::string entityName,LayerType layer){
  // to add an entity to our vector we need to push
  Entity *entity = new Entity(*this,entityName,layer);
  entities.emplace_back(entity);
  return *entity;
}

std::vector<Entity*> EntityManager::GetEntities() const {
  return entities;
}

unsigned int EntityManager::GetEntityCount() {
  return entities.size();
}

