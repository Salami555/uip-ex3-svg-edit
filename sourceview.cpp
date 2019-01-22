#include "sourceview.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QRegExp>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

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

    layout->addWidget(m_textEdit);

    connect(m_textEdit, &QTextEdit::cursorPositionChanged, this, &SourceView::updateStatusData);
    connect(m_textEdit, &QTextEdit::textChanged, this, &SourceView::updateStatusData);
    connect(m_textEdit, &QTextEdit::textChanged, this, &SourceView::sourceChanged);

    connect(m_textEdit, &QTextEdit::undoAvailable, this, &SourceView::on_undoAvailable);
    connect(m_textEdit, &QTextEdit::redoAvailable, this, &SourceView::on_redoAvailable);
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

void SourceView::setFont(const QFont & font)
{
    m_textEdit->setFont(font);
}

const QFont SourceView::font() const
{
    return m_textEdit->font();
}

void SourceView::setHighlighting(bool enable)
{
    if(enable == false && m_highlighter->document() == nullptr) {
        return;
    }
    m_highlighter->setDocument(enable ? m_textEdit->document() : nullptr);
    emit highlightingChanged(enable);
}

bool SourceView::hasHighlighting() const
{
    return m_highlighter->document() != nullptr;
}

void SourceView::setWordWrap(bool enable)
{
    if(enable == false && m_textEdit->wordWrapMode() == QTextOption::NoWrap) {
        return;
    }
    m_textEdit->setWordWrapMode(enable ? QTextOption::WrapAnywhere : QTextOption::NoWrap);
    emit wordWrapChanged(enable);
}

bool SourceView::hasWordWrap() const
{
    return m_textEdit->wordWrapMode() != QTextOption::NoWrap;
}

void SourceView::on_undoAvailable(bool available)
{
    m_undoAvailable = available;
    emit undoAvailable(available);
}
bool SourceView::isUndoAvailable() const
{
    return m_undoAvailable;
}

void SourceView::on_redoAvailable(bool available)
{
    m_redoAvailable = available;
    emit redoAvailable(available);
}
bool SourceView::isRedoAvailable() const
{
    return m_redoAvailable;
}

void SourceView::undo()
{
    m_textEdit->undo();
}

void SourceView::redo()
{
    m_textEdit->redo();
}

void SourceView::cut()
{
    m_textEdit->cut();
}

void SourceView::copy() const
{
    m_textEdit->copy();
}

void SourceView::paste()
{
    m_textEdit->paste();
}

void SourceView::selectAll() const
{
    m_textEdit->selectAll();
}

void SourceView::find()
{
    QString searchText = m_textEdit->textCursor().selectedText();
    bool aborted = false;
    while(!aborted) {
        searchText = QInputDialog::getText(this, "Find", "search text", QLineEdit::Normal, searchText);
        if(!searchText.isEmpty()) {
            auto match = m_textEdit->document()->find(searchText, m_textEdit->textCursor());
            if(!match.isNull()) {
                m_textEdit->setTextCursor(match);
            } else {
                match = m_textEdit->document()->find(searchText);
                if(!match.isNull()) {
                    m_textEdit->setTextCursor(match);
                } else {
                    aborted = true;
                    QMessageBox::information(this, "Find", "The search text '" + searchText + "' was not found.");
                }
            }
        } else {
            aborted = true;
        }
    }
}

void SourceView::replace()
{
    QString searchText = m_textEdit->textCursor().selectedText();
    searchText = QInputDialog::getText(this, "Replace", "search text", QLineEdit::Normal, searchText);
    if(searchText.isEmpty()) return;

    bool aborted = false, fastForward = false, restarted = false;
    if(!m_textEdit->document()->find(searchText).isNull()) {
        QString replaceText = QInputDialog::getText(this, "Replace", "replace text", QLineEdit::Normal, searchText);
        if(replaceText.isEmpty()) return;
        m_textEdit->textCursor().clearSelection();
        while(!aborted) {
            auto match = m_textEdit->document()->find(searchText, m_textEdit->textCursor());
            if(!match.isNull()) {
                m_textEdit->setTextCursor(match);
                auto button = fastForward
                        ? QMessageBox::Yes
                        : QMessageBox::question(
                            this, "Replace", "Replace this occurrence?",
                            QMessageBox::Yes|QMessageBox::YesToAll|QMessageBox::No|QMessageBox::Abort,
                            QMessageBox::Abort
                        );
                switch(button) {
                case QMessageBox::Yes:
                    m_textEdit->textCursor().removeSelectedText();
                    m_textEdit->textCursor().insertText(replaceText);
                    continue;
                case QMessageBox::No:
                    continue;
                case QMessageBox::YesToAll:
                    fastForward = true;
                    continue;
                default:
                    aborted = true;
                    continue;
                }
            } else {
                if(restarted) {
                    aborted = true;
                }
                m_textEdit->setTextCursor(QTextCursor());
                restarted = true;
            }
        }
    }
    if(fastForward) {
        QMessageBox::information(this, "Replace", "All occurrences of '" + searchText + "' replaced.");
    } else {
        QMessageBox::information(this, "Replace", "The search text '" + searchText + "' was not found.");
    }
}

void SourceView::gotoLine()
{
    auto cursor = m_textEdit->textCursor();
    auto document = m_textEdit->document();
    int currentLineIndex = cursor.blockNumber();
    int lineCount = document->blockCount();
    int newLineIndex = QInputDialog::getInt(this, "Go to line", "line number", currentLineIndex + 1, 1, lineCount) - 1;
    m_textEdit->setTextCursor(QTextCursor(document->findBlockByLineNumber(newLineIndex)));
}

void SourceView::updateStatusData() const
{
    const auto cursor = m_textEdit->textCursor();

    if(cursor.selectedText().isEmpty()) {
        const auto line = cursor.blockNumber() + 1;
        const auto column = cursor.positionInBlock();
        emit statusDataChanged(QString("Line %1, Column %2").arg(line).arg(column));

    } else {
        const auto numSelectedCharacters = cursor.selectionEnd() - cursor.selectionStart();
        const auto numSelectedLines = cursor.selection().toPlainText().count("\n");
        const auto selectedLinesStatus = numSelectedLines > 1 ? QString("%1 lines, ").arg(numSelectedLines) : QString("");
        emit statusDataChanged(QString("%1%2 characters selected").arg(selectedLinesStatus).arg(numSelectedCharacters));
    }
}

