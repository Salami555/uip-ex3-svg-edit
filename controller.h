#pragma once

#include <QObject>

class Model;


class Controller : public QObject
{
    Q_OBJECT

public:
    explicit Controller();

    void setModel(Model * model);
    Model * model() const;

signals:

public slots:
    void newResource() const;
    void openResource(const QString & file) const;
    void saveResource() const;
    void closeResource() const;

    void modifyResource(const QString & source) const;

    void exit(int code = 0);

protected:
    Model * m_model;

};
