#ifndef APPUPDATECONTROLLER_H
#define APPUPDATECONTROLLER_H

#include <QObject>
#include <QSimpleUpdater.h>
#include <QSimpleUpdater/src/Downloader.h>

namespace Ui {
class AppUpdateController;
}

class QSimpleUpdater;

class AppUpdateController : public QObject {

    Q_OBJECT

    Q_PROPERTY(bool notifyFinish     READ notifyFinish      WRITE setNotifyFinish          NOTIFY notifyFinishChanged     FINAL)
    Q_PROPERTY(bool notifyUpdate     READ notifyUpdate      WRITE setNotifyUpdate          NOTIFY notifyUpdateChanged     FINAL)
    Q_PROPERTY(bool downloadEnabled  READ downloadEnabled   WRITE setDownloadEnabled       NOTIFY downloadEnabledChanged  FINAL)
    Q_PROPERTY(bool useCustomInstall READ useCustomInstall  WRITE setUseCustomInstall      NOTIFY useCustomInstallChanged FINAL)
    Q_PROPERTY(bool useCustomAppcast READ useCustomAppcast  WRITE setUseCustomAppcast      NOTIFY useCustomAppcastChanged FINAL)
    Q_PROPERTY(bool mandatoryUpdate  READ mandatoryUpdate   WRITE setMandatoryUpdate       NOTIFY mandatoryUpdateChanged  FINAL)

    Q_PROPERTY(QString  changeLog READ changeLog  NOTIFY changeLogChanged )
public:
    explicit AppUpdateController(QObject *parent = nullptr);
    ~AppUpdateController();

    Q_INVOKABLE void checkForUpdates();;
    Q_INVOKABLE QString getAppVersion () {return qApp->applicationVersion();}

    bool notifyFinish       ()           { return m_notifyFinish;     }
    bool notifyUpdate       ()           { return m_notifyUpdate;     }
    bool downloadEnabled    ()           { return m_downloadEnabled;  }
    bool useCustomInstall   ()           { return m_useCustomInstall; }
    bool useCustomAppcast   ()           { return m_useCustomAppcast; }
    bool mandatoryUpdate    ()           { return m_mandatoryUpdate;  }
    QString changeLog       ()           { return m_changeLog;}

    void setNotifyFinish     (bool _notifyFinish);
    void setDownloadEnabled  (bool _downloadEnable);
    void setNotifyUpdate     (bool _notifyUpdate);
    void setUseCustomInstall (bool _customInstall);
    void setUseCustomAppcast (bool _customAppcast);
    void setMandatoryUpdate  (bool _mandatoryUpdate);


signals:
    void  notifyFinishChanged     ();
    void  notifyUpdateChanged     ();
    void  downloadEnabledChanged  ();
    void  useCustomInstallChanged ();
    void  useCustomAppcastChanged ();
    void  mandatoryUpdateChanged  ();
    void changeLogChanged(QString changedLog);

private slots:

    void updateChangelog(const QString &url);

private:
    QString DEFS_URL = "https://raw.githubusercontent.com/lebronkey/testUpdate/main/definitions/updates3.json";
    //更新文件json地址
    QString m_downloadUrl;
    QSimpleUpdater *m_updater;
    bool m_notifyUpdate;
    bool m_notifyFinish;
    bool m_downloadEnabled;
    bool m_useCustomInstall;
    bool m_useCustomAppcast;
    bool m_mandatoryUpdate;
    QString m_changeLog;
};


#endif // APPUPDATECONTROLLER_H
