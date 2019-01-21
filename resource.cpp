#include "resource.h"

#include <QFile>
#include <QXmlSimpleReader>
#include <QXmlInputSource>

#include <QGraphicsSvgItem>
#include <QString>
#include <QTemporaryFile>

#include <QDomDocument>

#include <QDebug>


QString Resource::operationResultString(const ResourceOperationResult result) {

    static const QMap<ResourceOperationResult, QString> stringsByResult{
    { ResourceOperationResult::Success,        QString("Success")},
    { ResourceOperationResult::FileNotFound,   QString("FileNotFound")},
    { ResourceOperationResult::FileOpenFailed, QString("FileOpenFailed")},
    { ResourceOperationResult::FileSaveFailed, QString("FileSaveFailed")},
    { ResourceOperationResult::XmlParseFailed, QString("XmlParseFailed")}};

    return stringsByResult[result];
}


Resource::Resource():
    m_xmlSource(new QXmlInputSource())
{
}

bool Resource::validate() const
{
    m_xmlSource->reset();

    QXmlSimpleReader xmlReader;
    return xmlReader.parse(m_xmlSource.get());
}

ResourceOperationResult Resource::load(const QFileInfo & file)
{
    if(!file.exists() || !file.isFile()) {
        return ResourceOperationResult::FileNotFound;
    }

    QScopedPointer<QFile> svgFile(new QFile(file.filePath()));
    svgFile->open(QIODevice::ReadOnly | QIODevice::Text);
    if(!svgFile->isOpen()) {
        return ResourceOperationResult::FileOpenFailed;
    }

    m_xmlSource->setData(svgFile->readAll());
    svgFile->close();

    if(!validate()) {
        return ResourceOperationResult::XmlParseFailed;
    }
    m_modified = false;

    m_fileInfo = file;
    return ResourceOperationResult::Success;
}

ResourceOperationResult Resource::save()
{
    return save(m_fileInfo);
}

ResourceOperationResult Resource::save(const QFileInfo & file)
{
    QScopedPointer<QFile> svgFile(new QFile(file.filePath()));
    svgFile->open(QIODevice::WriteOnly | QIODevice::Text);
    if(!svgFile->isOpen()) {
        return ResourceOperationResult::FileSaveFailed;
    }

    svgFile->write(m_xmlSource->data().toUtf8());
    svgFile->close();

    m_modified = false;
    m_fileInfo = file;
    return ResourceOperationResult::Success;
}


ResourceOperationResult Resource::setSource(const QString & source)
{
    if(m_xmlSource->data() == source) {
        return ResourceOperationResult::Success;
    }
    m_xmlSource->setData(source);
    m_modified = true;

    if(!validate()) {
        return ResourceOperationResult::XmlParseFailed;
    }
    emit changed();
    return ResourceOperationResult::Success;
}

QString Resource::source() const
{
    return m_xmlSource->data();
}

QGraphicsSvgItem * Resource::graphicsItem()
{
    if(!validate()) {
        return nullptr;
    }

    if(m_tempFile.isNull()) {
        m_tempFile.reset(new QTemporaryFile());
        m_tempFile->setAutoRemove(true);
    }

    // Little workaround for now, since i (DL) didn't find a setter based on QByteArray.
    m_tempFile->open();
    m_tempFile->write(m_xmlSource->data().toUtf8());
    m_tempFile->close();

    m_graphicsItem.reset(new QGraphicsSvgItem(m_tempFile->fileName()));
    return m_graphicsItem.get();
}

bool Resource::hasFile() const
{
    return m_fileInfo.isFile(); // could use exists
}

QFileInfo Resource::file() const
{
    return m_fileInfo;
}

bool Resource::isUnsaved() const
{
    return m_modified;
}
