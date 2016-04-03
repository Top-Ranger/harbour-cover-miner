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

CoverBackground {
    Column {
        anchors.centerIn: parent
        Label {
            text: "Cover Miner"
        }
        Label {
            id: busy
            text: "Not mining"
            color: Theme.secondaryHighlightColor
        }
    }
    Connections {
        target: generator
        onGeneration_started: {
            busy.color = Theme.highlightColor
            busy.text = "Currently mining!"
        }

        onGeneration_finished: {
            busy.color = Theme.secondaryHighlightColor
            busy.text = "Not mining"
        }
    }
}
