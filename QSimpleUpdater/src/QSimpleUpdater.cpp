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

#include "Updater.h"
#include "QSimpleUpdater.h"


/**
 * URLS 是一个静态的 QList，保存了一组url字符串（QString）。
 * UPDATERS 是一个静态的 QList，保存了一组指向 Updater 类对象的指针。
 */
static QList<QString> URLS;
static QList<Updater *> UPDATERS;

QSimpleUpdater::~QSimpleUpdater()
{
    URLS.clear();

    foreach (Updater *updater, UPDATERS)
        updater->deleteLater();

    UPDATERS.clear();
}

/**
 * 获取类的唯一实例
 * Returns the only instance of the class
 */
QSimpleUpdater *QSimpleUpdater::getInstance()
{
    static QSimpleUpdater updater;
    return &updater;
}

/**
 * 检查是否使用自定义的应用程序清单（appcast）格式
 * Returns \c true if the \c Updater instance registered with the given \a url
 * uses a custom appcast format and/or allows the application to read and
 * interpret the downloaded appcast file
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::usesCustomAppcast(const QString &url) const
{
    return getUpdater(url)->customAppcast();
}

/**
 * 检查是否在更新可用时通知用户
 * Returns \c true if the \c Updater instance registered with the given \a url
 * shall notify the user when an update is available.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::getNotifyOnUpdate(const QString &url) const
{
    return getUpdater(url)->notifyOnUpdate();
}

/**
 * 检查是否在更新检查完成时通知用户
 * Returns \c true if the \c Updater instance registered with the given \a url
 * shall notify the user when it finishes checking for updates.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::getNotifyOnFinish(const QString &url) const
{
    return getUpdater(url)->notifyOnFinish();
}

/**
 * 检查是否有可用的更新
 * Returns \c true if the \c Updater instance registered with the given \a url
 * has an update available.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::getUpdateAvailable(const QString &url) const
{
    return getUpdater(url)->updateAvailable();
}

/**
 * 检查是否启用集成的下载器
 * Returns \c true if the \c Updater instance registered with the given \a url
 * has the integrated downloader enabled.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::getDownloaderEnabled(const QString &url) const
{
    return getUpdater(url)->downloaderEnabled();
}

/**
 * 检查是否使用自定义的安装程序
 * Returns \c true if the \c Updater instance registered with the given \a url
 * shall try to open the downloaded file.
 *
 * If you want to implement your own way to handle the downloaded file, just
 * bind to the \c downloadFinished() signal and disable the integrated
 * downloader with the \c setUseCustomInstallProcedures() function.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::usesCustomInstallProcedures(const QString &url) const
{
    return getUpdater(url)->useCustomInstallProcedures();
}

/**
 * 获取用于在Web浏览器中打开的URL
 * Returns the URL to open in a web browser of the \c Updater instance
 * registered with the given \a url.
 *
 * \note If the module name is empty, then the \c Updater will use the
 *       application name as its module name.
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getOpenUrl(const QString &url) const
{
    return getUpdater(url)->openUrl();
}

/**
 * 获取更新日志
 * Returns the changelog of the \c Updater instance registered with the given
 * \a url.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getChangelog(const QString &url) const
{
    return getUpdater(url)->changelog();
}

/**
 * 获取模块名称
 * Returns the module name of the \c Updater instance registered with the given
 * \a url.
 *
 * \note If the module name is empty, then the \c Updater will use the
 *       application name as its module name.
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getModuleName(const QString &url) const
{
    return getUpdater(url)->moduleName();
}

/**
 * 获取下载URL
 * Returns the download URL of the \c Updater instance registered with the given
 * \a url.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getDownloadUrl(const QString &url) const
{
    return getUpdater(url)->downloadUrl();
}

/**
 * 获取平台(操作系统)关键字
 * Returns the platform key of the \c Updater registered with the given \a url.
 * If you do not define a platform key, the system will assign the following
 * platform key:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getPlatformKey(const QString &url) const
{
    return getUpdater(url)->platformKey();
}

/**
 * 获取远程模块版本
 * Returns the remote module version of the \c Updater instance registered with
 * the given \a url.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getLatestVersion(const QString &url) const
{
    return getUpdater(url)->latestVersion();
}

/**
 * 获取模块版本
 * Returns the module version of the \c Updater instance registered with the
 * given \a url.
 *
 * \note If the module version is empty, then the \c Updater will use the
 *       application version as its module version.
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getModuleVersion(const QString &url) const
{
    return getUpdater(url)->moduleVersion();
}

/**
 * 获取用于与远程服务器通信的用户代理字符串
 * Returns the user-agent string used by the updater to communicate with
 * the remote HTTP(S) server.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getUserAgentString(const QString &url) const
{
    return getUpdater(url)->userAgentString();
}

/**
 * 检查更新
 * Instructs the \c Updater instance with the registered \c url to download and
 * interpret the update definitions file.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::checkForUpdates(const QString &url)
{
    getUpdater(url)->checkForUpdates();
}

void QSimpleUpdater::setDownloadDir(const QString &url, const QString &dir)
{
   getUpdater(url)->setDownloadDir(dir);
}

/**
 * 设置模块名称
 * Changes the module \a name of the \c Updater instance registered at the
 * given \a url.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 * \note The module name is used on the user prompts. If the module name is
 *       empty, then the prompts will show the name of the application.
 */
void QSimpleUpdater::setModuleName(const QString &url, const QString &name)
{
    getUpdater(url)->setModuleName(name);
}

/**
 * 设置是否在更新可用时通知用户
 * If \a notify is set to \c true, then the \c Updater instance registered with
 * the given \a url will notify the user when an update is available.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setNotifyOnUpdate(const QString &url, const bool notify)
{
    getUpdater(url)->setNotifyOnUpdate(notify);
}

/**
 * 设置是否在更新检查完成时通知用户
 * If \a notify is set to \c true, then the \c Updater instance registered with
 * the given \a url will notify the user when it has finished interpreting the
 * update definitions file.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setNotifyOnFinish(const QString &url, const bool notify)
{
    getUpdater(url)->setNotifyOnFinish(notify);
}

/**
 * 设置平台关键字
 * Changes the platform key of the \c Updater isntance registered at the given
 * \a url.
 *
 * If the platform key is empty, then the system will use the following keys:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setPlatformKey(const QString &url, const QString &platform)
{
    getUpdater(url)->setPlatformKey(platform);
}

/**
 * 设置模块版本
 * Changes the module \version of the \c Updater instance registered at the
 * given \a url.
 *
 * \note The module version is used to compare it with the remove version.
 *       If the module name is empty, then the \c Updater instance will use the
 *       application version.
 */
void QSimpleUpdater::setModuleVersion(const QString &url, const QString &version)
{
    getUpdater(url)->setModuleVersion(version);
}

/**
 * 设置是否启用集成的下载器
 * If the \a enabled parameter is set to \c true, the \c Updater instance
 * registered with the given \a url will open the integrated downloader
 * if the user agrees to install the update (if any).
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setDownloaderEnabled(const QString &url, const bool enabled)
{
    getUpdater(url)->setDownloaderEnabled(enabled);
}

/**
 * 设置与远程服务器通信的用户代理字符串
 * Changes the user-agent string used by the updater to communicate
 * with the remote server
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setUserAgentString(const QString &url, const QString &agent)
{
    getUpdater(url)->setUserAgentString(agent);
}

/**
 * 设置是否使用自定义应用程序清单格式
 * If the \a customAppcast parameter is set to \c true, then the \c Updater
 * will not try to read the network reply from the server, instead, it will
 * emit the \c appcastDownloaded() signal, which allows the application to
 * read and interpret the appcast file by itself.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setUseCustomAppcast(const QString &url, const bool customAppcast)
{
    getUpdater(url)->setUseCustomAppcast(customAppcast);
}

/**
 * 设置是否使用自定义安装程序
 * If the \a custom parameter is set to \c true, the \c Updater instance
 * registered with the given \a url will not try to open the downloaded file.
 *
 * If you want to implement your own way to handle the downloaded file, just
 * bind to the \c downloadFinished() signal and disable the integrated
 * downloader with the \c setUseCustomInstallProcedures() function.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setUseCustomInstallProcedures(const QString &url, const bool custom)
{
    getUpdater(url)->setUseCustomInstallProcedures(custom);
}
/**
 * 设置是否为强制更新
 */
void QSimpleUpdater::setMandatoryUpdate(const QString &url, const bool mandatory_update)
{
    getUpdater(url)->setMandatoryUpdate(mandatory_update);
}

/**
 * 获取注册在给定URL的 Updater 实例，如果不存在则自动初始化。
 * Returns the \c Updater instance registered with the given \a url.
 *
 * If an \c Updater instance registered with teh given \a url does not exist,
 * this function will create it and configure it automatically.
 */
Updater *QSimpleUpdater::getUpdater(const QString &url) const
{
    if (!URLS.contains(url))
    {
        /**
        * 创建一个 Updater 类的实例并分配其内存空间给指针 updater
        * 一个动态分配对象的操作，用于在堆上创建一个 Updater 对象
        */
        Updater *updater = new Updater;
        updater->setUrl(url);

        URLS.append(url);
        UPDATERS.append(updater);

        /**
         * checkingFinished 信号是 Updater 类中定义的信号，用于通知检查完成。
         * 当 Updater 对象发出 checkingFinished 信号时，当前对象也会发出相同的信号
         * 连接这两个信号的目的是在当前对象中处理 Updater 对象检查完成的事件。
         * (也就是 Window.cpp中的“connect(m_updater, SIGNAL(checkingFinished(QString)), this, SLOT(updateChangelog(QString)))”中处理“updateChangelog(QString)”)
         */
        connect(updater, SIGNAL(checkingFinished(QString)), this, SIGNAL(checkingFinished(QString)));
        connect(updater, SIGNAL(downloadFinished(QString, QString)), this, SIGNAL(downloadFinished(QString, QString)));
        connect(updater, SIGNAL(appcastDownloaded(QString, QByteArray)), this,
                SIGNAL(appcastDownloaded(QString, QByteArray)));
    }

    //根据给定的URL返回相应 Updater 指针
    return UPDATERS.at(URLS.indexOf(url));
}

#if QSU_INCLUDE_MOC
#   include "moc_QSimpleUpdater.cpp"
#endif
