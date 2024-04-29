/*
 * Copyright (c) 2014-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
#include <QDesktopServices>
#include <QTextEdit>

#include "Updater.h"
#include "Downloader.h"

Updater::Updater()
{
    m_url = "";
    m_openUrl = "";
    m_changelog = "";
    m_downloadUrl = "";
    m_latestVersion = "";
    m_customAppcast = false;
    m_notifyOnUpdate = true;
    m_notifyOnFinish = false;
    m_updateAvailable = false;
    m_downloaderEnabled = true;
    /*
     * qApp 是一个指向全局的 QApplication 对象的指针，它提供了对应用程序的全局信息和状态的访问
     * QApplication 是 Qt 框架中用于管理应用程序全局状态的类。
     *
     * 需要注意的是，如果应用程序使用了 QApplication 类（通常情况下是 GUI 应用程序），开发人员可以使用 qApp 来访问应用程序的全局对象
     * 而如果是非 GUI 应用程序，可以使用 QCoreApplication
     *
     * 关于子窗口，如果子窗口是通过 QMainWindow 或 QWidget 派生的，那么它们也可以通过 qApp 或 QCoreApplication 访问应用程序的版本号
     */
    m_moduleName = qApp->applicationName();
    m_moduleVersion = qApp->applicationVersion();
    m_mandatoryUpdate = false;

    m_downloader = new Downloader();
    m_manager = new QNetworkAccessManager();

#if defined Q_OS_WIN
    m_platform = "windows";
#elif defined Q_OS_MAC
    m_platform = "osx";
#elif defined Q_OS_LINUX
    m_platform = "linux";
#elif defined Q_OS_ANDROID
    m_platform = "android";
#elif defined Q_OS_IOS
    m_platform = "ios";
#endif

    setUserAgentString(QString("%1/%2 (Qt; QSimpleUpdater)").arg(qApp->applicationName(), qApp->applicationVersion()));

    connect(m_downloader, SIGNAL(downloadFinished(QString, QString)), this, SIGNAL(downloadFinished(QString, QString)));
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(onReply(QNetworkReply *)));
}

Updater::~Updater()
{
    delete m_downloader;
}

/**
 * Returns the URL of the update definitions file
 * 返回更新定义文件的URL(.json配置文件)
 */
QString Updater::url() const
{
    return m_url;
}

/**
 * Returns the URL that the update definitions file wants us to open in
 * a web browser.
 * 返回检查更新后在Web浏览器中打开的URL
 *
 * \warning You should call \c checkForUpdates() before using this functio
 */
QString Updater::openUrl() const
{
    return m_openUrl;
}

/**
 * Returns the changelog defined by the update definitions file.
 * 返回更新定义文件中定义的变更日志
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::changelog() const
{
    return m_changelog;
}

/**
 * Returns the name of the module (if defined)
 * 返回模块的名称
 */
QString Updater::moduleName() const
{
    return m_moduleName;
}

/**
 * Returns the platform key (be it system-set or user-set).
 * 返回平台键
 * If you do not define a platform key, the system will assign the following
 * platform key:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 */
QString Updater::platformKey() const
{
    return m_platform;
}

/**
 * Returns the download URL defined by the update definitions file.
 * 返回更新定义文件中定义的下载URL
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::downloadUrl() const
{
    return m_downloadUrl;
}

/**
 * Returns the latest version defined by the update definitions file.
 * 返回更新定义文件中定义的最新版本
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::latestVersion() const
{
    return m_latestVersion;
}

/**
 * Returns the user-agent header used by the client when communicating
 * 返回在HTTP通信中使用的用户代理标头
 * with the server through HTTP
 */
QString Updater::userAgentString() const
{
    return m_userAgentString;
}

/**
 * Returns the "local" version of the installed module
 * 返回已安装模块的本地版本
 */
QString Updater::moduleVersion() const
{
    return m_moduleVersion;
}

/**
 * Returns \c true if the updater should NOT interpret the downloaded appcast.
 * 如果更新程序不应解释下载的应用广播，则返回true
 * This is useful if you need to store more variables (or information) in the
 * JSON file or use another appcast format (e.g. XML)
 */
bool Updater::customAppcast() const
{
    return m_customAppcast;
}

/**
 * Returns \c true if the updater should notify the user when an update is
 * available.
 * 如果更新程序应在有更新时通知用户，则返回true
 */
bool Updater::notifyOnUpdate() const
{
    return m_notifyOnUpdate;
}

/**
 * Returns \c true if the updater should notify the user when it finishes
 * checking for updates.
 * 如果更新程序应在检查更新完成时通知用户，则返回true
 *
 * \note If set to \c true, the \c Updater will notify the user even when there
 *       are no updates available (by congratulating him/her about being smart)
 */
bool Updater::notifyOnFinish() const
{
    return m_notifyOnFinish;
}

/**
 * Returns \c true if there the current update is mandatory.
 * 如果当前更新是强制性的，则返回true
 * \warning You should call \c checkForUpdates() before using this function
 */
bool Updater::mandatoryUpdate() const
{
    return m_mandatoryUpdate;
}

/**
 * Returns \c true if there is an update available.
 * 如果有更新可用，则返回true
 * \warning You should call \c checkForUpdates() before using this function
 */
bool Updater::updateAvailable() const
{
    return m_updateAvailable;
}

/**
 * Returns \c true if the integrated downloader is enabled.
 * 如果集成下载器已启用，则返回true
 * \note If set to \c true, the \c Updater will open the downloader dialog if
 *       the user agrees to download the update.
 */
bool Updater::downloaderEnabled() const
{
    return m_downloaderEnabled;
}

/**
 * Returns \c true if the updater shall not intervene when the download has
 * finished (you can use the \c QSimpleUpdater signals to know when the
 * download is completed).
 * 如果在下载完成时更新程序不应干预，则返回true
 */
bool Updater::useCustomInstallProcedures() const
{
    return m_downloader->useCustomInstallProcedures();
}

/**
 * Downloads and interpets the update definitions file referenced by the
 * \c url() function.
 * 下载和解释更新定义文件
 */
void Updater::checkForUpdates()
{
    QNetworkRequest request(url());
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    if (!userAgentString().isEmpty())
    {
        request.setRawHeader("User-Agent", userAgentString().toUtf8());
    }
    m_manager->get(request);

}

/**
 * Changes the \c url in which the \c Updater can find the update definitions
 * file.
 * 更改更新定义文件的URL
 */
void Updater::setUrl(const QString &url)
{
    m_url = url;
}

/**
 * Changes the module \a name.
 * 更改模块名称
 * \note The module name is used on the user prompts. If the module name is
 *       empty, then the prompts will show the name of the application.
 */
void Updater::setModuleName(const QString &name)
{
    m_moduleName = name;
}

/**
 * If \a notify is set to \c true, then the \c Updater will notify the user
 * when an update is available.
 * 设置是否在有更新时通知用户
 */
void Updater::setNotifyOnUpdate(const bool notify)
{
    m_notifyOnUpdate = notify;
}

/**
 * If \a notify is set to \c true, then the \c Updater will notify the user
 * when it has finished interpreting the update definitions file.
 * 设置是否在检查更新完成时通知用户
 */
void Updater::setNotifyOnFinish(const bool notify)
{
    m_notifyOnFinish = notify;
}

/**
 * Changes the user agent string used to identify the client application
 * from the server in a HTTP session.
 * 更改用户代理字符串
 *
 * By default, the user agent will co
 */
void Updater::setUserAgentString(const QString &agent)
{
    m_userAgentString = agent;
    m_downloader->setUserAgentString(agent);
}

/**
 * Changes the module \a version
 * 更改模块版本
 * \note The module version is used to compare the local and remote versions.
 *       If the \a version parameter is empty, then the \c Updater will use the
 *       application version (referenced by \c qApp)
 */
void Updater::setModuleVersion(const QString &version)
{
    m_moduleVersion = version;
}

/**
 * If the \a enabled parameter is set to \c true, the \c Updater will open the
 * integrated downloader if the user agrees to install the update (if any)
 * 设置是否启用集成下载器
 */
void Updater::setDownloaderEnabled(const bool enabled)
{
    m_downloaderEnabled = enabled;
}

/**
 * 更改集成下载器的下载目录
 */
void Updater::setDownloadDir(const QString &dir)
{
    m_downloader->setDownloadDir(dir);
}

/**
 * Changes the platform key.
 * 更改平台键
 * If the platform key is empty, then the system will use the following keys:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 */
void Updater::setPlatformKey(const QString &platformKey)
{
    m_platform = platformKey;
}

/**
 * If the \a customAppcast parameter is set to \c true, then the \c Updater
 * will not try to read the network reply from the server, instead, it will
 * emit the \c appcastDownloaded() signal, which allows the application to
 * read and interpret the appcast file by itself
 * 设置是否使用自定义应用广播
 */
void Updater::setUseCustomAppcast(const bool customAppcast)
{
    m_customAppcast = customAppcast;
}

/**
 * If the \a custom parameter is set to \c true, the \c Updater will not try
 * to open the downloaded file. Use the signals fired by the \c QSimpleUpdater
 * to install the update from the downloaded file by yourself.
 * 设置是否使用自定义安装程序
 */
void Updater::setUseCustomInstallProcedures(const bool custom)
{
    m_downloader->setUseCustomInstallProcedures(custom);
}

/**
 * If the \a mandatory_update is set to \c true, the \c Updater has to download and install the
 * update. If the user cancels or exits, the application will close
 * 设置当前更新是否强制
 */
void Updater::setMandatoryUpdate(const bool mandatory_update)
{
    m_mandatoryUpdate = mandatory_update;
}
/**
 * Called when the download of the update definitions file is finished.
 * 在更新定义文件下载完成时调用
 * 处理重定向、网络错误、自定义应用广播解释、JSON解析，并设置更新信息。
 */
void Updater::onReply(QNetworkReply *reply)
{
    /* Check if we need to redirect 检查是否需要重定向
    * 如果收到了重定向的URL，将新的URL设置为当前URL，并重新发起检查更新的请求。
    */
    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirectionTarget.isNull()) {
        QUrl redirect = redirectionTarget.toUrl();
        if (redirect.isValid()) {
            // 重定向URL有效，可以使用redirect
            qDebug() << "Redirect to:" << redirect.toString();
        } else {
            // 重定向URL无效
            qDebug() << "Invalid redirect URL";
        }
    } else {
        // 没有重定向
        qDebug() << "No redirection";
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    qInfo()<<redirect;
    if (!redirect.isEmpty())
    {
        setUrl(redirect.toString());
        checkForUpdates();
        return;
    }

    /* There was a network error 处理网络错误
    * 如果发生网络错误，设置更新不可用并发出 checkingFinished 信号。
    */
    if (reply->error() != QNetworkReply::NoError)
    {
        qInfo()<<"sss";
        setUpdateAvailable(false);
        emit checkingFinished(url());
        return;
    }
    /* The application wants to interpret the appcast by itself
    * 处理自定义的应用程序广播（appcast）是为了允许应用程序在检查更新时自定义处理广播的情况
    * 这通常用于应用程序需要特殊处理更新信息的情况，而不仅仅是解析标准的JSON响应
    */
    if (customAppcast())
    {
        emit appcastDownloaded(url(), reply->readAll());
        emit checkingFinished(url());
        return;
    }

    /* Try to create a JSON document from downloaded data
     * 尝试从下载的数据中创建一个 JSON 文档
     */
    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

    /* JSON is invalid  如果 JSON 无效，设置更新不可用并发出 checkingFinished 信号*/
    if (document.isNull())
    {
        qInfo()<<"fff";
        setUpdateAvailable(false);
        emit checkingFinished(url());
        return;
    }

    /* Get the platform information  获取平台信息和更新信息 */
    QJsonObject updates = document.object().value("updates").toObject();
    QJsonObject platform = updates.value(platformKey()).toObject();

    /* Get update information 从 JSON 文档中提取更新信息 */
    m_openUrl = platform.value("open-url").toString();
    m_changelog = platform.value("changelog").toString();
    m_downloadUrl = platform.value("download-url").toString();
    m_latestVersion = platform.value("latest-version").toString();
    //"mandatory-update"强制更新
    if (platform.contains("mandatory-update"))
        m_mandatoryUpdate = platform.value("mandatory-update").toBool();

    /* Compare latest and current version
     * 比较版本并设置相应信息（比较最新版本和当前版本，并设置更新是否可用）
     *
     * setUpdateAvailable(bool) 根据更新的可用性和更新程序的设置提示用户
     */
    setUpdateAvailable(compare(latestVersion(), moduleVersion()));

    /* 无论上述哪种情况，最后都会发出 checkingFinished 信号，通知检查完成 */
    emit checkingFinished(url());
}

/**
 * Prompts the user based on the value of the \a available parameter and the
 * settings of this instance of the \c Updater class.
 * 根据更新的可用性和更新程序的设置提示用户。
 */
void Updater::setUpdateAvailable(const bool available)
{
    m_updateAvailable = available;

    QMessageBox box;
    box.setTextFormat(Qt::RichText);
    box.setIcon(QMessageBox::Information);
    box.setWindowIcon(QIcon(":/icons/nupdate.png"));


    if (updateAvailable() && (notifyOnUpdate() || notifyOnFinish()))
    {
        QString text = tr("New versions of updates are available. Do you want to download them now?");
        if (m_mandatoryUpdate)
        {
            text = tr("Do you want to download the update now? This is a mandatory update, exiting now will close the application");
        }

        QString title
                = "<h3>" + tr(" %1 Version of %2 Published!").arg(latestVersion()).arg(moduleName()) + "</h3>";

        box.setText(title);
        box.setWindowTitle("Download window");
        box.setWindowIcon(QIcon(":/icons/nupdate.png"));
        box.setDetailedText(tr(m_changelog.toUtf8().constData()));
        box.setInformativeText(text);

        box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
        box.setDefaultButton(QMessageBox::Yes);

        if (box.exec() == QMessageBox::Yes)
        {
            if (!openUrl().isEmpty())
                QDesktopServices::openUrl(QUrl(openUrl()));

            else if (downloaderEnabled())
            {
                m_downloader->setUrlId(url());
                m_downloader->setFileName(downloadUrl().split("/").last());
                m_downloader->setMandatoryUpdate(m_mandatoryUpdate);
                m_downloader->startDownload(QUrl(downloadUrl()));
            }

            else
                QDesktopServices::openUrl(QUrl(downloadUrl()));
        }
        else
        {
            if (m_mandatoryUpdate)
            {
                QApplication::quit();
            }
        }
    }

    else if (notifyOnFinish())
    {
        box.setStandardButtons(QMessageBox::Close);
        box.setInformativeText(tr("There are currently no available updates"));
        box.setText("<h3>"
                    + tr("Currently, it is the latest version %1")
                    .arg(moduleName())
                    + "</h3>");

        box.exec();
    }
}

/**
 * Compares the two version strings (\a x and \a y).
 *     - If \a x is greater than \y, this function returns \c true.
 *     - If \a y is greater than \x, this function returns \c false.
 *     - If both versions are the same, this function returns \c false.
 * 比较两个版本字符串，如果 x 大于 y 则返回 true
 */
bool Updater::compare(const QString &x, const QString &y)
{
    qInfo()<<x<<y;
    QStringList versionsX = x.split(".");
    QStringList versionsY = y.split(".");

    int count = qMin(versionsX.count(), versionsY.count());

    for (int i = 0; i < count; ++i)
    {
        int a = QString(versionsX.at(i)).toInt();
        int b = QString(versionsY.at(i)).toInt();

        if (a > b)
            return true;

        else if (b > a)
            return false;
    }

    return versionsY.count() < versionsX.count();
}

#if QSU_INCLUDE_MOC
#   include "moc_Updater.cpp"
#endif
