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

    const QFont font() const;
    bool hasHighlighting() const;
    bool hasWordWrap() const;
    QString source() const;

signals:
    void highlightingChanged(bool enabled) const;
    void wordWrapChanged(bool enabled) const;
    void sourceChanged() const;

public slots:
    void setResource(const Resource * resource);

    void setFont(const QFont & font);
    void setHighlighting(bool enable);
    void setWordWrap(bool enable);

    void selectAll() const;

protected slots:
    void updateStatusData() const;

protected:
    QTextEdit * m_textEdit = nullptr;
    QLabel * m_statusLabel = nullptr;
    QSyntaxHighlighter * m_highlighter = nullptr;

};
