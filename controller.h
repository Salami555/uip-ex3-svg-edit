#pragma once

#include <QObject>
#include <QList>
#include <QFileInfo>

// class Model;


class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller();

    // void setModel(Model * model);
    // Model * model() const;

signals:

public slots:
    void newResource() const;
    void openResources(const QStringList & filePaths) const;
    void openResources(const QList<QFileInfo> & files) const;
    void openResource(const QString & file) const;
    void saveResource() const;
    void closeResource() const;

    void modifyResource(const QString & source) const;

    void exit(int code = 0);

protected:
    // Model * m_model;

};
