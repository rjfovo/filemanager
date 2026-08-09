/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "application.h"
#include "dbusinterface.h"
#include "window.h"
#include "thumbnailer/thumbnailprovider.h"
#include "filemanageradaptor.h"

#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QDBusConnection>
#include <QPixmapCache>
#include <QTranslator>
#include <QFileInfo>
#include <QIcon>
#include <QQuickStyle>
#include <QDir>

// KIO
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KIO/PreviewJob>
#include <KIO/DeleteJob>
#include <KIO/DropJob>
#include <KIO/FileUndoManager>
#include <KIO/JobUiDelegate>
#include <KIO/Paste>
#include <KIO/PasteJob>
#include <KIO/RestoreJob>

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
    , m_instance(false)
{
    // 显式指定 Qt Quick Controls 主题为 fish-style：
    // 会话设置了 QT_STYLE_OVERRIDE=cutefish，QQuickStyle 解析时优先用它（而不是
    // QT_QUICK_CONTROLS_STYLE），导致回退到 Basic 的"默认 demo 样式"。
    QQuickStyle::setStyle(QStringLiteral("fish-style"));

    if (QDBusConnection::sessionBus().registerService("com.cutefish.FileManager")) {
        setOrganizationName("cutefishos");
        setWindowIcon(QIcon::fromTheme("file-manager"));

        // Set icon theme for Qt6
        // In Qt6, we need to ensure icon theme is properly set
        // First set the search paths
        QStringList iconThemePaths;
        iconThemePaths << "/usr/share/icons";
        iconThemePaths << QDir::homePath() + "/.local/share/icons";
        iconThemePaths << "/usr/local/share/icons";
        QIcon::setThemeSearchPaths(iconThemePaths);
        
        // Try to set icon theme in order of preference
        QStringList preferredThemes = {"cutefish", "Crule", "Crule-dark", "breeze", "Adwaita", "hicolor"};
        QString themeSet = "hicolor"; // default fallback
        
        for (const QString &theme : preferredThemes) {
            QString themePath = QString("/usr/share/icons/%1").arg(theme);
            if (QDir(themePath).exists()) {
                themeSet = theme;
                break;
            }
        }
        
        QIcon::setThemeName(themeSet);
        qDebug() << "FileManager: Icon theme set to:" << QIcon::themeName() << "from search paths:" << QIcon::themeSearchPaths();
        
        // Ensure QIcon image provider is available for QML
        // This is needed for image://icontheme/ URLs to work
        if (QIcon::themeName().isEmpty()) {
            qWarning() << "FileManager: No icon theme set! image://icontheme/ URLs will not work.";
        }

        new FileManagerAdaptor(this);
        new DBusInterface;
        QDBusConnection::sessionBus().registerObject("/FileManager", this);

        // Translations
        QLocale locale;
        QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/cutefish-filemanager/translations/").arg(locale.name());
        if (QFile::exists(qmFilePath)) {
            QTranslator *translator = new QTranslator(this);
            if (translator->load(qmFilePath)) {
                installTranslator(translator);
            } else {
                translator->deleteLater();
            }
        }

        m_instance = true;
    }
}

Application::~Application()
{
}

int Application::run()
{
    if (!parseCommandLineArgs())
        return 0;

    return QApplication::exec();
}

void Application::openFiles(const QStringList &paths)
{
    if (paths.isEmpty()) {
        // 如果没有指定路径，打开默认主目录
        qDebug() << "FileManager::openFiles - paths is empty, opening home directory";
        openWindow(QDir::homePath());
    } else {
        for (const QString &path : paths) {
            openWindow(path);
        }
    }
}

void Application::moveToTrash(const QStringList &paths)
{
    if (paths.isEmpty())
        return;

    QList<QUrl> urls;

    for (const QString &path : paths) {
        urls.append(QUrl::fromLocalFile(path));
    }

    KIO::Job *job = KIO::trash(urls);
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
    KIO::FileUndoManager::self()->recordJob(KIO::FileUndoManager::Trash, urls, QUrl(QStringLiteral("trash:/")), job);
}

void Application::emptyTrash()
{
    Window *w = new Window;
    w->load(QUrl("qrc:/qml/Dialogs/EmptyTrashDialog.qml"));
}

void Application::openWindow(const QString &path)
{
    Window *w = new Window;
    w->rootContext()->setContextProperty("arg", path);
    w->addImageProvider("thumbnailer", new ThumbnailProvider());
    
    w->load(QUrl("qrc:/qml/main.qml"));
    
    // 存储窗口对象，防止被垃圾回收
    m_windows.append(w);
    
    // 窗口关闭时自动从列表中移除
    connect(w, &QObject::destroyed, this, [this, w]() {
        m_windows.removeAll(w);
    });
}

QStringList Application::formatUriList(const QStringList &list)
{
    QStringList val;

    for (const QString &path : list) {
        val.append(path == "." ? QDir::currentPath() : path);
    }

    if (val.isEmpty()) {
        val.append(QDir::currentPath());
    }

    return val;
}

bool Application::parseCommandLineArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("File Manager"));
    parser.addHelpOption();

    parser.addPositionalArgument("files", "Files", "[FILE1, FILE2,...]");

    QCommandLineOption emptyTrashOption(QStringList() << "e" << "empty-trash" << "Empty Trash");
    parser.addOption(emptyTrashOption);

    QCommandLineOption moveToTrashOption(QStringList() << "mtr" << "move-to-trash" << "Move To Trash");
    parser.addOption(moveToTrashOption);

    parser.process(arguments());

    qDebug() << "FileManager::parseCommandLineArgs - m_instance:" << m_instance;
    qDebug() << "FileManager::parseCommandLineArgs - arguments:" << arguments();
    qDebug() << "FileManager::parseCommandLineArgs - positionalArguments:" << parser.positionalArguments();

    if (m_instance) {
        QPixmapCache::setCacheLimit(2048);

        qDebug() << "FileManager::parseCommandLineArgs - Opening files";
        openFiles(formatUriList(parser.positionalArguments()));
    } else {
        qDebug() << "FileManager::parseCommandLineArgs - Second instance, calling via D-Bus";
        QDBusInterface iface("com.cutefish.FileManager",
                             "/FileManager",
                             "com.cutefish.FileManager",
                             QDBusConnection::sessionBus(), this);

        if (parser.isSet(emptyTrashOption)) {
            // Empty Dialog
            iface.call("emptyTrash");
        } else if (parser.isSet(moveToTrashOption)) {
            iface.call("moveToTrash", parser.positionalArguments());
        } else {
            iface.call("openFiles", formatUriList(parser.positionalArguments()));
        }
    }

    return m_instance;
}
