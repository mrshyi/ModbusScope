#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QObject>
#include <QMap>
#include "modbusresult.h"

#include "modbusconnection.h"
#include "readregisters.h"

/* Forward declaration */
class SettingsModel;
class GuiModel;

class ModbusMaster : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMaster(SettingsModel * pSettingsModel, quint8 connectionId);
    virtual ~ModbusMaster();

    void readRegisterList(QList<quint16> registerList);

signals:
    void modbusPollDone(QMap<quint16, ModbusResult> modbusResults, quint8 connectionId);
    void modbusAddToStats(quint32 successes, quint32 errors);
    void modbusLogError(QString msg);
    void modbusLogInfo(QString msg);
    void triggerNextRequest();

private slots:
    void handleConnectionOpened();
    void handlerConnectionError(QModbusDevice::Error error, QString msg);
    void handleRequestFinished();
    void handleRequestErrorOccurred(QModbusDevice::Error error);
    void handleTriggerNextRequest(void);

private:
    void finishRead();
    QString dumpToString(QMap<quint16, ModbusResult> map);
    QString dumpToString(QList<quint16> list);

    void logInfo(QString msg);
    void logError(QString msg);

    quint32 _success;
    quint32 _error;

    quint8 _connectionId;

    SettingsModel * _pSettingsModel;
    ModbusConnection * _pModbusConnection;
    ReadRegisters * _pReadRegisters;

};

#endif // MODBUSMASTER_H
