#include "model.h"

#include "mainwindow.h"
#include "resource.h"


Model::Model() :
    m_resource(nullptr)
{
}

Model::~Model()
{
    assert(m_resource == nullptr);
}

const Resource * Model::resource() const
{
    return m_resource;
}

void Model::newResource()
{
    assert(m_resource == nullptr);

    m_resource = new Resource();
    auto opResult = m_resource->load(":/picasso-dog");
    if(opResult == ResourceOperationResult::Success) {
        emit resourceOpened();
        return;
    }
    emit operationFailed(opResult);
}

void Model::openResource(const QString & file)
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

void Model::saveResource()
{
    assert(m_resource != nullptr);

    auto opResult = m_resource->save();
    if(opResult == ResourceOperationResult::Success) {
        emit resourceSaved();
        return;
    }
    emit operationFailed(opResult);
}

void Model::closeResource()
{
    if(m_resource == nullptr) {
        return;
    }
    delete m_resource;
    m_resource = nullptr;

    emit resourceClosed();
}

void Model::modifyResource(const QString & source)
{
    if(m_resource == nullptr) {
        return;
    }
    m_resource->setSource(source);
    emit resourceModified();
}

