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
    // default: left source, right graphic
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

void Tab::swapContentPositions(bool restoreSize)
{
    const auto widget0 = this->widget(0);
    const auto widget1 = this->widget(1);

    const auto sizes = this->sizes();

    this->insertWidget(0, widget1);
    this->insertWidget(1, widget0);

    if(restoreSize) {
        this->setSizes(sizes);
    }
    m_defaultPositioned = !m_defaultPositioned;
}

bool Tab::isDefaultPositioned() const
{
    return m_defaultPositioned;
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
