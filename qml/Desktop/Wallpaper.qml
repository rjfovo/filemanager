import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import QtQuick.Window 6.0
import Qt5Compat.GraphicalEffects 6.0

import Cutefish.FileManager 1.0 as FM
import FishUI 1.0 as FishUI

Item {
    id: control

    FM.DesktopSettings {
        id: settings
    }

    Loader {
        id: backgroundLoader
        anchors.fill: parent
        anchors.margins: 0
        sourceComponent: settings.backgroundType === 0 ? wallpaper : background
    }

    Component {
        id: background

        Rectangle {
            anchors.fill: parent
            visible: settings.backgroundVisible
            color: settings.backgroundColor
        }
    }

    Component {
        id: wallpaper

        Image {
            source: settings.wallpaper ? "file://" + settings.wallpaper : ""
            sourceSize: Qt.size(width * Screen.devicePixelRatio,
                                height * Screen.devicePixelRatio)
            fillMode: Image.PreserveAspectCrop
            clip: true
            visible: settings.backgroundVisible && source.toString() !== ""
            cache: false

            // Clear cache
            onSourceChanged: dirModel.clearPixmapCache()

            ColorOverlay {
                id: dimsWallpaper
                anchors.fill: parent
                source: parent
                color: "#000000"
                opacity: FishUI.Theme.darkMode && settings.dimsWallpaper ? 0.4 : 0.0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 200
                    }
                }

            }
        }
    }
}
