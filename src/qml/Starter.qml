/*
Copyright (C) 2016 Marcus Soll

The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQuick.Window 2.0

Page {
    id: page
    allowedOrientations: Orientation.All

    Connections {
        target: generator
        onGeneration_finished: functions.finished()
    }

    Item {
        id: functions
        function finished() {
            start_button.enabled = true
            busy_indicator.running = false
            abort_button.enabled = false
            home.enabled = true
            sd.enabled = true
        }

        function start_mining() {
            generator.clear_list()
            if(home.checked) {
                generator.add_path(StandardPaths.music)
            }
            if(sd.checked) {
                generator.add_path("/media/sdcard/")
            }
            start_button.enabled = false
            busy_indicator.running = true
            abort_button.enabled = true
            home.enabled = false
            sd.enabled = false
            generator.start()

        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("about.qml"))
            }

            MenuItem {
                text: qsTr("Remove all covers")
                onClicked: remorsePopup.execute("Remove all covers", function() { generator.clear_covers() } )
            }
        }

        Column {
            anchors {
                left: parent.left
                right: parent.right
                margins: Theme.paddingSmall
            }

            id: column
            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: "Cover Miner"
            }

            Label {
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.primaryColor
                text: "Select folders for mining:"
            }

            TextSwitch {
                id: home
                text: "Home music folder"
            }

            TextSwitch {
                id: sd
                text: "SD card"
            }

            Button {
                id: start_button
                width: parent.width
                text: "Start mining"
                enabled: true
                onClicked: functions.start_mining()
            }

            Item {
                width: Screen.width
                height: busy_indicator.height
                BusyIndicator {
                    id: busy_indicator
                    size: BusyIndicatorSize.Large
                    running: false
                    anchors.centerIn: parent
                }
            }

            Button {
                id: abort_button
                width: parent.width
                text: "Abort"
                enabled: false
                onClicked: generator.abort()
            }
        }
    }

    RemorsePopup {
        id: remorsePopup
    }
}
