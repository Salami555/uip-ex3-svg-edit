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

    ResourceOperationResult load(const QFileInfo & file, bool setFile = true);
    ResourceOperationResult save();
    ResourceOperationResult save(const QFileInfo & file);

    ResourceOperationResult setSource(const QString & source);
    QString source() const;

    QGraphicsSvgItem * graphicsItem();

    bool hasFile() const;
    QFileInfo file() const;

    bool isValid() const;
    bool isUnsaved() const;
    void setDirty(bool unsaved);

protected:
    bool validate();

signals:
    void changed();

public slots:


protected:
    QFileInfo m_fileInfo;
    bool m_valid = false;
    bool m_modified = false;

    QScopedPointer<QXmlInputSource> m_xmlSource;

    mutable QScopedPointer<QTemporaryFile> m_tempFile;
    mutable QScopedPointer<QGraphicsSvgItem> m_graphicsItem;

    QString m_rawSource;

};
