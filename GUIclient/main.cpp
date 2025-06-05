#include <QApplication>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>
#include "MyOpenGLWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MyOpenGLWidget widget;
    widget.resize(800, 600);
    widget.show();

    QTcpSocket* socket = new QTcpSocket(&app);

    // 1. Получаем данные от сервера и обновляем сцену
    QObject::connect(socket, &QTcpSocket::readyRead, [&]() {
        QByteArray response = socket->readAll();
        qDebug() << "[GUI] Получен ответ от сервера (size:" << response.size() << "):" << response;
        widget.updateSceneFromJson(response);
    });

    // 2. После подключения сразу запросить состояние сцены
    QObject::connect(socket, &QTcpSocket::connected, [&]() {
        qDebug() << "[GUI] Отправлен запрос get_state";
        socket->write("get_state");
        socket->flush();
    });

    // 3. Периодически запрашиваем состояние (например, раз в 300 мс)
    QTimer* timer = new QTimer(&app);
    QObject::connect(timer, &QTimer::timeout, [=]() {
        qDebug() << "[GUI] Отправлен запрос get_state";
        socket->write("get_state");
        socket->flush();
    });
    timer->start(300);

    socket->connectToHost("127.0.0.1", 12345);

    return app.exec();
}
