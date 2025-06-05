#include "MyOpenGLWidget.h"
#include <QPainter>
#include <QDebug>
#include <QKeyEvent>


MyOpenGLWidget::MyOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void MyOpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void MyOpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    QPainter painter(this);
    for (const GuiEntity& e : entities) {
        drawEntity(e, painter);
    }
    painter.end();
}

void MyOpenGLWidget::drawEntity(const GuiEntity& entity,  QPainter& painter)
{
    int centerX = int(width() * (0.5f + entity.x / cameraZoom));
    int centerY = int(height() * (0.5f - entity.y / cameraZoom));

    QImage img;
    if (!textures.contains(entity.texture)) {
        img = QImage(":/textures/" + entity.texture); // Путь к ресурсам
        textures[entity.texture] = img;
    } else {
        img = textures[entity.texture];
    }
    if (!img.isNull()) {
        // Масштабируем img так, чтобы он занял mesh.width × mesh.height в игровых координатах
        float scaleX = entity.width / img.width();
        float scaleY = entity.height / img.height();

        // Размеры на экране, учитывая камеру
        float drawW = entity.width / cameraZoom * width();
        float drawH = entity.height / cameraZoom * height();

        QRectF target(
            centerX - drawW/2,
            centerY - drawH/2,
            drawW,
            drawH
            );
        painter.drawImage(target, img);
    } else {
        // fallback — кружок такого же размера
        float drawR = entity.width / cameraZoom * width() / 2;
        painter.setBrush(Qt::red);
        painter.drawEllipse(QPointF(centerX, centerY), drawR, drawR);
    }


    painter.setPen(Qt::red);
    painter.drawText(centerX - 10, centerY - 25, QString("HP: %1").arg(entity.hp));
}

void MyOpenGLWidget::updateSceneFromJson(const QByteArray& data)
{
    entities.clear();
    try {
        nlohmann::json j = nlohmann::json::parse(data.toStdString());
        for (const auto& e : j["entities"]) {
            GuiEntity ge;
            ge.id = e.value("id", 0);
            ge.type = QString::fromStdString(e.value("type", ""));
            ge.x = e.value("x", 0.0f);
            ge.y = e.value("y", 0.0f);
            ge.hp = e.value("hp", 0.0f);
            ge.texture = QString::fromStdString(e.value("texture", ""));
            ge.width = e.value("width", 0.0f);
            ge.height = e.value("height", 0.0f);
            entities.push_back(ge);
        }
    } catch (...) {
        qDebug() << "Ошибка парсинга JSON для GUI";
    }
    update(); // Перерисовать сцену
}
