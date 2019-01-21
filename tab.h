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

    bool loadFile(const QFileInfo& file);
    bool saveFile();
    bool saveFileAs(const QFileInfo& file);

    void sourceChanged();

    void swapContentPositions(bool restoreSize = true);
    bool isDefaultPositioned() const;

    QString name(bool windowTitleReady = false) const;

    GraphicsView * graphicsView() const;
    SourceView * sourceView() const;

    const Resource * resource() const;

protected:
    GraphicsView * m_graphicView;
    SourceView * m_sourceView;

    Resource * m_resource;

private:
    bool m_defaultPositioned = true;
};
