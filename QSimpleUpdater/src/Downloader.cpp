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

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDateTime>
#include <QMessageBox>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <math.h>

#include "Downloader.h"

static const QString PARTIAL_DOWN(".part");

//构造函数，初始化界面和成员变量
Downloader::Downloader(QWidget *parent)
   : QWidget(parent)
{
   m_ui = new Ui::Downloader;
   m_ui->setupUi(this);

   /* Initialize private members */
   m_manager = new QNetworkAccessManager();

   /* Initialize internal values */
   m_url = "";
   m_fileName = "";
   m_startTime = 0;
   m_useCustomProcedures = false;
   m_mandatoryUpdate = false;

   /* Set download directory */
   m_downloadDir.setPath(QDir::homePath() + "/Downloads/");

   /* Make the window look like a modal dialog */
   setWindowIcon(QIcon());
   setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

   /* Configure the appearance and behavior of the buttons */
   m_ui->openButton->setEnabled(false);
   m_ui->openButton->setVisible(false);
   connect(m_ui->stopButton, SIGNAL(clicked()), this, SLOT(cancelDownload()));
   connect(m_ui->openButton, SIGNAL(clicked()), this, SLOT(installUpdate()));

   /* Resize to fit */
   setFixedSize(minimumSizeHint());
}

//析构函数，释放内存
Downloader::~Downloader()
{
   delete m_ui;
   delete m_reply;
   delete m_manager;
}

/**
 * Returns \c true if the updater shall not intervene when the download has
 * finished (you can use the \c QSimpleUpdater signals to know when the
 * download is completed).
 * 返回是否使用自定义安装过程
 */
bool Downloader::useCustomInstallProcedures() const
{
   return m_useCustomProcedures;
}

/**
 * Changes the URL, which is used to indentify the downloader dialog
 * with an \c Updater instance
 * 设置与Updater实例相关联的URL
 * \note the \a url parameter is not the download URL, it is the URL of
 *       the AppCast file
 */
void Downloader::setUrlId(const QString &url)
{
   m_url = url;
}

/**
 * Begins downloading the file at the given \a url
 * 通过指定的URL开始下载文件。它配置网络请求并启动下载。下载过程中，使用信号和槽机制实时更新UI。
 */
void Downloader::startDownload(const QUrl &url)
{
   /* Reset UI */
   m_ui->progressBar->setValue(0);//将下载进度条重置为0
   m_ui->stopButton->setText(tr("停止"));
   m_ui->downloadLabel->setText(tr("下载更新"));
   m_ui->timeLabel->setText(tr("剩余时间") + ": " + tr("..."));

   /* Configure the network request 创建QNetworkRequest对象，配置URL和一些请求属性，例如重定向策略和用户代理*/
   QNetworkRequest request(url);
   request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
   if (!m_userAgentString.isEmpty())
      request.setRawHeader("User-Agent", m_userAgentString.toUtf8());

   /* Start download */
   m_reply = m_manager->get(request);
   m_startTime = QDateTime::currentDateTime().toSecsSinceEpoch();

   /* Ensure that downloads directory exists 检查下载目录是否存在，如果不存在则创建 */
   if (!m_downloadDir.exists())
      m_downloadDir.mkpath(".");

   /* Remove old downloads 删除可能存在的旧的下载文件和部分下载文件*/
   QFile::remove(m_downloadDir.filePath(m_fileName));
   QFile::remove(m_downloadDir.filePath(m_fileName + PARTIAL_DOWN));

   /* Update UI when download progress changes or download finishes */
   //SIGNAL(metaDataChanged()) >> 在QNetworkReply的元数据发生变化时（例如，文件名变化）发射>>metaDataChanged()信号被用于获取HTTP响应的Content-Disposition头，从中提取文件名。这个文件名是服务端告诉客户端下载文件时建议的文件名。获取到文件名后，它会被用作下载文件的名称，以便后续的文件保存和处理。
   //SIGNAL(downloadProgress(qint64, qint64)) >> 在下载过程中发射，提供已接收和总大小的参数，用于更新下载进度
   //SIGNAL(finished()) >> 在下载完成时发射
   connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
   connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateProgress(qint64, qint64)));
   connect(m_reply, SIGNAL(finished()), this, SLOT(finished()));

   //显示下载器窗口函数，是QWidget类的成员函数，被用于确保下载器窗口在开始下载时处于正常状态，以便用户能够看到和交互下载进度。这样，即使用户之前将窗口最小化或最大化，下载开始时窗口会被还原到正常状态
   showNormal();
}

/**
 * Changes the name of the downloaded file
 * 设置下载的文件名
 */
void Downloader::setFileName(const QString &file)
{
   m_fileName = file;

   if (m_fileName.isEmpty())
      m_fileName = "QSU_Update.bin";
}

/**
 * Changes the user-agent string used to communicate with the remote HTTP server
 * 设置用于与远程HTTP服务器通信的用户代理字符串
 */
void Downloader::setUserAgentString(const QString &agent)
{
   m_userAgentString = agent;
}

/**
 * 下载完成后的处理，重命名文件，发射下载完成信号，安装更新
 */
void Downloader::finished()
{
   if (m_reply->error() != QNetworkReply::NoError)
   {
      QFile::remove(m_downloadDir.filePath(m_fileName + PARTIAL_DOWN));
      return;
   }

   /* Rename file */
   QFile::rename(m_downloadDir.filePath(m_fileName + PARTIAL_DOWN), m_downloadDir.filePath(m_fileName));

   /* Notify application */
   emit downloadFinished(m_url, m_downloadDir.filePath(m_fileName));

   /* Install the update */
   m_reply->close();
   installUpdate();
   setVisible(false);
}

/**
 * Opens the downloaded file.
 * 打开下载的文件
 * \note If the downloaded file is not found, then the function will alert the
 *       user about the error.
 */
void Downloader::openDownload()
{
   if (!m_fileName.isEmpty())
      QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadDir.filePath(m_fileName)));

   else
   {
      QMessageBox::critical(this, tr("Error"), tr("无法找到已下载的更新文件!"), QMessageBox::Close);
   }
}

/**
 * Instructs the OS to open the downloaded file.
 * 安装更新，提示用户是否退出应用
 *
 * \note If \c useCustomInstallProcedures() returns \c true, the function will
 *       not instruct the OS to open the downloaded file. You can use the
 *       signals fired by the \c QSimpleUpdater to install the update with your
 *       own implementations/code.
 */
void Downloader::installUpdate()
{
   if (useCustomInstallProcedures())
      return;

   /* Update labels */
   m_ui->stopButton->setText(tr("关闭"));
   m_ui->downloadLabel->setText(tr("已完成下载!"));
   m_ui->timeLabel->setText(tr("即将打开已安装程序，请稍后") + "...");

   /* Ask the user to install the download */
   QMessageBox box;
   box.setIcon(QMessageBox::Question);
   box.setWindowIcon(QIcon(":/icons/nupdate.png"));
   box.setWindowTitle("安装窗口");
   box.setDefaultButton(QMessageBox::Ok);
   box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
   box.setInformativeText(tr("点击 \"OK\" 开始安装"));

   QString text = tr("为了完成新版本的安装，我们将关闭当前的应用程序，并在安装完成后重新启动应用程序");

   if (m_mandatoryUpdate)
      text = tr("为了完成新版本的安装，我们将关闭当前的应用程序，并在安装完成后重新启动应用程序。这是一个强制更新，现在退出将关闭应用程序");

   box.setText("<h3>" + text + "</h3>");

   /* User wants to install the download
    * 用户想要安装 下载文件
    */
   if (box.exec() == QMessageBox::Ok)
   {
      if (!useCustomInstallProcedures())
           openDownload();

      //关闭当前的应用程序
      QApplication::quit();
   }
   /* Wait */
   else
   {
      if (m_mandatoryUpdate)
         QApplication::quit();
//      m_ui->openButton->setEnabled(true);
//      m_ui->openButton->setVisible(true);
//      m_ui->timeLabel->setText(tr("点击 \"打开\" 按钮来应用更新"));
   }
}

/**
 * Prompts the user if he/she wants to cancel the download and cancels the
 * download if the user agrees to do that.
 * 取消下载，如果是强制更新，可能会退出应用
 */
void Downloader::cancelDownload()
{
   if (!m_reply->isFinished())
   {
      QMessageBox box;
      box.setWindowTitle(tr("取消更新"));
      box.setIcon(QMessageBox::Question);
      box.setWindowIcon(QIcon(":/icons/nupdate.png"));
      box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

      QString text = tr("确定要取消下载吗？?");
      if (m_mandatoryUpdate)
      {
         text = tr("确定要取消下载吗？这是一个强制更新，现在退出将关闭应用程序。");
      }
      box.setText(text);
      if (box.exec() == QMessageBox::Yes)
      {
         hide();
         m_reply->abort();
         if (m_mandatoryUpdate)
            QApplication::quit();
      }
   }
   else
   {
      if (m_mandatoryUpdate)
         QApplication::quit();

      hide();
   }
}

/**
 * Writes the downloaded data to the disk
 * 将下载的数据写入磁盘
 */
void Downloader::saveFile(qint64 received, qint64 total)
{
   Q_UNUSED(received);
   Q_UNUSED(total);

   /* Check if we need to redirect */
   QUrl url = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
   if (!url.isEmpty())
   {
      startDownload(url);
      return;
   }

   /* Save downloaded data to disk */
   QFile file(m_downloadDir.filePath(m_fileName + PARTIAL_DOWN));
   if (file.open(QIODevice::WriteOnly | QIODevice::Append))
   {
      file.write(m_reply->readAll());
      file.close();
   }
}

/**
 * Calculates the appropiate size units (bytes, KB or MB) for the received
 * data and the total download size. Then, this function proceeds to update the
 * dialog controls/UI.
 * 计算合适的大小单位（字节，KB或MB）并更新对话框控件/UI
 */
void Downloader::calculateSizes(qint64 received, qint64 total)
{
   QString totalSize;
   QString receivedSize;

   if (total < 1024)
      totalSize = tr("%1 bytes").arg(total);

   else if (total < 1048576)
      totalSize = tr("%1 KB").arg(round(total / 1024));

   else
      totalSize = tr("%1 MB").arg(round(total / 1048576));

   if (received < 1024)
      receivedSize = tr("%1 bytes").arg(received);

   else if (received < 1048576)
      receivedSize = tr("%1 KB").arg(received / 1024);

   else
      receivedSize = tr("%1 MB").arg(received / 1048576);

   m_ui->downloadLabel->setText(tr("正在下载更新") + " (" + receivedSize + " " + tr("/") + " " + totalSize
                                + ")");
}

/**
 * Get response filename.
 * 获取响应的文件名
 */
void Downloader::metaDataChanged()
{
   QString filename = "";
   QVariant variant = m_reply->header(QNetworkRequest::ContentDispositionHeader);
   if (variant.isValid())
   {
      QString contentDisposition = QByteArray::fromPercentEncoding(variant.toByteArray()).constData();
      QRegularExpression regExp("filename=(\S+)");
      QRegularExpressionMatch match = regExp.match(contentDisposition);
      if (match.hasMatch())
      {
         filename = match.captured(1);
      }
      setFileName(filename);
   }
}

/**
 * Uses the \a received and \a total parameters to get the download progress
 * and update the progressbar value on the dialog.
 * 更新下载进度条，计算下载剩余时间，保存文件
 */
void Downloader::updateProgress(qint64 received, qint64 total)
{
   if (total > 0)
   {
      m_ui->progressBar->setMinimum(0);
      m_ui->progressBar->setMaximum(100);
      m_ui->progressBar->setValue((received * 100) / total);

      calculateSizes(received, total);
      calculateTimeRemaining(received, total);
      saveFile(received, total);
   }

   else
   {
      m_ui->progressBar->setMinimum(0);
      m_ui->progressBar->setMaximum(0);
      m_ui->progressBar->setValue(-1);
      m_ui->downloadLabel->setText(tr("更新") + "...");
      m_ui->timeLabel->setText(QString("%1: %2").arg(tr("剩余时间")).arg(tr("...")));
   }
}

/**
 * Uses two time samples (from the current time and a previous sample) to
 * calculate how many bytes have been downloaded.
 * 根据两个时间样本计算剩余时间
 * Then, this function proceeds to calculate the appropiate units of time
 * (hours, minutes or seconds) and constructs a user-friendly string, which
 * is displayed in the dialog.
 */
void Downloader::calculateTimeRemaining(qint64 received, qint64 total)
{
   uint difference = QDateTime::currentDateTime().toSecsSinceEpoch() - m_startTime;

   if (difference > 0)
   {
      QString timeString;
      qreal timeRemaining = (total - received) / (received / difference);

      if (timeRemaining > 7200)
      {
         timeRemaining /= 3600;
         int hours = int(timeRemaining + 0.5);

         if (hours > 1)
            timeString = tr("大概 %1 小时").arg(hours);
         else
            timeString = tr("大概 1 小时");
      }

      else if (timeRemaining > 60)
      {
         timeRemaining /= 60;
         int minutes = int(timeRemaining + 0.5);

         if (minutes > 1)
            timeString = tr("%1 分钟").arg(minutes);
         else
            timeString = tr("1 分钟");
      }

      else if (timeRemaining <= 60)
      {
         int seconds = int(timeRemaining + 0.5);

         if (seconds > 1)
            timeString = tr("%1 秒").arg(seconds);
         else
            timeString = tr("1 秒");
      }

      m_ui->timeLabel->setText(tr("剩余时间") + ": " + timeString);
   }
}

/**
 * Rounds the given \a input to two decimal places
 * 将输入四舍五入到两位小数
 */
qreal Downloader::round(const qreal &input)
{
   return static_cast<qreal>(roundf(static_cast<float>(input) * 100) / 100);
}

/**
 * 获取下载目录
 */
QString Downloader::downloadDir() const
{
   return m_downloadDir.absolutePath();
}

/**
 * 设置下载目录
 */
void Downloader::setDownloadDir(const QString &downloadDir)
{
   if (m_downloadDir.absolutePath() != downloadDir)
      m_downloadDir.setPath(downloadDir);
}

/**
 * If the \a mandatory_update is set to \c true, the \c Downloader has to download and install the
 * update. If the user cancels or exits, the application will close
 * 设置是否是强制更新
 */
void Downloader::setMandatoryUpdate(const bool mandatory_update)
{
   m_mandatoryUpdate = mandatory_update;
}

/**
 * If the \a custom parameter is set to \c true, then the \c Downloader will not
 * attempt to open the downloaded file.
 * 设置是否使用自定义安装过程
 * Use the signals fired by the \c QSimpleUpdater to implement your own install
 * procedures.
 */
void Downloader::setUseCustomInstallProcedures(const bool custom)
{
   m_useCustomProcedures = custom;
}

#if QSU_INCLUDE_MOC
#   include "moc_Downloader.cpp"
#endif
