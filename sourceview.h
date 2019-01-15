#pragma once

#include <QWidget>

class QTextEdit;
class QLabel;
class QSyntaxHighlighter;

class Resource;


class SourceView: public QWidget
{
    Q_OBJECT

public:
    explicit SourceView(QWidget * parent = nullptr);

    QString source() const;

signals:
    void highlightChanged(bool enabled) const;
    void wordWrapChanged(bool enabled) const;
    void sourceChanged() const;

public slots:
    void setResource(const Resource * resource);

    void setHighlighting(bool enable) const;
    void setWordWrap(bool enable) const;

    void selectAll() const;

protected slots:
    void updateStatusData() const;

protected:
    QTextEdit * m_textEdit = nullptr;
    QLabel * m_statusLabel = nullptr;
    QSyntaxHighlighter * m_highlighter = nullptr;

};
