
#include "gmock/gmock.h"
#include <gmock/gmock-matchers.h>

#include <QtTest/QtTest>

#include "tst_updatenotify.h"

#include "../mocks/gmockutils.h"

#include "../mocks/mockversiondownloader.h"

#include "updatenotify.h"

using ::testing::Return;
using namespace testing;

// Configured to fixed version 2.1.1
const QString TestUpdateNotify::_cVersion = QString("2.1.1");

const QString TestUpdateNotify::_cVersionLowerRevision = QString("2.1.0");
const QString TestUpdateNotify::_cVersionLowerMinor = QString("2.0.0");
const QString TestUpdateNotify::_cVersionLowerMajor = QString("1.1.0");

const QString TestUpdateNotify::_cVersionHigherRevision = QString("2.1.2");
const QString TestUpdateNotify::_cVersionHigherMinor = QString("2.2.0");
const QString TestUpdateNotify::_cVersionHigherMajor = QString("3.1.0");

void TestUpdateNotify::init()
{
    // IMPORTANT: This must be called before any mock object constructors
    GMockUtils::InitGoogleMock();

    _invalidPublishData = "";
    _tooRecentPublishData = QDate().currentDate().addDays(-13).toString(Qt::ISODate);
    _notRecentPublishData = QDate().currentDate().addDays(-15).toString(Qt::ISODate);

    qRegisterMetaType<UpdateNotify::UpdateState>("UpdateNotify::UpdateState");

    _pVersionDownloader = new MockVersionDownloader();
}

void TestUpdateNotify::cleanup()
{
    delete _pVersionDownloader;
}

void TestUpdateNotify::triggerDownloader()
{
    UpdateNotify updateNotify(_pVersionDownloader, _cVersion);

    EXPECT_CALL(*_pVersionDownloader, performCheck())
        .Times(1)
        ;

    updateNotify.checkForUpdate();
}

void TestUpdateNotify::versionLowerRevision()
{
    checkServerCheck(_cVersionLowerRevision, UpdateNotify::VERSION_LATEST);
}

void TestUpdateNotify::versionLowerMinor()
{
    checkServerCheck(_cVersionLowerMinor, UpdateNotify::VERSION_LATEST);
}

void TestUpdateNotify::versionLowerMajor()
{
    checkServerCheck(_cVersionLowerMajor, UpdateNotify::VERSION_LATEST);
}

void TestUpdateNotify::versionEqual()
{
    configureServerData(QString("v1.2.3"), QString("http://google.be"), _notRecentPublishData);

    UpdateNotify updateNotify(_pVersionDownloader, _cVersion);

    QSignalSpy spyUpdateResult(&updateNotify, &UpdateNotify::updateCheckResult);

    emit _pVersionDownloader->versionDownloaded();

    QCOMPARE(spyUpdateResult.count(), 1);

    QList<QVariant> arguments = spyUpdateResult.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.first().toUInt(), UpdateNotify::VERSION_LATEST);

    QCOMPARE(updateNotify.version(), QString("1.2.3"));
    QCOMPARE(updateNotify.link(), QString("http://google.be"));
    QCOMPARE(updateNotify.versionState(), UpdateNotify::VERSION_LATEST);
}

void TestUpdateNotify::versionHigherRevision()
{
    checkServerCheck(_cVersionHigherRevision, UpdateNotify::VERSION_UPDATE_AVAILABLE);
}

void TestUpdateNotify::versionHigherMinor()
{
    checkServerCheck(_cVersionHigherMinor, UpdateNotify::VERSION_UPDATE_AVAILABLE);
}

void TestUpdateNotify::versionHigherMajor()
{
    checkServerCheck(_cVersionHigherMajor, UpdateNotify::VERSION_UPDATE_AVAILABLE);
}

void TestUpdateNotify::incorrectVersion()
{
    configureServerData(QString("v1.2"), QString("http://google.be"), _notRecentPublishData);

    UpdateNotify updateNotify(_pVersionDownloader, _cVersion);

    QSignalSpy spyUpdateResult(&updateNotify, &UpdateNotify::updateCheckResult);

    emit _pVersionDownloader->versionDownloaded();

    QCOMPARE(spyUpdateResult.count(), 0);
}

void TestUpdateNotify::incorrectUrl()
{
    configureServerData(QString("v1.2.3"), QString(" "), _notRecentPublishData);

    UpdateNotify updateNotify(_pVersionDownloader, _cVersion);

    QSignalSpy spyUpdateResult(&updateNotify, &UpdateNotify::updateCheckResult);

    emit _pVersionDownloader->versionDownloaded();

    QCOMPARE(spyUpdateResult.count(), 0);
}

void TestUpdateNotify::configureServerData(QString version, QString url, QString publishDate)
{
    EXPECT_CALL(*_pVersionDownloader, version())
        .WillRepeatedly(Return(version))
        ;

    EXPECT_CALL(*_pVersionDownloader, url())
        .WillRepeatedly(Return(url))
        ;

    EXPECT_CALL(*_pVersionDownloader, publishDate())
        .WillRepeatedly(Return(publishDate))
        ;
}

void TestUpdateNotify::checkServerCheck(QString version, UpdateNotify::UpdateState updateState)
{
    configureServerData(version, QString("http://google.be"), _notRecentPublishData);

    UpdateNotify updateNotify(_pVersionDownloader, _cVersion);

    QSignalSpy spyUpdateResult(&updateNotify, &UpdateNotify::updateCheckResult);

    emit _pVersionDownloader->versionDownloaded();

    QCOMPARE(spyUpdateResult.count(), 1);

    QList<QVariant> arguments = spyUpdateResult.takeFirst();
    QCOMPARE(arguments.count(), 1);
    QCOMPARE(arguments.first().toUInt(), updateState);

    QCOMPARE(updateNotify.version(), version);
    QCOMPARE(updateNotify.versionState(), updateState);
}

QTEST_GUILESS_MAIN(TestUpdateNotify)
