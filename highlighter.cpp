#include "highlighter.h"


Highlighter::Highlighter(QTextDocument * parent) :
    QSyntaxHighlighter(parent)
{
    setRegexes();
    setFormats();
}

void Highlighter::highlightBlock(const QString & text)
{
    // Special treatment for xml element regex as we use captured text to emulate lookbehind
    int xmlElementIndex = m_typeRegex.indexIn(text);
    while(xmlElementIndex >= 0)
    {
        int matchedPos = m_typeRegex.pos(1);
        int matchedLength = m_typeRegex.cap(1).length();
        setFormat(matchedPos, matchedLength, m_typeFormat);

        xmlElementIndex = m_typeRegex.indexIn(text, matchedPos + matchedLength);
    }

    // Highlight xml keywords *after* xml elements to fix any occasional / captured into the enclosing element
    typedef QList<QRegExp>::const_iterator Iter;
    Iter xmlKeywordRegexesEnd = m_elementRegexes.end();
    for(Iter it = m_elementRegexes.begin(); it != xmlKeywordRegexesEnd; ++it) {
        const QRegExp & regex = *it;
        highlightByRegex(m_elementFormat, regex, text);
    }

    highlightByRegex(m_attributeFormat, m_attributeRegex, text);
    highlightByRegex(m_commentFormat, m_commentRegex, text);
    highlightByRegex(m_valueFormat, m_valueRegex, text);
}

void Highlighter::highlightByRegex(const QTextCharFormat & format, const QRegExp & regex, const QString & text)
{
    int index = regex.indexIn(text);

    while(index >= 0)
    {
        int matchedLength = regex.matchedLength();
        setFormat(index, matchedLength, format);

        index = regex.indexIn(text, index + matchedLength);
    }
}

void Highlighter::setRegexes()
{
    m_typeRegex.setPattern("<[\\s]*([^\\n]\\w*)(?=[\\s/>])");
    m_attributeRegex.setPattern("[\\w-:]+(?=\\=)");
    m_valueRegex.setPattern("\"[^\\n\"]+\"(?=[\\s/>])");
    m_commentRegex.setPattern("<!--[^\\n]*-->");

    m_elementRegexes = QList<QRegExp>{ QRegExp("<"), QRegExp("<\\?"), QRegExp("/>"), QRegExp(">"),
        QRegExp("</"), QRegExp("</"), QRegExp("\\?>") };
}

void Highlighter::setFormats()
{
    m_elementFormat.setForeground(QColor("#657b83"));
    m_typeFormat.setForeground(QColor("#268bd2"));
    m_attributeFormat.setForeground(QColor("#93a1a1"));
    m_valueFormat.setForeground(QColor("#2aa198"));

    m_commentFormat.setForeground(QColor("#268bd2"));
}
