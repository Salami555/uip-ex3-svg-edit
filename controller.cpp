#include "controller.h"

#include <QApplication>

#include "model.h"
#include "mainwindow.h"


Controller::Controller()
{

}

void Controller::setModel(Model * model)
{
    if(m_model == model) {
        return;
    }
    m_model = model;
}

Model * Controller::model() const
{
    return m_model;
}

void Controller::newResource() const
{
    m_model->closeResource();
    m_model->newResource();
}

void Controller::openResource(const QString & file) const
{
    m_model->closeResource();
    m_model->openResource(file);
}
void Controller::saveResource() const
{
    m_model->saveResource();
}

void Controller::closeResource() const
{
    m_model->closeResource();
}

void Controller::modifyResource(const QString & source) const
{
    m_model->modifyResource(source);
}


void Controller::exit(const int code)
{
    m_model->closeResource();
    QApplication::exit(code);
}
