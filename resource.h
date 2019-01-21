#pragma once

#include <QObject>
#include <QScopedPointer>
#include <QTemporaryFile>
#include <QXmlInputSource>
#include <QGraphicsSvgItem>
#include <QFileInfo>

class QString;


/*

auto r = Resource();
r.setNodeContent("...");
r.nodeContent("..");
r.graphicsItem();
r.load(...)
r.save(...)

*/

enum class ResourceOperationResult {
    Success,
    FileNotFound,
    FileOpenFailed,
    FileSaveFailed,
    XmlParseFailed,
};


class Resource : public QObject
{
    Q_OBJECT

public:
    static QString operationResultString(ResourceOperationResult result);

public:
    explicit Resource();

    ResourceOperationResult load(const QFileInfo & file);
    ResourceOperationResult save();
    ResourceOperationResult save(const QFileInfo & file);

    ResourceOperationResult setSource(const QString & source);
    QString source() const;

    QGraphicsSvgItem * graphicsItem();

    bool hasFile() const;
    QFileInfo file() const;

    bool isUnsaved() const;

protected:
    bool validate() const;

signals:

public slots:


protected:
    QFileInfo m_fileInfo;
    bool m_modified = false;

    QScopedPointer<QXmlInputSource> m_xmlSource;

    mutable QScopedPointer<QTemporaryFile> m_tempFile;
    mutable QScopedPointer<QGraphicsSvgItem> m_graphicsItem;

    QString m_rawSource;

};
