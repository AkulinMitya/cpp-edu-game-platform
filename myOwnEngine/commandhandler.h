#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include "scene/scene.h"
#include "json.hpp"
#include <string>
#include "entitybuilder.h"

class CommandHandler {
public:
    void handle(const nlohmann::json& commands, Scene& scene) {
        for (const auto& cmd : commands["commands"]) {
            std::string action = cmd["action"];
            if (action == "summon") {
                std::string unit = cmd["unit"];
                float x = cmd["x"];
                float y = cmd["y"];
                if (unit == "soldier") {
                    EntityBuilder(scene)
                    .withTransform({x, y})
                        .withVelocity()
                        .withHealth(100)
                        .withTeam(TeamComponent::ALLY)
                        .withMesh("ally.png", 1.3f, 1.3f)
                        .withAI()
                        .withCombat(0.5f, 10)
                        .withCollidable()
                        .build();
                } else if (unit == "enemy") {
                    EntityBuilder(scene)
                    .withTransform({x, y})
                        .withVelocity()
                        .withHealth(100)
                        .withTeam(TeamComponent::ENEMY)
                        .withMesh("enemy.png", 1.3f, 1.3f)
                        .withAI()
                        .withCombat(0.5f, 10)
                        .withCollidable()
                        .build();
                } else if (unit == "fort") {
                    EntityBuilder(scene)
                    .withTransform({x, y})
                        .withHealth(300)
                        .withTeam(TeamComponent::ALLY)
                        .withMesh("fort.png", 2.0f, 2.4f)
                        .withCollidable()
                        .build();
                } else if (unit == "archer") {
                    EntityBuilder(scene)
                    .withTransform({x, y})
                        .withVelocity()
                        .withHealth(80)
                        .withTeam(TeamComponent::ALLY)
                        .withMesh("archer.png", 1.3f, 1.3f)
                        .withAI()
                        .withCombat(2.5f, 6)   // Дальше радиус, меньше урон
                        .withCollidable()
                        .build();
                }
            }
        }
    }
};


#endif // COMMANDHANDLER_H
