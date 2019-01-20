#pragma once

#include <QWidget>
#include <QSplitter>

class GraphicsView;
class SourceView;


class Tab: public QSplitter
{
    Q_OBJECT

public:
    explicit Tab(QWidget * parent = nullptr);
    ~Tab();

    bool hasPendingChanges() const;

protected:
    GraphicsView * m_graphicView;
    SourceView * m_sourceView;

};
