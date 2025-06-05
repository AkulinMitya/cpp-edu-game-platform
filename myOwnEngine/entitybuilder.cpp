#include "entitybuilder.h"
#include "meshUtils.h"

EntityBuilder::EntityBuilder(Scene& scene)
    : scene(scene)
{
    entity = scene.createEntity(false, false);
}

EntityBuilder& EntityBuilder::withTransform(Point position) {
    scene.addComponent<TransformComponent>(entity, position);
    return *this;
}

EntityBuilder& EntityBuilder::withVelocity() {
    scene.addComponent<VelocityComponent>(entity);
    return *this;
}

EntityBuilder& EntityBuilder::withHealth(int hp) {
    scene.addComponent<HealthComponent>(entity, hp);
    return *this;
}

EntityBuilder& EntityBuilder::withTeam(TeamComponent::Team team) {
    scene.addComponent<TeamComponent>(entity, team);
    return *this;
}

EntityBuilder& EntityBuilder::withMesh(const std::string& textureName, float w, float h) {
    MeshComponent mesh;
    mesh.textureName = textureName; // просто имя файла!
    mesh.width = w;
    mesh.height = h;
    mesh.vertices = makeRectangleMesh(w, h);
    scene.addComponent<MeshComponent>(entity, mesh);
    return *this;
}

EntityBuilder& EntityBuilder::withAI() {
    scene.addComponent<AIComponent>(entity);
    return *this;
}

EntityBuilder& EntityBuilder::withCombat(float range, int damage) {
    CombatComponent combat;
    combat.attackRange = range;
    combat.damage = damage;
    scene.addComponent<CombatComponent>(entity, combat);
    return *this;
}

EntityBuilder& EntityBuilder::withCollidable() {
    scene.addComponent<CollidableComponent>(entity);
    return *this;
}

Entity EntityBuilder::build() {
    return entity; // все подписки уже обновлены после добавления компонентов
}
