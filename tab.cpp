#include "tab.h"

#include <QLayout>
#include <QMessageBox>
#include <QDebug>

#include "graphicsview.h"
#include "sourceview.h"
#include "resource.h"


Tab::Tab(QWidget * parent) : QSplitter(parent)
{
    m_resource = new Resource();

    auto graphicView = new GraphicsView();
    m_graphicView = graphicView;
    auto sourceView = new SourceView();
    m_sourceView = sourceView;

    this->setOrientation(Qt::Orientation::Horizontal);
    this->addWidget(m_sourceView);
    this->addWidget(m_graphicView);
}

Tab::~Tab()
{
}

void Tab::loadFile(const QFileInfo& file)
{
    auto opResult = m_resource->load(file.absoluteFilePath());
}

GraphicsView * Tab::graphicsView() const
{
    return m_graphicView;
}

SourceView * Tab::sourceView() const
{
    return m_sourceView;
}

const Resource * Tab::resource() const
{
    return m_resource;
}
