#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMap>
#include <QString>
#include <QVector>
#include "json.hpp"


struct GuiEntity {
    int id;
    QString type;
    float x, y;
    float hp;
    QString texture;
    float width;
    float height;
};

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    MyOpenGLWidget(QWidget* parent = nullptr);

    // Обновление данных сцены из JSON (от сервера)
    void updateSceneFromJson(const QByteArray& data);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QVector<GuiEntity> entities;
    QMap<QString, QImage> textures;
    float cameraZoom = 10.0f;

    void drawEntity(const GuiEntity& entity,  QPainter& painter);

};

#endif // MYOPENGLWIDGET_H
