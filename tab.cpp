#include "tab.h"

#include <QLayout>
#include <QMessageBox>
#include <QDebug>

#include "graphicsview.h"
#include "sourceview.h"


Tab::Tab(QWidget * parent) : QSplitter(parent)
{
    auto graphicView = new GraphicsView();
    this->m_graphicView = graphicView;
    auto sourceView = new SourceView();
    this->m_sourceView = sourceView;

    this->setOrientation(Qt::Orientation::Horizontal);
    this->addWidget(this->m_sourceView);
    this->addWidget(this->m_graphicView);
}

Tab::~Tab()
{
    qDebug() << "closing";
}

bool Tab::hasPendingChanges() const
{
    return true;
}
