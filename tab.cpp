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

bool Tab::loadFile(const QFileInfo& file)
{
    auto result = m_resource->load(file.absoluteFilePath());
    if(result == ResourceOperationResult::Success) {
        m_sourceView->setResource(m_resource);
        m_graphicView->setResource(m_resource);
        return true;
    } else {
        QMessageBox::critical(this, "File opening failed", "Error code: " + Resource::operationResultString(result));
        return false;
    }
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
