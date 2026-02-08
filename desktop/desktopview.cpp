/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#include "desktopview.h"
#include "dockdbusinterface.h"
#include "thumbnailer/thumbnailprovider.h"

#include <QQmlEngine>
#include <QQmlContext>

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>
#include <QWidget>
#include <QIcon>
#include <QDir>
#include <QQuickImageProvider>
#include <QPixmap>

#include <KWindowSystem>

// Simple IconThemeProvider for Qt6 compatibility
class IconThemeProvider : public QQuickImageProvider
{
public:
    IconThemeProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString &id, QSize *realSize, const QSize &requestedSize) override
    {
        QSize size(requestedSize);
        if (size.width() <= 0)
            size.setWidth(32);
        if (size.height() <= 0)
            size.setHeight(32);
        
        if (realSize)
            *realSize = size;
        
        QIcon icon = QIcon::fromTheme(id);
        if (icon.isNull()) {
            // Fallback to generic icon
            icon = QIcon::fromTheme("application-x-executable");
        }
        
        return icon.pixmap(size);
    }
};

DesktopView::DesktopView(QScreen *screen, QQuickView *parent)
    : QQuickView(parent)
    , m_screen(screen)
{
    m_screenRect = m_screen->geometry();
    this->setFlag(Qt::FramelessWindowHint);
    this->setColor(QColor(Qt::transparent));

    // 在Qt6中，Qt::WindowType::Desktop可能不再像Qt5那样工作
    // 使用更兼容的标志组合
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::WindowDoesNotAcceptFocus);
    
    // 设置窗口类型为桌面，确保窗口管理器正确处理
    // 在Qt6中，我们需要使用setProperty来设置窗口类型
    setProperty("_q_platform_WindowType", "desktop");
    
    // 确保窗口不会获得焦点
    setProperty("_q_NoFocus", true);
    
    // 移除透明输入标志，以便桌面可以接收鼠标事件
    // setFlags(flags() | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
    
    // 在Qt6中，尝试使用KWindowSystem设置窗口类型
    // 注意：KF6中KWindowSystem的API可能已经改变
    // 我们尝试使用Qt原生的方式
    setProperty("_NET_WM_WINDOW_TYPE", "_NET_WM_WINDOW_TYPE_DESKTOP");

    // 设置图标主题
    // 在Qt6中，每个QML引擎都需要确保图标主题可用
    // 首先设置搜索路径
    QStringList iconThemePaths;
    iconThemePaths << "/usr/share/icons";
    iconThemePaths << QDir::homePath() + "/.local/share/icons";
    iconThemePaths << "/usr/local/share/icons";
    QIcon::setThemeSearchPaths(iconThemePaths);
    
    // 尝试按优先级设置图标主题
    QStringList preferredThemes = {"cutefish", "Crule", "Crule-dark", "breeze", "Adwaita", "hicolor"};
    QString themeSet = "hicolor"; // 默认回退
    
    for (const QString &theme : preferredThemes) {
        QString themePath = QString("/usr/share/icons/%1").arg(theme);
        if (QDir(themePath).exists()) {
            themeSet = theme;
            break;
        }
    }
    
    QIcon::setThemeName(themeSet);
    qDebug() << "DesktopView: Icon theme set to:" << QIcon::themeName() << "from search paths:" << QIcon::themeSearchPaths();
    
    // 确保QIcon图像提供者可用于QML
    // 这是image://icontheme/ URL正常工作所必需的
    if (QIcon::themeName().isEmpty()) {
        qWarning() << "DesktopView: No icon theme set! image://icontheme/ URLs will not work.";
    }

    engine()->rootContext()->setContextProperty("desktopView", this);
    engine()->rootContext()->setContextProperty("Dock", DockDBusInterface::self());
    // QWindow::fromWinId(winId())->setOpacity(0.99); // 这可能导致窗口显示异常
    engine()->addImageProvider("thumbnailer", new ThumbnailProvider());
    engine()->addImageProvider(QStringLiteral("icontheme"), new IconThemeProvider());

    setTitle(tr("Desktop"));
    setScreen(m_screen);
    setResizeMode(QQuickView::SizeRootObjectToView);

    onGeometryChanged();
    onPrimaryScreenChanged(QGuiApplication::primaryScreen());

    // 主屏改变
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &DesktopView::onPrimaryScreenChanged);

    connect(m_screen, &QScreen::virtualGeometryChanged, this, &DesktopView::onGeometryChanged);
    connect(m_screen, &QScreen::geometryChanged, this, &DesktopView::onGeometryChanged);
}

QRect DesktopView::screenRect()
{
    return m_screenRect;
}

void DesktopView::onPrimaryScreenChanged(QScreen *screen)
{
    bool isPrimaryScreen = m_screen->name() == screen->name();

    onGeometryChanged();

    setSource(isPrimaryScreen ? QStringLiteral("qrc:/qml/Desktop/Main.qml")
                              : QStringLiteral("qrc:/qml/Desktop/Wallpaper.qml"));
}

void DesktopView::onGeometryChanged()
{
    m_screenRect = m_screen->geometry().adjusted(0, 0, 1, 1);
    setGeometry(m_screenRect);
    emit screenRectChanged();
}
