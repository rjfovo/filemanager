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

import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import QtQuick.Window 6.0
import Qt5Compat.GraphicalEffects 6.0

import Cutefish.FileManager 1.0 as FM
import FishUI 1.0 as FishUI
import "../"

Item {
    id: rootItem

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    // 调试：清空内容，二分排查 exec 立即返回问题
}


    Wallpaper {
        anchors.fill: parent
        // 分层桌面：壁纸由 cutefish-desktop-background 提供，本层只画图标
        visible: !desktopLayerOnly
    }

    FM.FolderModel {
        id: dirModel
        url: desktopPath()
        isDesktop: true
        sortMode: -1
        viewAdapter: viewAdapter

        onCurrentIndexChanged: {
            _folderView.currentIndex = dirModel.currentIndex
        }
    }

    FM.ItemViewAdapter {
        id: viewAdapter
        adapterView: _folderView
        adapterModel: dirModel
        adapterIconSize: 40
        adapterVisibleArea: Qt.rect(_folderView.contentX, _folderView.contentY,
                                    _folderView.contentWidth, _folderView.contentHeight)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: _folderView.forceActiveFocus()
    }

    FolderGridView {
        id: _folderView
        anchors.fill: parent

        isDesktopView: true
        iconSize: globalSettings.desktopIconSize
        maximumIconSize: globalSettings.maximumIconSize
        minimumIconSize: 22
        focus: true
        model: dirModel

        ScrollBar.vertical.policy: ScrollBar.AlwaysOff

        // Handle for topbar
        topMargin: 28

        // From dock
        leftMargin: Dock.leftMargin
        rightMargin: Dock.rightMargin
        bottomMargin: Dock.bottomMargin

        flow: GridView.FlowTopToBottom

        delegate: FolderGridItem {}

        onIconSizeChanged: {
            globalSettings.desktopIconSize = _folderView.iconSize
        }

        onActiveFocusChanged: {
            if (!activeFocus) {
                _folderView.cancelRename()
                dirModel.clearSelection()
            }
        }

        Component.onCompleted: {
            dirModel.requestRename.connect(rename)
        }
    }

    // 统一的桌面右键菜单（QML 菜单机制，与文件管理器共用同一组件）
    // 打开前调用 model.updateMenuState() 刷新动态可见性/启用状态
    FileContextMenu {
        id: desktopMenu
        model: dirModel
    }

    Connections {
        target: _folderView

        function onContextMenuRequested(x, y) {
            dirModel.updateMenuState()
            desktopMenu.popupAt(x, y)
        }
    }

    FM.ShortCut {
        id: shortCut

        Component.onCompleted: {
            shortCut.install(_folderView)
        }

        onOpen: {
            dirModel.openSelected()
        }
        onCopy: {
            dirModel.copy()
        }
        onCut: {
            dirModel.cut()
        }
        onPaste: {
            dirModel.paste()
        }
        onRename: {
            dirModel.requestRename()
        }
        onOpenPathEditor: {
            folderPage.requestPathEditor()
        }
        onSelectAll: {
            dirModel.selectAll()
        }
        onDeleteFile: {
            dirModel.keyDeletePress()
        }
        onKeyPressed: {
            dirModel.keyboardSearch(text)
        }
        onShowHidden: {
            dirModel.showHiddenFiles = !dirModel.showHiddenFiles
        }
        onUndo: {
            dirModel.undo()
        }
    }

    Component {
        id: rubberBandObject

        FM.RubberBand {
            id: rubberBand

            width: 0
            height: 0
            z: 99999
            color: FishUI.Theme.highlightColor

            function close() {
                opacityAnimation.restart()
            }

            OpacityAnimator {
                id: opacityAnimation
                target: rubberBand
                to: 0
                from: 1
                duration: 150

                easing {
                    bezierCurve: [0.4, 0.0, 1, 1]
                    type: Easing.Bezier
                }

                onFinished: {
                    rubberBand.visible = false
                    rubberBand.enabled = false
                    rubberBand.destroy()
                }
            }
        }
    }
}
