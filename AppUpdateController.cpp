#include "AppUpdateController.h"
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>

AppUpdateController::AppUpdateController(QObject *parent) : QObject(parent),
    m_notifyUpdate     (true),
    m_notifyFinish     (true),
    m_downloadEnabled  (true),
    m_useCustomInstall (false),
    m_useCustomAppcast (false),
    m_mandatoryUpdate  (false)
{

    m_updater = QSimpleUpdater::getInstance();

    connect(m_updater, &QSimpleUpdater::checkingFinished,  this, &AppUpdateController::updateChangelog);
}

AppUpdateController::~AppUpdateController() {


}

void AppUpdateController::checkForUpdates() {
    /* Apply the settings */
    //  m_updater->setModuleVersion(DEFS_URL, "0.1");
    m_updater->setNotifyOnFinish(DEFS_URL, notifyFinish());
    m_updater->setNotifyOnUpdate(DEFS_URL, notifyUpdate());
    m_updater->setUseCustomAppcast(DEFS_URL, useCustomAppcast());
    m_updater->setDownloaderEnabled(DEFS_URL, downloadEnabled());
    m_updater->setMandatoryUpdate(DEFS_URL, mandatoryUpdate());

    /* Check for updates */
    m_updater->checkForUpdates(DEFS_URL);

    qInfo()<<DEFS_URL;
}

void AppUpdateController::setNotifyFinish     (bool _notifyFinish)
{
    m_notifyFinish = _notifyFinish;
    emit notifyFinishChanged();
}
void AppUpdateController::setDownloadEnabled  (bool _downloadEnable)
{
    m_downloadEnabled = _downloadEnable;
    emit downloadEnabledChanged();
}
void AppUpdateController::setNotifyUpdate     (bool _notifyUpdate)
{
    m_notifyUpdate = _notifyUpdate;
    emit notifyUpdateChanged();

}
void AppUpdateController::setUseCustomInstall (bool _customInstall)
{
    m_useCustomInstall = _customInstall;
    emit useCustomInstallChanged();
}
void AppUpdateController::setUseCustomAppcast (bool _customAppcast)
{
    m_useCustomAppcast = _customAppcast;
    emit useCustomAppcastChanged();
}
void AppUpdateController::setMandatoryUpdate  (bool _mandatoryUpdate)
{
    m_mandatoryUpdate = _mandatoryUpdate;
    emit mandatoryUpdateChanged();
}


void AppUpdateController::updateChangelog(const QString &url) {

    QString log = m_updater->getChangelog(url);

    qDebug() << "updateChangelog " << log;
    m_changeLog = log;
    emit changeLogChanged(log);

}
