#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "./entity.h"
#include "component.h"

class EntityManager{
  private:
    std::vector <Entity*> entities; 
  public:
    void Cleardata();
    void Update(float deltaTime);
    void Render();
    void ListAllEntities() const;
    bool HasNoEntities() const ;
    Entity& AddEntity(std::string entityName,LayerType layer);
    std::vector<Entity*> GetEntities() const;
    std::vector<Entity*> GetEntitiesByLayer(LayerType layer) const;
    unsigned int GetEntityCount();
    CollisionType CheckCollisions() const;
};

// entity manager has a private fucnction of all the entities and public for clearing the data updating it rendering it , checking whether it has no entities or not , a fucntion to add entities , a function to get the entities and finally an int to get the count of entities //


#endif
