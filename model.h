#pragma once

#include <QObject>

class MainWindow;
class Resource;
enum class ResourceOperationResult;

class Model : public QObject
{
    Q_OBJECT

public:
    explicit Model();
    ~Model();

    void newResource();
    void openResource(const QString & file);
    void modifyResource(const QString & source);
    void saveResource();
    void closeResource();

    const Resource * resource() const;

signals:
    void resourceOpened() const;
    void resourceModified() const;
    void resourceSaved() const;
    void resourceClosed() const;

    void operationFailed(ResourceOperationResult result) const;

public slots:


protected:
    Resource * m_resource;
};
