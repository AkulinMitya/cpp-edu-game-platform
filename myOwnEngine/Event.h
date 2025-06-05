#ifndef EVENT_H
#define EVENT_H

#include "entity/Entity.h"

struct EventBase {
    virtual ~EventBase() = default;
};

struct EntityDiedEvent : EventBase {
    Entity entity;
    Entity killer;
    EntityDiedEvent(Entity e, Entity k = MAX_ENTITIES) : entity(e), killer(k) {}
};

struct HealthChangedEvent : EventBase {
    Entity entity;
    float oldHealth;
    float newHealth;
    HealthChangedEvent(Entity e, float oldH, float newH) : entity(e), oldHealth(oldH), newHealth(newH) {}
};

#endif // EVENT_H
