#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>

// Taken from https://github.com/d1vanov/basic-xml-syntax-highlighter

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument * parent);

protected:
    virtual void highlightBlock(const QString & text);

private:
    void highlightByRegex(
        const QTextCharFormat & format,
        const QRegExp & regex, const QString & text);

    void setRegexes();
    void setFormats();

private:
    QTextCharFormat m_elementFormat;
    QTextCharFormat m_typeFormat;
    QTextCharFormat m_attributeFormat;
    QTextCharFormat m_valueFormat;
    QTextCharFormat m_commentFormat;

    QList<QRegExp> m_elementRegexes;
    QRegExp m_typeRegex;
    QRegExp m_attributeRegex;
    QRegExp m_valueRegex;
    QRegExp m_commentRegex;

};
