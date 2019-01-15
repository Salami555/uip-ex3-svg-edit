#include "graphicsview.h"

#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLabel>
#include <QMouseEvent>
#include <QPointF>
#include <QRectF>
#include <QPalette>

#include <QGraphicsSvgItem>
#include <QSvgRenderer>

#include "resource.h"

#include <QDebug>


GraphicsView::GraphicsView(QWidget * parent) : QWidget(parent)
{
    this->setLayout(new QVBoxLayout());
    auto layout = this->layout();

    m_graphicsView = new QGraphicsView(new QGraphicsScene());
    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    m_graphicsView->setMouseTracking(true);
    m_graphicsView->viewport()->installEventFilter(this);

    auto palette = this->palette();
    palette.setBrush(QPalette::ColorRole::Base, QBrush(QImage(":/ps-background")));
    setPalette(palette);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_graphicsView->setRenderHints(QPainter::RenderHint::HighQualityAntialiasing | QPainter::RenderHint::Antialiasing);

    m_statusLabel = new QLabel();
    m_statusLabel->setDisabled(true); // just for the grayish look

    layout->addWidget(m_graphicsView);
    layout->addWidget(m_statusLabel);
}

void GraphicsView::setResource(const Resource * resource, const bool reset)
{
    m_resource = resource;
    auto scene = m_graphicsView->scene();
    if(resource == nullptr) {
        scene->clear();
        scene->setSceneRect(0, 0, 0, 0);
        m_graphicsView->centerOn(0, 0);
        return;
    }

    m_graphicsItem = const_cast<Resource *>(resource)->graphicsItem();
    if(m_graphicsItem == nullptr) {
        return;
    }

    scene->clear();
    scene->addItem(m_graphicsItem);
    m_graphicsView->setSceneRect(m_graphicsItem->boundingRect());

    if(reset) {
        m_graphicsView->centerOn(m_graphicsItem);
    }
    applyFitView();
}

void GraphicsView::reloadFromResource()
{
    m_graphicsItem->renderer()->load(m_resource->source().toUtf8());
    applyFitView();
}

void GraphicsView::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event)
    applyFitView();
}

void GraphicsView::mouseMoveEvent(QMouseEvent * event)
{
    updateStatusData(event->pos());
    event->accept();
}


bool GraphicsView::eventFilter(QObject * obj, QEvent * event)
{
    if(event->type() == QEvent::MouseMove) {
        const auto mouseEvent = static_cast<QMouseEvent *>(event);
        updateStatusData(mouseEvent->pos());
    }
    return QWidget::eventFilter(obj, event);
}


void GraphicsView::setFitView(bool enabled)
{
    if(m_fitView == enabled) {
        return;
    }
    m_fitView = enabled;
    emit fitViewChanged(enabled);

    applyFitView();
}

void GraphicsView::setZoom(qreal zoom)
{
    setFitView(false);
    m_graphicsView->scale(zoom, zoom);
}

void GraphicsView::applyFitView() const
{
    if(m_fitView == false || m_graphicsView->scene() == nullptr) {
        return;
    }
    m_graphicsView->fitInView(m_graphicsView->sceneRect(), Qt::KeepAspectRatio);
}

void GraphicsView::updateStatusData(const QPoint & position) const {

    static const auto status = QString("x %1, y %2"); //, hex %3");
    if(this->childAt(position) != m_graphicsView->viewport()) {
        m_statusLabel->setText(status.arg("").arg("")); //.arg(""));
        return;
    }

    const auto coordinates = m_graphicsView->mapToScene(position);

//    QPixmap pixmap(1, 1);
//    QPainter painter(&pixmap);
//    m_graphicsView->render(&painter, QRect(0, 0, 1, 1), QRect(position.x(), position.y(), 1, 1));

//    const auto color = QColor(pixmap.toImage().pixel(0, 0));
    const auto statusWithData = QString(status).arg(coordinates.x()).arg(coordinates.y()); //.arg(color.name());

    m_statusLabel->setText(statusWithData);
}

