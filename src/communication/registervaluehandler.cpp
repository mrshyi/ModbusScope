#include "registervaluehandler.h"
#include "settingsmodel.h"

RegisterValueHandler::RegisterValueHandler(SettingsModel *pSettingsModel) :
    _pSettingsModel(pSettingsModel)
{
}

void RegisterValueHandler::startRead()
{
    _resultList.clear();

    for(quint16 listIdx = 0; listIdx < _registerList.size(); listIdx++)
    {
        _resultList.append(Result());
    }
}

void RegisterValueHandler::finishRead()
{
    // propagate processed data
    emit registerDataReady(_resultList);
}

void RegisterValueHandler::processPartialResult(QMap<quint16, Result> partialResultMap, quint8 connectionId)
{
    QMapIterator<quint16, Result> i(partialResultMap);
    while (i.hasNext())
    {
        i.next();

        for(quint16 listIdx = 0; listIdx < _registerList.size(); listIdx++)
        {
            const ModbusRegister mbReg = _registerList[listIdx];

            if (mbReg.connectionId() == connectionId)
            {
                if (mbReg.address() == i.key())
                {
                    bool bSuccess = i.value().isSuccess();
                    uint16_t value = static_cast<uint16_t>(i.value().value());

                    qint64 processedResult = 0;
                    if (bSuccess)
                    {
                        if (mbReg.is32Bit())
                        {
                            Result nextResult = i.peekNext().value();

                            if (nextResult.isSuccess())
                            {
                                uint32_t combinedValue = convertEndianness(_pSettingsModel->int32LittleEndian(connectionId), value, nextResult.value());

                                if (mbReg.isUnsigned())
                                {
                                    processedResult = static_cast<qint64>(static_cast<quint32>(combinedValue));
                                }
                                else
                                {
                                    processedResult = static_cast<qint64>(static_cast<qint32>(combinedValue));
                                }
                            }
                            else
                            {
                                bSuccess = false;
                                processedResult = 0;
                            }
                        }
                        else
                        {
                            if (mbReg.isUnsigned())
                            {
                                processedResult = static_cast<qint64>(static_cast<quint16>(value));
                            }
                            else
                            {
                                processedResult = static_cast<qint64>(static_cast<qint16>(value));
                            }
                        }
                    }
                    else
                    {
                        processedResult = 0;
                    }

                    _resultList[listIdx] = Result(static_cast<double>(processedResult), bSuccess);
                }
            }
        }
    }
}

// Get sorted list of active (unique) register addresses for a specific connection id
void RegisterValueHandler::registerAddresList(QList<quint16>& registerList, quint8 connectionId)
{
    QList<quint16> connRegisterList;

    foreach(ModbusRegister mbReg, _registerList)
    {
        if (mbReg.connectionId() == connectionId)
        {
            if (!connRegisterList.contains(mbReg.address()))
            {
                connRegisterList.append(mbReg.address());
            }

            /* When reading 32 bit value, also read next address */
            if (mbReg.is32Bit())
            {
                const uint16_t reg = mbReg.address() + 1;
                if (!connRegisterList.contains(reg))
                {
                    connRegisterList.append(reg);
                }
            }
        }
    }

    // sort qList
    std::sort(connRegisterList.begin(), connRegisterList.end(), std::less<int>());

    registerList = connRegisterList;
}

void RegisterValueHandler::setRegisters(QList<ModbusRegister>& registerList)
{
    _registerList = registerList;
}

uint32_t RegisterValueHandler::convertEndianness(bool bLittleEndian, uint16_t value, uint16_t nextValue)
{
    uint32_t combinedValue;
    if (bLittleEndian)
    {
        combinedValue = (static_cast<uint32_t>(nextValue) << 16) | value;
    }
    else
    {
        combinedValue = (static_cast<uint32_t>(value) << 16) | nextValue;
    }

    return combinedValue;
}
