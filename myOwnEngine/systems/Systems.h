#ifndef SYSTEMS_H
#define SYSTEMS_H

#include <QDebug>
#include <set>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <cassert>
#include <cmath>
#include "entity/Entity.h"
#include "entity/EntityManager.h"
#include "components/Components.h"
#include "components/ComponentManager.h"
#include "camera/camera2d.h"
#include "EventBus.h"


class System {
public:
    std::set<Entity> entities;
};

using Signature = std::set<std::type_index>;

class SystemManager {
private:
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;
    std::unordered_map<std::type_index, Signature> systemSignatures;

public:
    template<typename T>
    void setSystemSignature(Signature signature) {
        std::type_index typeName(typeid(T));
        systemSignatures[typeName] = signature;
    }

    template<typename T>
    Signature getSystemSignature() {
        std::type_index typeName(typeid(T));
        return systemSignatures[typeName];
    }
    const Signature& getSystemSignature(const std::type_index& type) const {
        auto it = systemSignatures.find(type);
        assert(it != systemSignatures.end() && "System signature not found.");
        return it->second;
    }
    const std::unordered_map<std::type_index, std::shared_ptr<System>>& getSystems() const {
        return systems;
    }

    template<typename T>
    std::shared_ptr<T> registerSystem() {
        std::type_index typeName(typeid(T));
        assert(systems.find(typeName) == systems.end() && "System already registered.");
        auto system = std::make_shared<T>();
        systems[typeName] = system;
        return system;
    }

    template<typename T>
    std::shared_ptr<T> getSystem() {
        std::type_index typeName(typeid(T));
        assert(systems.find(typeName) != systems.end() && "System not found.");
        return std::static_pointer_cast<T>(systems[typeName]);
    }

    template<typename T>
    void addEntityToSystem(Entity entity) {
        getSystem<T>()->entities.insert(entity);
    }

    template<typename T>
    void removeEntityFromSystem(Entity entity) {
        getSystem<T>()->entities.erase(entity);
    }

    void addEntityToSystem(const std::type_index& systemType, Entity entity) {
        systems[systemType]->entities.insert(entity);
    }

    void removeEntityFromSystem(const std::type_index& systemType, Entity entity) {
        systems[systemType]->entities.erase(entity);
    }
    void removeEntityFromAllSystems(Entity entity) {
        for (auto& [_, system] : systems) {
            system->entities.erase(entity);
        }
    }
};

class MovementSystem : public System {
public:
    void update(ComponentManager& cm) {
        for (Entity e : entities) {
            auto& transform = cm.getComponent<TransformComponent>(e);
            auto& velocity = cm.getComponent<VelocityComponent>(e);

            // Просто добавляем скорость к позиции (фиксированный шаг)
            transform.position.x += velocity.velocity.x * 0.016f;
            transform.position.y += velocity.velocity.y * 0.016f;

            // Замедление (если не AI-объект)
            if (!cm.hasComponent<AIComponent>(e)) {
                velocity.velocity.x *= 0.95f;
                velocity.velocity.y *= 0.95f;
            }
        }
    }
};


class CollisionSystem : public System {
private:
    struct AABB {
        float minX, maxX, minY, maxY;
    };

    AABB getAABB(const TransformComponent& transform, const MeshComponent& mesh) {
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();

        for (const auto& v : mesh.vertices) {
            Point world { transform.position.x + v.x, transform.position.y + v.y };
            minX = std::min(minX, world.x);
            maxX = std::max(maxX, world.x);
            minY = std::min(minY, world.y);
            maxY = std::max(maxY, world.y);
        }

        return { minX, maxX, minY, maxY };
    }

    bool checkCollision(const AABB& box1, const AABB& box2) {
        bool noCollision = (box1.maxX < box2.minX) ||
                           (box1.minX > box2.maxX) ||
                           (box1.maxY < box2.minY) ||
                           (box1.minY > box2.maxY);
        return !noCollision;
    }

public:
    void update(ComponentManager& components) {
        for (Entity a : entities) {
            if (!components.hasComponent<TransformComponent>(a) ||
                !components.hasComponent<MeshComponent>(a) ||
                !components.hasComponent<CollidableComponent>(a)) {
                continue;
            }

            const auto& ta = components.getComponent<TransformComponent>(a);
            const auto& ma = components.getComponent<MeshComponent>(a);
            auto boxA = getAABB(ta, ma);

            for (Entity b : entities) {
                if (a == b) continue;

                if (!components.hasComponent<TransformComponent>(b) ||
                    !components.hasComponent<MeshComponent>(b) ||
                    !components.hasComponent<CollidableComponent>(b)) {
                    continue;
                }

                const auto& tb = components.getComponent<TransformComponent>(b);
                const auto& mb = components.getComponent<MeshComponent>(b);
                auto boxB = getAABB(tb, mb);

                if (checkCollision(boxA, boxB)) {
                    // Реакция на столкновение (например, откат позиции или смена направления)
                    if (components.hasComponent<VelocityComponent>(a)) {
                        auto& va = components.getComponent<VelocityComponent>(a);
                        va.velocity.x = -va.velocity.x;
                        va.velocity.y = -va.velocity.y;
                    }
                }
            }
        }
    }
};

class AISystem : public System {
public:
    void update(ComponentManager& cm, SystemManager& sm, EntityManager& em, EventBus& eventBus) {
        for (Entity e : entities) {
            auto& ai = cm.getComponent<AIComponent>(e);
            auto& combat = cm.getComponent<CombatComponent>(e);
            auto& transform = cm.getComponent<TransformComponent>(e);
            auto& team = cm.getComponent<TeamComponent>(e);

            if (ai.state == AIComponent::MOVING) {
                combat.target = findTarget(cm, e, team.team);
                if (combat.target != MAX_ENTITIES) {
                    auto& targetTransform = cm.getComponent<TransformComponent>(combat.target);
                    float dx = targetTransform.position.x - transform.position.x;
                    float dy = targetTransform.position.y - transform.position.y;
                    float distance = std::sqrt(dx * dx + dy * dy);

                    if (distance <= combat.attackRange) {
                        ai.state = AIComponent::ATTACKING;
                        auto& velocity = cm.getComponent<VelocityComponent>(e);
                        velocity.velocity = {0, 0}; // стопаем движение
                    } else {
                        // Двигаться к цели
                        auto& velocity = cm.getComponent<VelocityComponent>(e);
                        float speed = 1.5f;
                        velocity.velocity.x = (dx / distance) * speed;
                        velocity.velocity.y = (dy / distance) * speed;
                    }
                }
            }

            if (ai.state == AIComponent::ATTACKING || ai.state == AIComponent::RELOADING) {
                // Стоим на месте
                if (cm.hasComponent<VelocityComponent>(e)) {
                    auto& velocity = cm.getComponent<VelocityComponent>(e);
                    velocity.velocity = {0, 0};
                }

                if (combat.target == MAX_ENTITIES ||
                    !cm.hasComponent<HealthComponent>(combat.target)) {
                    ai.state = AIComponent::MOVING;
                    combat.target = MAX_ENTITIES;
                    continue;
                }

                auto& targetTransform = cm.getComponent<TransformComponent>(combat.target);
                float dx = targetTransform.position.x - transform.position.x;
                float dy = targetTransform.position.y - transform.position.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance > combat.attackRange) {
                    ai.state = AIComponent::MOVING;
                    continue;
                }

                combat.ticksSinceLastAttack++;

                if (combat.ticksSinceLastAttack >= combat.attackCooldownTicks) {
                    handleAttack(cm, e, sm, em, eventBus);
                    combat.ticksSinceLastAttack = 0;
                    ai.state = AIComponent::ATTACKING; // после атаки снова в ATTACKING
                }
                else {
                    ai.state = AIComponent::RELOADING;
                }
            }
        }
    }

private:
    bool isInFront(const TransformComponent& seeker, const TransformComponent& target, TeamComponent::Team seekerTeam) {
        if (seekerTeam == TeamComponent::ALLY) {
            return target.position.x > seeker.position.x;
        } else {
            return target.position.x < seeker.position.x;
        }
    }

    Entity findTarget(ComponentManager& cm, Entity seeker, TeamComponent::Team seekerTeam) {
        const auto& seekerTransform = cm.getComponent<TransformComponent>(seeker);
        Entity closestTarget = MAX_ENTITIES;
        float minDistance = std::numeric_limits<float>::max();

        for (Entity other : cm.getAllEntitiesWith<TeamComponent>()) {
            if (other == seeker) continue;

            auto& otherTeam = cm.getComponent<TeamComponent>(other);
            if (otherTeam.team != seekerTeam && cm.hasComponent<TransformComponent>(other)) {
                const auto& otherTransform = cm.getComponent<TransformComponent>(other);

                if (isInFront(seekerTransform, otherTransform, seekerTeam)) {
                    float dx = otherTransform.position.x - seekerTransform.position.x;
                    float dy = otherTransform.position.y - seekerTransform.position.y;
                    float distance = dx*dx + dy*dy;

                    if (distance < minDistance) {
                        minDistance = distance;
                        closestTarget = other;
                    }
                }
            }
        }

        return closestTarget;
    }


    void handleAttack(ComponentManager& cm, Entity attacker, SystemManager& sm, EntityManager& em, EventBus& eventBus) {
        auto& combat = cm.getComponent<CombatComponent>(attacker);

        if (!cm.hasComponent<HealthComponent>(combat.target)) {
            combat.target = MAX_ENTITIES;
            return;
        }

        // Просто уменьшаем здоровье каждый раз при атаке
        auto& targetHealth = cm.getComponent<HealthComponent>(combat.target);
        float oldHp = targetHealth.health;
        targetHealth.health  -= combat.damage;

        eventBus.publish(HealthChangedEvent(combat.target, oldHp, targetHealth.health));

        qDebug() << "Entity" << attacker << "attacks! Target health:" << targetHealth.health;
        if (targetHealth.health <= 0) {
            qDebug() << "Entity" << combat.target << "died!";
            eventBus.publish(EntityDiedEvent(combat.target, attacker));
            combat.target = MAX_ENTITIES;
        }
    }

};

struct WinConditionSystem : System {
    void update(ComponentManager& cm) {
        for (auto entity : cm.getAllEntitiesWith<WinConditionComponent>()) {
            auto& winComp = cm.getComponent<WinConditionComponent>(entity);
            if (winComp.condition(cm)) {
                winComp.onWin();
            }
        }
    }
};

class DeathSystem {
public:
    DeathSystem(EventBus& bus, ComponentManager& cm, SystemManager& sm, EntityManager& em)
        : componentManager(cm), systemManager(sm), entityManager(em)
    {
        bus.subscribe<EntityDiedEvent>(
            [this](const EntityDiedEvent& event) {
                this->onEntityDied(event);
            }
            );
    }
private:
    ComponentManager& componentManager;
    SystemManager& systemManager;
    EntityManager& entityManager;

    void onEntityDied(const EntityDiedEvent& event) {
        systemManager.removeEntityFromAllSystems(event.entity);
        componentManager.removeEntity(event.entity);
        entityManager.destroyEntity(event.entity);
    }
};

class HealthChangeSystem {
public:
    HealthChangeSystem(EventBus& bus) {
        bus.subscribe<HealthChangedEvent>(
            [](const HealthChangedEvent& event) {
                qDebug() << "[HealthChangeSystem] Entity" << event.entity
                         << "HP changed from" << event.oldHealth
                         << "to" << event.newHealth;
                // Здесь можно вызывать анимации, обновлять GUI и т.д.
            }
            );
    }
};

struct ResourceSystem : System {
    void update(ComponentManager& cm) {
        for (Entity e : cm.getAllEntitiesWith<ResourceComponent>()) {
            auto& res = cm.getComponent<ResourceComponent>(e);
            res.resources["elixir"] += 0.03f; // ~+1 за 30 апдейтов (0.5 сек при 60 FPS)
            if (res.resources["elixir"] > 10.0f) res.resources["elixir"] = 10.0f; // max cap
        }
    }
};


#endif // SYSTEMS_H
