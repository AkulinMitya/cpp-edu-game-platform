#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include "scene/scene.h"
#include "CommandHandler.h"
#include "json.hpp"

using json = nlohmann::json;

json serializeScene(const Scene& scene) {
    json state;
    state["entities"] = json::array();
    for (const auto& entity : scene.getAllEntities()) {
        if (!scene.hasComponent<TransformComponent>(entity)) continue;
        const auto& transform = scene.getComponent<TransformComponent>(entity);

        // Могут быть дефолтные значения
        std::string type = "soldier"; // Можешь расширить если есть другие типы
        std::string texture = "default.png";
        float width = 0.5f;
        float height = 0.5f;

        if (scene.hasComponent<MeshComponent>(entity)) {
            const auto& mesh = scene.getComponent<MeshComponent>(entity);
            texture = mesh.textureName;
            width = mesh.width;
            height = mesh.height;
        }

        float hp = scene.hasComponent<HealthComponent>(entity)
                       ? scene.getComponent<HealthComponent>(entity).health : 0;

        state["entities"].push_back(json{
            {"id", entity},
            {"type", type},
            {"x", transform.position.x},
            {"y", transform.position.y},
            {"hp", hp},
            {"texture", texture},
            {"width", width},
            {"height", height}
        });
    }
    return state;
}


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QTcpServer server;
    Scene scene;
    CommandHandler handler;

    QTimer updateTimer;
    QObject::connect(&updateTimer, &QTimer::timeout, [&scene]() {
        scene.update();
    });
    updateTimer.start(16);


    QObject::connect(&server, &QTcpServer::newConnection, [&]() {
        QTcpSocket *client = server.nextPendingConnection();
        qDebug() << "[SERVER] Новый клиент подключился:" << client;
        QObject::connect(client, &QTcpSocket::readyRead, [client, &scene, &handler]() {
            qDebug() << "[SERVER] readyRead, bytesAvailable:" << client->bytesAvailable();
            QByteArray data = client->readAll();
            qDebug() << "[SERVER] Получены данные от клиента (size:" << data.size() << "):" << data;

            // Проверка: команда или запрос состояния?
            if (data == "get_state" || data == "{}") {
                json state = serializeScene(scene);
                QByteArray reply = QString::fromStdString(state.dump()).toUtf8();
                client->write(reply);
                client->flush();
                qDebug() << "[SERVER] GUI-клиент запросил состояние, отправлен ответ.";
                return;
            }

            try {
                json cmd = json::parse(data.toStdString());
                qDebug() << "[SERVER] JSON принят, команд:" << (cmd.contains("commands") ? cmd["commands"].size() : 0);
                handler.handle(cmd, scene);
                scene.update();
                json state = serializeScene(scene);
                QByteArray reply = QString::fromStdString(state.dump()).toUtf8();
                client->write(reply);
                client->flush();
                qDebug() << "[SERVER] Ответ отправлен клиенту (size:" << reply.size() << ")";
            } catch (const std::exception& e) {
                QByteArray err = QString("JSON parse error: %1").arg(e.what()).toUtf8();
                client->write(err);
                client->flush();
                qDebug() << "[SERVER] Ошибка парсинга JSON:" << e.what();
            }
        });
        QObject::connect(client, &QTcpSocket::disconnected, [client]() {
            qDebug() << "[SERVER] Клиент отключился:" << client;
            client->deleteLater();
        });
    });

    if (!server.listen(QHostAddress::Any, 12345)) {
        qDebug() << "Не удалось запустить сервер!";
        return 1;
    }
    qDebug() << "[SERVER] Сервер слушает порт 12345...";
    return app.exec();
}
