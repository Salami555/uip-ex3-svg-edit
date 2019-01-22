#include "tab.h"

#include <QTabWidget>
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

    connect(m_sourceView, &SourceView::sourceChanged, this, &Tab::sourceChanged);
}

Tab::~Tab()
{
}

bool Tab::loadFile(const QFileInfo& file)
{
    auto result = m_resource->load(file.absoluteFilePath());
    m_sourceView->setResource(m_resource);
    m_graphicView->setResource(m_resource);
    if(result != ResourceOperationResult::Success) {
        QMessageBox::critical(
            this, "File reading failed",
            "An error occured while reading the file.\nError code: "
                + Resource::operationResultString(result)
                + "\nwith: " + m_resource->file().fileName()
        );
    }
    return true;
}

void Tab::loadEmptyTemplate()
{
    auto result = m_resource->load(QFileInfo(":/picasso-dog"), false);
    assert(result == ResourceOperationResult::Success);
    m_sourceView->setResource(m_resource);
    m_graphicView->setResource(m_resource);
    m_resource->setDirty(true);
    this->sourceChanged();
}

bool Tab::saveFile()
{
    auto result = m_resource->save();
    if(result == ResourceOperationResult::Success) {
        this->setWindowModified(false);
        return true;
    } else {
        QMessageBox::critical(
            this, "File saving failed",
            "An error occured while saving the file.\nError code: "
                + Resource::operationResultString(result)
                + "\nwith: " + m_resource->file().fileName()
        );
        return false;
    }
}

bool Tab::saveFileAs(const QFileInfo& file)
{
    auto result = m_resource->save(file);
    if(result == ResourceOperationResult::Success) {
        this->setWindowModified(false);
        return true;
    } else {
        QMessageBox::critical(
            this, "File saving failed",
            "Error code: " + Resource::operationResultString(result)
                + "\nwith: " + m_resource->file().fileName()
        );
        return false;
    }
}

void Tab::sourceChanged()
{
    auto result = m_resource->setSource(m_sourceView->source());
    if(result == ResourceOperationResult::Success) {
        m_graphicView->reloadFromResource();
        this->setWindowModified(true);
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

QString Tab::name(bool withExtension, bool windowTitleReady) const
{
    QString name = m_resource->hasFile()
        ? (withExtension
            ? m_resource->file().fileName()
            : m_resource->file().baseName())
        : "Unnamed file";
    if(windowTitleReady) {
        name += "[*]";
    } else if(m_resource->isUnsaved()) {
        name += "*";
    }
    return name;
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
