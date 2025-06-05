#include <QCoreApplication>
#include <QTcpSocket>
#include <QTimer>
#include "json.hpp"
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QTcpSocket socket;

    QObject::connect(&socket, &QTcpSocket::connected, [&]() {
        nlohmann::json j;
        j["commands"] = {
            { { "action", "summon" }, { "unit", "archer" }, { "x", -3.3 }, { "y", 0.0 } },
            { { "action", "summon" }, { "unit", "soldier" }, { "x", -2.3 }, { "y", 0.0 } },
            { { "action", "summon" }, { "unit", "enemy" },   { "x",  2.3 }, { "y", 0.0 } },
            { { "action", "summon" }, { "unit", "enemy" },   { "x",  2.3 }, { "y", 1.0 } },
            { { "action", "summon" }, { "unit", "fort" },    { "x",  -4.0 }, { "y", -2.0 } }
        };
        QByteArray data = QString::fromStdString(j.dump()).toUtf8();
        socket.write(data);
        socket.flush();
        qDebug() << "Sendable data: " << data;
    });

    QObject::connect(&socket, &QTcpSocket::readyRead, [&]() {
        QByteArray response = socket.readAll();
        qDebug() << "Sever answer:" << response;
        QCoreApplication::quit();
    });

    QTimer::singleShot(5000, &app, &QCoreApplication::quit);

    socket.connectToHost("127.0.0.1", 12345);

    return app.exec();
}
