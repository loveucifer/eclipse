#include "./entitymanager.h"
#include <iostream>
#include "entity.h"
#include "collidercomponent.h"
#include "component.h"
#include "collision.h"

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

// std::string EntityManager::CheckEntityCollisions(Entity &entity) const {
//   ColliderComponent *collider = entity.GetComponent<ColliderComponent>();
//   for (auto &other : entities) {
//     if (other->entityName.compare(entity.entityName) != 0 &&
//         other->entityName.compare("Tile") != 0) {
//       if (other->HasComponent<ColliderComponent>()) {
//         ColliderComponent *otherCollider =
//             other->GetComponent<ColliderComponent>();
//         if (Collision::CheckCollision(collider->collider,
//                                       otherCollider->collider))
//           return otherCollider->colliderId;
//       }
//     }
//   }
//   return std::string();
// }

CollisionType EntityManager::CheckCollisions() const {
  for (int i = 0; i < entities.size() - 1; i++) {
    auto &thisEntity = entities[i];
    if (thisEntity->HasComponent<ColliderComponent>()) {
      ColliderComponent *thisCollider =
          thisEntity->GetComponent<ColliderComponent>();
      for (int j = i + 1; j < entities.size(); j++) {
        auto &thatEntity = entities[j];
        if (thisEntity->entityName.compare(thatEntity->entityName) != 0 &&
            thatEntity->HasComponent<ColliderComponent>()) {
          ColliderComponent *thatCollider =
              thatEntity->GetComponent<ColliderComponent>();
          if (Collision::CheckCollision(thisCollider->collider,
                                        thatCollider->collider)) {
            if (thisCollider->colliderId.compare("PLAYER") == 0 &&
                thatCollider->colliderId.compare("ENEMY") == 0) {
              return PLAYER_ENEMY_COLLISION;
            }
            if (thisCollider->colliderId.compare("PLAYER") == 0 &&
                thatCollider->colliderId.compare("PROJECTILE") == 0) {
              return PLAYER_PROJECTILE_COLLISION;
            }
            if (thisCollider->colliderId.compare("ENEMY") == 0 &&
                thatCollider->colliderId.compare("FRIENDLY_PROJECTILE") == 0) {
              return ENEMY_PROJECTILE_COLLISION;
            }
            if (thisCollider->colliderId.compare("PLAYER") == 0 &&
                thatCollider->colliderId.compare("LEVEL_COMPLETE") == 0) {
              return PLAYER_LEVEL_COMPLETE_COLLISION;
            }
          }
        }
      }
    }
  }
  return NO_COLLISION;
}

void EntityManager::Render() {
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

