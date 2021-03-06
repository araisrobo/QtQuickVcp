/****************************************************************************
**
** Copyright (C) 2014 Alexander Rössler
** License: LGPL version 2.1
**
** This file is part of QtQuickVcp.
**
** All rights reserved. This program and the accompanying materials
** are made available under the terms of the GNU Lesser General Public License
** (LGPL) version 2.1 which accompanies this distribution, and is available at
** http://www.gnu.org/licenses/lgpl-2.1.html
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
**
** Contributors:
** Alexander Rössler @ The Cool Tool GmbH <mail DOT aroessler AT gmail DOT com>
**
****************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

Item {
    property alias model: object.gcodeProgramModel
    property color selectedColor: "lightblue"
    property color activeColor: "lightcoral"
    property color executedColor: "lightgreen"
    property alias font: dummyLabel.font

    id: root

    SystemPalette { id: systemPalette }

    PathViewObject {
        id: object
        // onGcodeEditModeChanged: {
        //     if ((!object.gcodeEditMode) && (edit.text !== object.gcodeProgramLoader.text)) {
        //         // switching from edit-mode to read-mode
        //         console.log("TODO: confirm before saving")
        //         object.gcodeProgramLoader.save(edit.text)
        //         object.core.file.startUpload()
        //     }
        // }
    }

    Rectangle {
        id: lineNumberBackground
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: dummyLabel.width * (Math.ceil(Math.log(listView.count) / Math.LN10) + 1)  // adapting width to line numbers
        color: systemPalette.window
    }

    Rectangle {
        id: normalBackground
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: lineNumberBackground.right
        anchors.right: parent.right
        color: systemPalette.light
    }

    Label {
        id: dummyLabel
        text: "0"
        visible: false
    }

//    Rectangle {
//        id: gcodeEdit
//        visible: object.gcodeEditMode
//        color: "white"
//        anchors.fill: parent
//
//        Flickable {
//            id: flick
//
//            anchors.fill: parent
//            contentWidth: edit.paintedWidth
//            contentHeight: edit.paintedHeight
//            interactive: true
//            clip: true
//
//            function ensureVisible(r) {
//                if (contentX >= r.x)
//                    contentX = r.x;
//                else if (contentX+width <= r.x+r.width)
//                    contentX = r.x+r.width-width;
//                if (contentY >= r.y)
//                    contentY = r.y;
//                else if (contentY+height <= r.y+r.height)
//                    contentY = r.y+r.height-height;
//            }
//
//            TextEdit {
//                id: edit
//                text: object.gcodeProgramLoader.text
//
//                color: "#2B2C2E"
//                cursorVisible: activeFocus
//                width: flick.width
//                height: flick.height
//                focus: true
//                wrapMode: TextEdit.Wrap
//                textFormat: TextEdit.PlainText
//                onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
//                selectionColor: Qt.rgba(1.0, 1.0, 0.0, 0.75)
//                selectedTextColor: Qt.rgba(0.0, 0.0, 0.0, 0.8)
//                selectByMouse: true
//            }
//        }
//    }

    ScrollView {
        id: gcodeScroll
        visible: !object.gcodeEditMode
        frameVisible: true
        anchors.fill: parent

        ListView {
            id: listView
            anchors.fill: parent
            model: object.gcodeProgramModel
            delegate:
                Item {
                    property bool lineActive: Boolean(active)

                    anchors.left: parent ? parent.left : undefined
                    anchors.right: parent ? parent.right : undefined
                    height: dummyLabel.height

                    /*onLineActiveChanged: {
                        if (lineActive) {
                            listView.positionViewAtIndex(index, ListView.Center)
                        }
                    }*/

                    Item {
                        id: lineNumberRect
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: lineNumberBackground.width

                        Label {
                            anchors.fill: parent
                            anchors.rightMargin: 5
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            horizontalAlignment: Text.AlignRight
                            color: selected ? "white" : label.color
                            font: dummyLabel.font
                            text: String(lineNumber)
                        }
                    }

                    Rectangle {
                        color: selected ? root.selectedColor : (active ? root.activeColor : (executed ? root.executedColor : "transparent"))
                        anchors.left: lineNumberRect.right
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom


                        Label {
                            id: label
                            anchors.fill: parent
                            anchors.leftMargin: 5
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            font: dummyLabel.font
                            text: String(gcode).trim()
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: selected = !selected
                    }
            }
        }
    }
}
