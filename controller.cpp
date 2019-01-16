#include "controller.h"

#include <QApplication>

// #include "tabmodel.h"
#include "mainwindow.h"

#include <QDebug>

Controller::Controller()
{

}

// void Controller::setModel(Model * model)
// {
//     if(m_model == model) {
//         return;
//     }
//     m_model = model;
// }

// Model * Controller::model() const
// {
//     return m_model;
// }

void Controller::newResource() const
{
    // m_model->closeResource();
    // m_model->newResource();
}

void Controller::openResources(const QStringList & filePaths) const
{
    QList<QFileInfo> files;
    for(auto filePath : filePaths) {
        files.append(QFileInfo(filePath));
    }
    this->openResources(files);
}

void Controller::openResources(const QList<QFileInfo> & files) const
{
    QSet<QString> paths;
    for(auto file : files) {
        if(file.exists() && file.isFile()) {
            paths.insert(file.absoluteFilePath());
        }
    }
    for(auto filePath : paths) {
        this->openResource(filePath);
    }
}

void Controller::openResource(const QString & file) const
{
    // m_model->closeResource();
    // m_model->openResource(file);
    qDebug() << file;
}

void Controller::saveResource() const
{
    // m_model->saveResource();
}

void Controller::closeResource() const
{
    // m_model->closeResource();
}

void Controller::modifyResource(const QString & source) const
{
    // m_model->modifyResource(source);
}


void Controller::exit(const int code)
{
    // m_model->closeResource();
    QApplication::exit(code);
}
