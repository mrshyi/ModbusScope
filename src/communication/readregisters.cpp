#include "readregisters.h"

ReadRegisters::ReadRegisters()
{

}

/*!
 * Load ReadRegisterCollection with register read list
 * \param registerList  Register read list
 * \param consecutiveMax Number of consecutive registers that is allowed to read at once
 */
void ReadRegisters::resetRead(QList<quint16> registerList, quint16 consecutiveMax)
{
    _resultMap.clear();

    if (registerList.size() == 0)
    {
        _readItemList.clear();
    }

    while(registerList.size() > 0)
    {
        if (
            (registerList.size() == 1)
            || (consecutiveMax == 1)
        )
        {
            // Only single reads allowed
            _readItemList.append(ModbusReadItem(registerList.first(), 1));

            registerList.removeFirst();
        }
        else
        {
            int currentIdx = 0;

            bool bSubsequent = false;
            do
            {

                // if next is current + 1, dan subsequent = true
                if (registerList.at(currentIdx) + 1 == registerList.at(currentIdx + 1))
                {
                    bSubsequent = true;
                    currentIdx++;
                }

                // Break loop when end of list
                if (currentIdx >= (registerList.size() - 1))
                {
                    break;
                }

                // Limit number of register in 1 read
                if (currentIdx >= consecutiveMax)
                {
                    break;
                }

            } while(bSubsequent == true);

            if (bSubsequent)
            {
                // Convert idx to count
                quint8 count = static_cast<quint8>(currentIdx) + 1;

                _readItemList.append(ModbusReadItem(registerList.first(), count));

                for (int idx = 0; idx < count; idx++)
                {
                    registerList.removeFirst();
                }

            }

        }
    }
}

/*!
 * Return whether there is stil a ModbusReadItem left
 * \retval true     Still ModbusReadItemLeft
 * \retval false    None left
 */
bool ReadRegisters::hasNext()
{
    return !_readItemList.isEmpty();
}

/*!
 * Get next ModbusReadItem
 * \return next ModbusReadItem
 */
ModbusReadItem ReadRegisters::next()
{
    return _readItemList.first();
}

/*!
 * Add success result for current ReadRegister cluster
 * \param startRegister     Start register address
 * \param registerDataList  List with result data
 */
void ReadRegisters::addSuccess(quint16 startRegister, QList<quint16> registerDataList)
{

    if (
        (next().address() == startRegister)
        && (next().count() == registerDataList.size())
    )
    {
        for (qint32 i = 0; i < registerDataList.size(); i++)
        {
            const quint16 registerAddr = startRegister + static_cast<quint16>(i);
            const ModbusResult result = ModbusResult(registerDataList[i], true);

            _resultMap.insert(registerAddr, result);
        }

        _readItemList.removeFirst();
    }
}

/*!
 * Add error result for current ReadRegister cluster
 * \param startRegister     Start register address
 * \param count             Number of failed registers reads
 */
void ReadRegisters::addError(quint16 startRegister, quint32 count)
{
    if (
        (next().address() == startRegister)
        && (next().count() == count)
    )
    {
        for (quint32 i = 0; i < count; i++)
        {
            const quint16 registerAddr = startRegister + static_cast<quint16>(i);
            const ModbusResult result = ModbusResult(0, false);

            _resultMap.insert(registerAddr, result);
        }

        _readItemList.removeFirst();
    }
}

/*!
 * Mark all remaining register as errors
 */
void ReadRegisters::addAllErrors()
{
    while(hasNext())
    {
        addError(next().address(), next().count());
    }
}

/*!
 * Split "next" ModbusReadItem into single reads.
 */
void ReadRegisters::splitNextToSingleReads()
{
    ModbusReadItem firstItem = _readItemList.first();

    _readItemList.removeFirst();

    for(int idx = 0; idx < firstItem.count(); idx++)
    {
        _readItemList.prepend(ModbusReadItem(static_cast<quint16>(firstItem.address()) + static_cast<quint16>(idx), 1));
    }
}

/*!
 * Return result map
 * \return Result map
 */
QMap<quint16, ModbusResult> ReadRegisters::resultMap()
{
    return _resultMap;
}
