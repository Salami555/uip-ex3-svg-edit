#include "tabmodel.h"

#include "mainwindow.h"
#include "resource.h"


TabModel::TabModel() :
    m_resource(nullptr)
{
}

TabModel::~TabModel()
{
    assert(m_resource == nullptr);
}

const Resource * TabModel::resource() const
{
    return m_resource;
}

void TabModel::newResource()
{
    assert(m_resource == nullptr);

    m_resource = new Resource();
    auto opResult = m_resource->load(QFileInfo(":/picasso-dog"));
    if(opResult == ResourceOperationResult::Success) {
        emit resourceOpened();
        return;
    }
    emit operationFailed(opResult);
}

void TabModel::openResource(const QString & file)
{
    assert(m_resource == nullptr);

    m_resource = new Resource();
    auto opResult = m_resource->load(file);
    if(opResult == ResourceOperationResult::Success) {
        emit resourceOpened();
        return;
    }
    emit operationFailed(opResult);
}

void TabModel::saveResource()
{
    assert(m_resource != nullptr);

    auto opResult = m_resource->save();
    if(opResult == ResourceOperationResult::Success) {
        emit resourceSaved();
        return;
    }
    emit operationFailed(opResult);
}

void TabModel::closeResource()
{
    if(m_resource == nullptr) {
        return;
    }
    delete m_resource;
    m_resource = nullptr;

    emit resourceClosed();
}

void TabModel::modifyResource(const QString & source)
{
    if(m_resource == nullptr) {
        return;
    }
    m_resource->setSource(source);
    emit resourceModified();
}

