#include "graphdatamodel.h"
#include "util.h"

GraphDataModel::GraphDataModel(QObject *parent) : QObject(parent)
{
    _graphData.clear();
}

qint32 GraphDataModel::size()
{
    return _graphData.size();
}

qint32 GraphDataModel::activeCount()
{
    return _activeGraphList.size();
}

bool GraphDataModel::isVisible(quint32 index) const
{
    return _graphData[index]->isVisible();
}

QString GraphDataModel::label(quint32 index) const
{
    return _graphData[index]->label();
}

QColor GraphDataModel::color(quint32 index) const
{
    return _graphData[index]->color();
}

bool GraphDataModel::isActive(quint32 index) const
{
    return _graphData[index]->isActive();
}

bool GraphDataModel::isUnsigned(quint32 index) const
{
    return _graphData[index]->isUnsigned();
}

double GraphDataModel::multiplyFactor(quint32 index) const
{
    return _graphData[index]->multiplyFactor();
}

double GraphDataModel::divideFactor(quint32 index) const
{
    return _graphData[index]->divideFactor();
}

quint16 GraphDataModel::registerAddress(quint32 index) const
{
    return _graphData[index]->registerAddress();
}

quint16 GraphDataModel::bitmask(quint32 index) const
{
    return _graphData[index]->bitmask();
}

qint32 GraphDataModel::shift(quint32 index) const
{
    return _graphData[index]->shift();
}


void GraphDataModel::setVisible(quint32 index, bool bVisible)
{
    if (_graphData[index]->isVisible() != bVisible)
    {
         _graphData[index]->setVisible(bVisible);
         emit visibilityChanged(index);
    }
}

void GraphDataModel::setLabel(quint32 index, const QString &label)
{
    if (_graphData[index]->label() != label)
    {
         _graphData[index]->setLabel(label);
         emit labelChanged(index);
    }
}

void GraphDataModel::setColor(quint32 index, const QColor &color)
{
    if (_graphData[index]->color() != color)
    {
         _graphData[index]->setColor(color);
         emit colorChanged(index);
    }
}

void GraphDataModel::setActive(quint32 index, bool bActive)
{
    if (_graphData[index]->isActive() != bActive)
    {
        _graphData[index]->setActive(bActive);

        // When activated, keep activeList in sync
        if (bActive)
        {
            if (!_activeGraphList.contains(index))
            {
                _activeGraphList.append(index);
            }
        }
        else
        {
            _activeGraphList.removeOne(index);
        }

        emit activeChanged(index);
    }
}

void GraphDataModel::setUnsigned(quint32 index, bool bUnsigned)
{
    if (_graphData[index]->isUnsigned() != bUnsigned)
    {
         _graphData[index]->setUnsigned(bUnsigned);
         emit unsignedChanged(index);
    }
}

void GraphDataModel::setMultiplyFactor(quint32 index, double multiplyFactor)
{
    if (_graphData[index]->multiplyFactor() != multiplyFactor)
    {
         _graphData[index]->setMultiplyFactor(multiplyFactor);
         emit multiplyFactorChanged(index);
    }
}

void GraphDataModel::setDivideFactor(quint32 index, double divideFactor)
{
    if (_graphData[index]->divideFactor() != divideFactor)
    {
         _graphData[index]->setDivideFactor(divideFactor);
         emit divideFactorChanged(index);
    }
}

void GraphDataModel::setRegisterAddress(quint32 index, const quint16 &registerAddress)
{
    if (_graphData[index]->registerAddress() != registerAddress)
    {
         _graphData[index]->setRegisterAddress(registerAddress);
         emit registerAddressChanged(index);
    }
}

void GraphDataModel::setBitmask(quint32 index, const quint16 &bitmask)
{
    if (_graphData[index]->bitmask() != bitmask)
    {
         _graphData[index]->setBitmask(bitmask);
         emit bitmaskChanged(index);
    }
}

void GraphDataModel::setShift(quint32 index, const qint32 &shift)
{
    if (_graphData[index]->shift() != shift)
    {
         _graphData[index]->setShift(shift);
         emit shiftChanged(index);
    }
}

void GraphDataModel::add(GraphData & rowData)
{
    /* Select color */
    if (!rowData.color().isValid())
    {
        quint32 colorIndex = _graphData.size() % Util::cColorlist.size();
        rowData.setColor(Util::cColorlist[colorIndex]);
    }

    _graphData.append(rowData);

    if (rowData.isActive())
    {
        // Always make sure active event is send when active is true
        setActive(_graphData.size() - 1, false);
        setActive(_graphData.size() - 1, true);
    }
}

void GraphDataModel::add(QList<GraphData> graphDataList)
{
    foreach(GraphData graphItem, graphDataList)
    {
        add(graphItem);
    }
}

void GraphDataModel::add()
{
    GraphData data;

    data.setRegisterAdress(nextFreeAddress());
    data.setLabel(QString("Register %1 (bitmask: 0x%2)").arg(_registerAddress));

    add(data);
}

void GraphDataModel::add(QList<QString> labelList, QList<double> timeData, QList<QList<double> > data)
{
    foreach(QString label, labelList)
    {
        add();
        setLabel(_graphData.size() - 1, label);
    }

    emit graphsAddData(timeData, data);
}

void GraphDataModel::removeRegister(qint32 idx)
{   
    if (idx < _graphData.size())
    {
        if (_graphData[idx]->bActive)
        {
            _activeGraphList.removeOne(idx);
        }
        _graphData.removeAt(idx);
    }

    emit removed(idx);
}

void GraphDataModel::clear()
{
    _graphData.clear();
    _activeGraphList.clear();

    emit cleared();
}

// Get sorted list of active register addresses
void GraphDataModel::activeGraphAddresList(QList<quint16> * pRegisterList)
{
    // Clear list
    pRegisterList->clear();

    foreach(quint32 idx, _activeGraphList)
    {
        pRegisterList->append(_graphData[idx]->registerAddress());
    }

    // sort qList
    qSort(pRegisterList);
}

bool GraphDataModel::areExclusive(quint16 * pRegister, quint16 * pBitmask)
{
    for (qint32 idx = 0; idx < (_graphData.size() - 1); idx++) // Don't need to check last entry
    {
        for (int checkIdx = (idx + 1); checkIdx < _graphData.size(); checkIdx++)
        {
            if (
                (_graphData[idx]->registerAddress() == _graphData[checkIdx]->registerAddress())
                && (_graphData[idx]->bitmask() == _graphData[checkIdx]->bitmask())
            )
            {
                *pRegister = _graphData[idx]->registerAddress();
                *pBitmask = _graphData[idx]->bitmask();
                return false;
            }
        }
    }

    return true;
}

qint32 GraphDataModel::convertToActiveGraphIndex(quint32 idx)
{
    qint32 activeIdx;
    qint16 result = -1;
    for (activeIdx = 0; activeIdx < _activeGraphList.size(); activeIdx++)
    {
        if (_activeGraphList[activeIdx] == idx)
        {
            result = activeIdx;
            break;
        }
    }
    return result;
}

qint32 GraphDataModel::convertToGraphIndex(quint32 idx)
{
    qint32 activeIdx;
    qint16 result = -1;
    for (activeIdx = 0; activeIdx < _activeGraphList.size(); activeIdx++)
    {
        if (_activeGraphList[activeIdx] == idx)
        {
            result = activeIdx;
            break;
        }
    }
    return result;
}

quint16 GraphDataModel::nextFreeAddress()
{
    // Create local copy of active register list

    QList<quint32> regList = QList<quint32>(_activeGraphList);
    quint16 nextAddress;   

    // sort qList
    qSort(regList);

    if (regList.size() > 0)
    {
        nextAddress = regList.last() + 1;
    }
    else
    {
        nextAddress = 40001;
    }

    return nextAddress;
}