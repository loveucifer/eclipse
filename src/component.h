#ifndef COMPONENT_H
#define COMPONENT_H

class Entity;


class Component { 
  private:
  public:
    Entity *owner;
    virtual ~Component() {}
    virtual void Initialize () {}
    virtual void Update(float deltaTime) {}
    virtual void Render() {}
};


// we are using virtual for components as we want to call them from the entities
// we want to work with for eg a spritecomponent and a transformcomponenet have
// virtual update and render so that we can create those components and then render
// them with overrride 

#endif
