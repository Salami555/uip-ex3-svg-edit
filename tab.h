#pragma once

#include <QWidget>
#include <QFileInfo>
#include <QSplitter>

class GraphicsView;
class SourceView;
class Resource;


class Tab: public QSplitter
{
    Q_OBJECT

public:
    explicit Tab(QWidget * parent = nullptr);
    ~Tab();

    void loadFile(const QFileInfo& file);

    GraphicsView * graphicsView() const;
    SourceView * sourceView() const;

    const Resource * resource() const;

protected:
    GraphicsView * m_graphicView;
    SourceView * m_sourceView;

    Resource * m_resource;
};
