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

Page {
    id: about_page

    allowedOrientations: Orientation.All

    SilicaFlickable {

        VerticalScrollDecorator {}

        PullDownMenu {
            visible: false
        }

        anchors.fill: parent

        contentHeight: column.height

        Column {
            id: column

            anchors {
                left: parent.left
                right: parent.right
                margins: Theme.paddingLarge
            }

            PageHeader {
                title: "About"
            }

            Label {
                text: "Cover Miner 1.0"
            }

            Text  {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingSmall
                }

                focus: true
                color: Theme.primaryColor
                width: about_page.width
                font.pixelSize: Theme.fontSizeExtraSmall
                wrapMode: Text.Wrap

                text: "Copyright (C) 2016 Marcus Soll

The contents of this file are subject to the Mozilla Public License Version 1.1 (the \"License\"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an \"AS IS\" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License."
            }

            PageHeader {
                title: "About TagLib"
            }

            Text  {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.paddingSmall
                }

                focus: true
                color: Theme.primaryColor
                width: about_page.width
                font.pixelSize: Theme.fontSizeExtraSmall
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally("https://taglib.github.io/")

                text: "<html>This application uses TagLib.

TagLib is distributed under the GNU Lesser General Public License (LGPL) and/or Mozilla Public License (MPL). For more information see: <a href=\"https://taglib.github.io/\">https://taglib.github.io/</a></html>"
            }
        }
    }
}

