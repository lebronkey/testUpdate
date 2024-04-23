#pragma execution_character_set("utf-8")

#include "QmlUpdater.h"
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>

QmlUpdater::QmlUpdater(QObject *parent) : QObject(parent) {

  m_updater = QSimpleUpdater::getInstance();
  m_downloader = new Downloader();

  connect(m_updater, SIGNAL(appcastDownloaded(QString, QByteArray)), this,
          SLOT(displayAppcast(QString, QByteArray)));

  connect(m_downloader, SIGNAL(downloadFinished(QString, QString)), this,
          SIGNAL(downloadFinished(QString, QString)));

  DEFS_URL = DEFS_URL +
             "?t=" + QString::number(QDateTime::currentDateTime().toTime_t());
  checkForUpdates();
}

QmlUpdater::~QmlUpdater() {}

void QmlUpdater::checkForUpdates() {
  /* Apply the settings */
  //  m_updater->setModuleVersion(DEFS_URL, "0.1");
  m_updater->setNotifyOnFinish(DEFS_URL, true);
  m_updater->setNotifyOnUpdate(DEFS_URL, true);
  m_updater->setUseCustomAppcast(DEFS_URL, true);
  m_updater->setDownloaderEnabled(DEFS_URL, true);
  m_updater->setMandatoryUpdate(DEFS_URL, true);

  /* Check for updates */
  m_updater->checkForUpdates(DEFS_URL);
}

void QmlUpdater::updateChangelog(const QString &url) {
  QString log = m_updater->getChangelog(url);
  qDebug() << "updateChangelog " << log;
  if (url == DEFS_URL) {
    //    setChangeLog(log);
  }
}

void QmlUpdater::displayAppcast(const QString &url, const QByteArray &reply) {
  if (url != DEFS_URL) {
    return;
  }

  /* Try to create a JSON document from downloaded data */
  QJsonDocument document = QJsonDocument::fromJson(reply);

  /* JSON is invalid */
  if (document.isNull()) {
    qDebug() << " update json is error!!";
    return;
  }

  /* Get the platform information */
  QJsonObject updates = document.object().value("updates").toObject();
  QJsonObject platform =
      updates.value(m_updater->getPlatformKey(url)).toObject();

  /* Get update information */
  QString m_openUrl = platform.value("open-url").toString();
  QString m_changelog = platform.value("changelog").toString();
  m_downloadUrl = platform.value("download-url").toString();
  QString m_latestVersion = platform.value("latest-version").toString();
  if (platform.contains("mandatory-update")) {
    bool m_mandatoryUpdate = platform.value("mandatory-update").toBool();
  }

  if (compare(m_latestVersion, m_updater->getModuleVersion(url))) {
    emit notifyUpdate(m_latestVersion, m_updater->getModuleName(url),
                      m_changelog);
  }
}

void QmlUpdater::finish(QString filepath) {
  QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
}

void QmlUpdater::startDownload() {
  QString path = QCoreApplication::applicationDirPath() + "/../"; //下载文件保存地址
  QString filename = m_downloadUrl.split("/").last();  //文件名

  qDebug() << "download path" << path;
  qDebug() << "download url" << m_downloadUrl << " file name " << filename;

  m_downloader->setUseCustomInstallProcedures(true);
  m_downloader->setUrlId(DEFS_URL);
  m_downloader->setDownloadDir(path);
  m_downloader->setFileName(filename);
  m_downloader->setMandatoryUpdate(true);
  m_downloader->startDownload(QUrl(m_downloadUrl));
}

bool QmlUpdater::compare(const QString &x, const QString &y) {
  QStringList versionsX = x.split(".");
  QStringList versionsY = y.split(".");

  int count = qMin(versionsX.count(), versionsY.count());

  for (int i = 0; i < count; ++i) {
    int a = QString(versionsX.at(i)).toInt();
    int b = QString(versionsY.at(i)).toInt();

    if (a > b)
      return true;

    else if (b > a)
      return false;
  }

  return versionsY.count() < versionsX.count();
}
