#include "sourceview.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QRegExp>

#include "resource.h"
#include "highlighter.h"


SourceView::SourceView(QWidget * parent) : QWidget(parent)
{
    this->setLayout(new QVBoxLayout());
    auto layout = this->layout();

    m_textEdit = new QTextEdit();
    m_textEdit->blockSignals(true);

    m_highlighter = new Highlighter(m_textEdit->document());

    auto palette = m_textEdit->palette();
    palette.setColor(QPalette::Base, QColor("#002b36"));
    palette.setColor(QPalette::Text, QColor("#bbbbbb"));
    m_textEdit->setPalette(palette);

    m_statusLabel = new QLabel();
    m_statusLabel->setDisabled(true); // just for the grayish look

    layout->addWidget(m_textEdit);
    layout->addWidget(m_statusLabel);

    connect(m_textEdit, &QTextEdit::cursorPositionChanged, this, &SourceView::updateStatusData);
    connect(m_textEdit, &QTextEdit::textChanged, this, &SourceView::updateStatusData);
    connect(m_textEdit, &QTextEdit::textChanged, this, &SourceView::sourceChanged);
}

QString SourceView::source() const
{
    return m_textEdit->toPlainText();
}

void SourceView::setResource(const Resource * resource)
{
    m_textEdit->blockSignals(true);
    m_textEdit->clear();

    if(resource == nullptr) {
        return;
    }
    m_textEdit->setText(const_cast<Resource *>(resource)->source());
    m_textEdit->blockSignals(false);
}

void SourceView::setHighlighting(bool enable) const
{
    if(enable == false && m_highlighter->document() == nullptr) {
        return;
    }
    m_highlighter->setDocument(enable ? m_textEdit->document() : nullptr);
    emit highlightChanged(enable);
}

void SourceView::setWordWrap(bool enable) const
{
    if(enable == false && m_textEdit->wordWrapMode() == QTextOption::NoWrap) {
        return;
    }
    m_textEdit->setWordWrapMode(enable ? QTextOption::WrapAnywhere : QTextOption::NoWrap);
    emit wordWrapChanged(enable);
}

void SourceView::selectAll() const
{
    m_textEdit->selectAll();
}

void SourceView::updateStatusData() const
{
    const auto cursor = m_textEdit->textCursor();

    if(cursor.selectedText().isEmpty()) {
        const auto line = cursor.blockNumber() + 1;
        const auto column = cursor.positionInBlock();
        m_statusLabel->setText(QString("Line %1, Column %2").arg(line).arg(column));

    } else {
        const auto numSelectedCharacters = cursor.selectionEnd() - cursor.selectionStart();
        const auto numSelectedLines = cursor.selection().toPlainText().count("\n");
        const auto selectedLinesStatus = numSelectedLines > 1 ? QString("%1 lines, ").arg(numSelectedLines) : QString("");
        m_statusLabel->setText(QString("%1%2 characters selected").arg(selectedLinesStatus).arg(numSelectedCharacters));
    }
}

