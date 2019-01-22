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

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;

    const QFont font() const;
    bool hasHighlighting() const;
    bool hasWordWrap() const;
    QString source() const;

signals:
    void highlightingChanged(bool enabled) const;
    void wordWrapChanged(bool enabled) const;
    void sourceChanged() const;
    void undoAvailable(bool available) const;
    void redoAvailable(bool available) const;

public slots:
    void setResource(const Resource * resource);

    void setFont(const QFont & font);
    void setHighlighting(bool enable);
    void setWordWrap(bool enable);

    void on_undoAvailable(bool available);
    void on_redoAvailable(bool available);

    void undo();
    void redo();
    void cut();
    void copy() const;
    void paste();
    void selectAll() const;
    void find();
    void replace();
    void gotoLine();

protected slots:
    void updateStatusData() const;

protected:
    QTextEdit * m_textEdit = nullptr;
    QLabel * m_statusLabel = nullptr;
    QSyntaxHighlighter * m_highlighter = nullptr;

    bool m_undoAvailable = false, m_redoAvailable = false;
};
