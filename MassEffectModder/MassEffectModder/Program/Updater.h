/*
 * MassEffectModder
 *
 * Copyright (C) 2019 Pawel Kolodziejski
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef UPDATER_H
#define UPDATER_H

class MainWindow;

class Updater : public QWidget
{
    Q_OBJECT

    const int kButtonMinWidth = 200;
    const int kButtonMinHeight = 22;
#if defined(__APPLE__)
    const int kFontSize = 12;
#elif defined(__linux__)
    const int kFontSize = 8;
#else
    const int kFontSize = 8;
#endif

    QNetworkAccessManager managerListReleases;
    QNetworkAccessManager managerRelease;
    QNetworkReply         *replyRelease;
    MainWindow            *parentWindow;
    QDialog               dialog;
    QLabel                textWidget;
    QPushButton           button;
    QProgressBar          progressBar;
    QTemporaryDir         *tmpDir{};
    QFile                 *file{};
    QString               filePath;

    void downloadRelease(const QString &downLoadUrl);

public:
    Updater(MainWindow *window);
    ~Updater() override;

private slots:
    void finishedDownloadRelasesList(QNetworkReply *reply);
    void finishedDownloadRelase(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);
    void ButtonSelected();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

public slots:
    void processUpdate();
};

#endif
