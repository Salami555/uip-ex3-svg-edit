#pragma once

#include <QWidget>

class Resource;

class QGraphicsView;
class QGraphicsSvgItem;
class QLabel;

class MouseEventGrabbedScene;


class GraphicsView : public QWidget
{
    Q_OBJECT

public:
    explicit GraphicsView(QWidget * parent = nullptr);

    bool shouldFitView() const;
    qreal zoom() const;
    void zoomIn();
    void zoomOut();
    void setZoom(qreal zoom);
    void setFitView(bool enabled);

protected:
    void resizeEvent(QResizeEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

    void applyFitView() const;


signals:
    void fitViewChanged(bool enabled) const;

public slots:
    void setResource(const Resource * resource, bool reset = false);
    void reloadFromResource();

protected slots:
    void updateStatusData(const QPoint & position) const;

protected:
    const Resource * m_resource = nullptr; // will be removed

    QGraphicsView * m_graphicsView = nullptr;
    QGraphicsSvgItem * m_graphicsItem = nullptr;

    QLabel * m_statusLabel = nullptr;

    qreal m_zoom = 1;
};
