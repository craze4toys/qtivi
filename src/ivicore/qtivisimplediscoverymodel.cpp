/****************************************************************************
**
** Copyright (C) 2015 Pelagicore AG
** Contact: http://www.pelagicore.com/
**
** This file is part of Qt IVI.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt IVI licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Pelagicore. For licensing terms
** and conditions see http://www.pelagicore.com.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtivisimplediscoverymodel.h"
#include "qtiviservicemanager.h"

#include "qtiviserviceobject.h"

QtIVISimpleDiscoveryModelBase::QtIVISimpleDiscoveryModelBase(QString interface, bool autoDiscovery, QObject *parent)
    : QAbstractListModel(parent)
    , m_interface(interface)
    , m_autoDiscovery(autoDiscovery)
{
    connect(this, &QtIVISimpleDiscoveryModelBase::rowsInserted, this, &QtIVISimpleDiscoveryModelBase::countChanged);
    connect(this, &QtIVISimpleDiscoveryModelBase::rowsRemoved, this, &QtIVISimpleDiscoveryModelBase::countChanged);
    connect(this, &QtIVISimpleDiscoveryModelBase::layoutChanged, this, &QtIVISimpleDiscoveryModelBase::countChanged); // FIXME: why?
    connect(this, &QtIVISimpleDiscoveryModelBase::modelReset, this, &QtIVISimpleDiscoveryModelBase::countChanged);
}

QtIVISimpleDiscoveryModelBase::~QtIVISimpleDiscoveryModelBase()
{
}

void QtIVISimpleDiscoveryModelBase::setAutoDiscovery(bool autoDiscovery)
{
    if (m_autoDiscovery == autoDiscovery)
        return;

    m_autoDiscovery = autoDiscovery;
    emit autoDiscoveryChanged(autoDiscovery);
}

int QtIVISimpleDiscoveryModelBase::rowCount(const QModelIndex &parent) const
{
     return parent.isValid() ? 0 : m_serviceObjects.count();
}

QVariant QtIVISimpleDiscoveryModelBase::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    if (row >= 0 && row < m_serviceObjects.count()) {
        QtIVIServiceObject* so = serviceObjectAt(row);

        if (role == ServiceObject)
            return QVariant::fromValue<QtIVIServiceObject*>(so);

        QObject* instance = so->interfaceInstance(m_interface);
        if (!instance)
            return QVariant();

        /* TODO is this casting really needed? The QObject property system works anyway, doesn't it?
        T* obj = qobject_cast<T*>(instance);
        if(!obj)
            return QVariant();
        */

        QVariant ret = /*obj*/ instance->property(m_roles.value(role));
        if(m_enumProperties.contains(role))
        {
            ret = ret.toInt();
        }
        return ret;
    }

    return QVariant();
}

QtIVIServiceObject *QtIVISimpleDiscoveryModelBase::serviceObjectAt(int index) const
{
    if (index >= 0 && index < m_serviceObjects.count()) {
        return m_serviceObjects.at(index);
    }

    return 0;
}

void QtIVISimpleDiscoveryModelBase::startAutoDiscovery()
{
    setAutoDiscovery(true);

    QtIVIServiceManager* serviceManager = QtIVIServiceManager::instance();

    connect(serviceManager, &QtIVIServiceManager::rowsInserted, this, &QtIVISimpleDiscoveryModelBase::onRowsInserted);
    connect(serviceManager, &QtIVIServiceManager::rowsRemoved, this, &QtIVISimpleDiscoveryModelBase::onRowsRemoved);
    connect(serviceManager, &QtIVIServiceManager::modelReset, this, &QtIVISimpleDiscoveryModelBase::onModelReset);

    onModelReset();
}

void QtIVISimpleDiscoveryModelBase::onModelReset()
{
    QtIVIServiceManager* serviceManager = QtIVIServiceManager::instance();
    QList<QtIVIServiceObject*> serviceObjects = serviceManager->findServiceByInterface(m_interface);

    if (m_serviceObjects.count() == 0 && serviceObjects.count() == 0)
        return;

    beginResetModel();
    m_serviceObjects = serviceObjects;

    // FIXME does not call the virtual methods updateServiceObjects, addServiceObjects, removeServiceObjects

    endResetModel();
}

void QtIVISimpleDiscoveryModelBase::onRowsInserted(const QModelIndex &index, int first, int last)
{
    QList<QtIVIServiceObject*> newServiceObjects;
    QtIVIServiceManager* serviceManager = QtIVIServiceManager::instance();
    QList<QtIVIServiceObject*> serviceObjects = serviceManager->findServiceByInterface(m_interface);

    foreach(QtIVIServiceObject *so, serviceObjects) {
        bool found = false;
        foreach(const QtIVIServiceObject *so2, m_serviceObjects)
            if (so2->id() == so->id())
                found = true;

        if (!found)
            newServiceObjects << so;
    }

    if (newServiceObjects.count())
        addServiceObjects(newServiceObjects);
}

void QtIVISimpleDiscoveryModelBase::onRowsRemoved(const QModelIndex &index, int first, int last)
{
    QList<QtIVIServiceObject*> oldServiceObjects;
    QtIVIServiceManager* serviceManager = QtIVIServiceManager::instance();
    QList<QtIVIServiceObject*> serviceObjects = serviceManager->findServiceByInterface(m_interface);

    foreach(QtIVIServiceObject *so, serviceObjects) {
        bool found = false;
        foreach(const QtIVIServiceObject *so2, m_serviceObjects)
            if (so2->id() == so->id())
                found = true;

        if (found)
            oldServiceObjects << so;
    }

    if (oldServiceObjects.count())
        removeServiceObjects(oldServiceObjects);
}

bool QtIVISimpleDiscoveryModelBase::autoDiscovery() const
{
    return m_autoDiscovery;
}

void QtIVISimpleDiscoveryModelBase::updateServiceObjects(QList<QtIVIServiceObject *> objects)
{
    Q_UNUSED(objects)
    //Do a logic here to call update for as many buckets as possible
}

void QtIVISimpleDiscoveryModelBase::addServiceObjects(QList<QtIVIServiceObject *> objects)
{
    beginInsertRows(QModelIndex(), m_serviceObjects.count(), m_serviceObjects.count());
    m_serviceObjects += objects;
    endInsertRows();
}

void QtIVISimpleDiscoveryModelBase::removeServiceObjects(QList<QtIVIServiceObject *> objects)
{
    //FIXME removing the wrong rows
    beginRemoveRows(QModelIndex(), m_serviceObjects.count(), m_serviceObjects.count());
    foreach(QtIVIServiceObject *obj, objects)
        m_serviceObjects.removeAll(obj);
    endRemoveRows();
}

void QtIVISimpleDiscoveryModelBase::classBegin()
{

}

void QtIVISimpleDiscoveryModelBase::componentComplete()
{
    if (m_autoDiscovery)
        startAutoDiscovery();
}

QHash<int, QByteArray> QtIVISimpleDiscoveryModelBase::roleNames() const
{
    return m_roles;
}

void QtIVISimpleDiscoveryModelBase::setRoleNames(QHash<int, QByteArray> roles, QSet<int> enumProperties)
{
    roles.insert(ServiceObject, "serviceObject");

    m_roles = roles;
    m_enumProperties = enumProperties;
}
