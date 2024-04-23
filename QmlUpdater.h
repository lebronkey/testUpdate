#ifndef QMLUPDATER_H
#define QMLUPDATER_H

#include <QObject>
#include <QSimpleUpdater.h>
#include <QSimpleUpdater/src/Downloader.h>

namespace Ui {
class QmlUpdater;
}

class QSimpleUpdater;

class QmlUpdater : public QObject {
  Q_OBJECT
public:
  explicit QmlUpdater(QObject *parent = nullptr);
  ~QmlUpdater();
  Q_INVOKABLE void checkForUpdates();
  Q_INVOKABLE void startDownload();

public slots:
  void finish(QString filepath);

private slots:
  void updateChangelog(const QString &url);
  void displayAppcast(const QString &url, const QByteArray &reply);

private:
  bool compare(const QString &x, const QString &y);

signals:
  void notifyUpdate(QString ver, QString name, QString log);
  void downloadFinished(const QString &url, const QString &filepath);

private:
  QString DEFS_URL = "http://wiki.developlink.cloud/vcom/updates.json";  //更新文件json地址
  QString m_downloadUrl;
  QSimpleUpdater *m_updater;
  Downloader *m_downloader;
};


#endif // QMLUPDATER_H
