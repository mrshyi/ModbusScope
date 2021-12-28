
#include <QtTest/QtTest>
#include <QMap>

#include "modbusmaster.h"
#include "testslavedata.h"
#include "testslavemodbus.h"

#include "testmodbuspoll.h"

Q_DECLARE_METATYPE(Result);

void TestModbusPoll::init()
{
    qRegisterMetaType<Result>("Result");
    qRegisterMetaType<QList<Result> >("QList<Result>");

    _pSettingsModel = new SettingsModel;

    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_1, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_1, 5020);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_1, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_1, 1);

    _pSettingsModel->setConnectionState(SettingsModel::CONNECTION_ID_2, true);
    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_2, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_2, 5021);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_2, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_2, 2);

    _pSettingsModel->setConnectionState(SettingsModel::CONNECTION_ID_3, true);
    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_3, "127.0.0.1");
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_3, 5022);
    _pSettingsModel->setTimeout(SettingsModel::CONNECTION_ID_3, 500);
    _pSettingsModel->setSlaveId(SettingsModel::CONNECTION_ID_3, 3);

    _pSettingsModel->setPollTime(100);

    for (quint8 idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _serverConnectionDataList.append(QUrl());
        _serverConnectionDataList.last().setPort(_pSettingsModel->port(idx));
        _serverConnectionDataList.last().setHost(_pSettingsModel->ipAddress(idx));

        _testSlaveDataList.append(new TestSlaveData());
        _testSlaveModbusList.append(new TestSlaveModbus(_testSlaveDataList.last()));

        QVERIFY(_testSlaveModbusList.last()->connect(_serverConnectionDataList.last(), _pSettingsModel->slaveId(idx)));
    }
}

void TestModbusPoll::cleanup()
{
    delete _pSettingsModel;

    for (int idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    qDeleteAll(_testSlaveDataList);
    qDeleteAll(_testSlaveModbusList);

    _serverConnectionDataList.clear();
    _testSlaveDataList.clear();
    _testSlaveModbusList.clear();
}

void TestModbusPoll::singleSlaveSuccess()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(0, 5);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(1, 65000);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                   << ModbusRegister(40002, SettingsModel::CONNECTION_ID_1, false, true);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = QList<Result>() << Result(5, true)
                                            << Result(65000, true);

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::singleSlaveFail()
{
    for (quint8 idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                   << ModbusRegister(40002, SettingsModel::CONNECTION_ID_1, false, true);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(static_cast<int>(_pSettingsModel->timeout(SettingsModel::CONNECTION_ID_1)) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();

    auto expResults = QList<Result>() << Result(0, false)
                                            << Result(0, false);

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSuccess()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(0, 5020);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterValue(0, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                   << ModbusRegister(40001, SettingsModel::CONNECTION_ID_2, false, true);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = QList<Result>() << Result(5020, true)
                                            << Result(5021, true);

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSuccess_2()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(0, 5020);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterValue(1, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                   << ModbusRegister(40002, SettingsModel::CONNECTION_ID_2, false, true);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = QList<Result>() << Result(5020, true)
                                            << Result(5021, true);

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSuccess_3()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(0, 5020);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(1, 5021);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterState(1, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterValue(1, 5022);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                   << ModbusRegister(40002, SettingsModel::CONNECTION_ID_2, false, true)
                                                   << ModbusRegister(40002, SettingsModel::CONNECTION_ID_1, false, true);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = QList<Result>() << Result(5020, true)
                                            << Result(5022, true)
                                            << Result(5021, true);

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveSingleFail()
{
    _testSlaveModbusList[SettingsModel::CONNECTION_ID_1]->disconnectDevice();

    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterValue(0, 5021);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                   << ModbusRegister(40001, SettingsModel::CONNECTION_ID_2, false, true);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(static_cast<int>(_pSettingsModel->timeout(SettingsModel::CONNECTION_ID_1)) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = QList<Result>() << Result(0, false)
                                            << Result(5021, true);

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveAllFail()
{
    for (quint8 idx = 0; idx < SettingsModel::CONNECTION_ID_CNT; idx++)
    {
        _testSlaveModbusList[idx]->disconnectDevice();
    }

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                   << ModbusRegister(40001, SettingsModel::CONNECTION_ID_2, false, true);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(static_cast<int>(_pSettingsModel->timeout(SettingsModel::CONNECTION_ID_1)) + 100));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();
    auto expResults = QList<Result>() << Result(0, false)
                                            << Result(0, false);

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::multiSlaveDisabledConnection()
{
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_1]->setRegisterValue(0, 5020);

    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterState(0, true);
    _testSlaveDataList[SettingsModel::CONNECTION_ID_2]->setRegisterValue(0, 5021);

    /* Disable connection */
    _pSettingsModel->setConnectionState(SettingsModel::CONNECTION_ID_2, false);

    ModbusPoll modbusPoll(_pSettingsModel);
    QSignalSpy spyDataReady(&modbusPoll, &ModbusPoll::registerDataReady);

    auto modbusRegisters = QList<ModbusRegister>() << ModbusRegister(40001, SettingsModel::CONNECTION_ID_1, false, true)
                                                   << ModbusRegister(40001, SettingsModel::CONNECTION_ID_2, false, true);

    /*-- Start communication --*/
    modbusPoll.startCommunication(modbusRegisters);

    QVERIFY(spyDataReady.wait(50));
    QCOMPARE(spyDataReady.count(), 1);

    QList<QVariant> arguments = spyDataReady.takeFirst();

    /* Disabled connections return error and zero */
    auto expResults = QList<Result>() << Result(5020, true)
                                            << Result(0, false);

    /* Verify arguments of signal */
    verifyReceivedDataSignal(arguments, expResults);
}

void TestModbusPoll::verifyReceivedDataSignal(QList<QVariant> arguments, QList<Result> expResultList)
{
    QVERIFY(arguments.count() > 0);

    QVariant varResultList = arguments.first();
    QVERIFY((varResultList.canConvert<QList<Result> >()));
    QList<Result> result = varResultList.value<QList<Result> >();

    QCOMPARE(result, expResultList);
}

QTEST_GUILESS_MAIN(TestModbusPoll)