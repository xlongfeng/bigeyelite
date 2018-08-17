/*
 * Bigeye Lite - Accessorial Tool for Daily Test
 *
 * Copyright (c) 2018, longfeng.xiao <xlongfeng@126.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.10
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import Backend 1.0

Page {
    title: qsTr("Home")

    Flickable {
        id: controlPanel

        width: option.implicitWidth
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        contentWidth: option.implicitWidth
        contentHeight: option.implicitHeight

        Column {
            id: option
            spacing: 4
            anchors.left: parent.left

            GroupBox {
                id: shutdown

                label: Text {
                    font.pointSize: 12
                    text: qsTr("Shutdown Duration")
                    leftPadding: 8
                }

                Column {
                    spacing: 4

                    LabelSpinBox {
                        text: qsTr("Day")
                        from: 0
                        to: 30
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        text: qsTr("Hour")
                        from: 0
                        to: 24
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        text: qsTr("Minute")
                        from: 0
                        to: 60
                        stepSize: 1
                        value: 5
                    }

                    LabelSpinBox {
                        text: qsTr("Second")
                        from: 0
                        to: 60
                        stepSize: 1
                        value: 0
                    }
                }
            }

            GroupBox {
                id: running

                label: Text {
                    font.pointSize: 12
                    text: qsTr("Running Duration")
                    leftPadding: 4
                }

                Column {
                    spacing: 8

                    LabelSpinBox {
                        text: qsTr("Day")
                        from: 0
                        to: 30
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        text: qsTr("Hour")
                        from: 0
                        to: 24
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        text: qsTr("Minute")
                        from: 0
                        to: 60
                        stepSize: 1
                        value: 5
                    }

                    LabelSpinBox {
                        text: qsTr("Second")
                        from: 0
                        to: 60
                        stepSize: 1
                        value: 0
                    }
                }
            }

            Button {
                text: qsTr("Start")
                width: parent.width
                onPressed: BigeyeLite.start()
            }
        }
    }

    Frame {
        anchors.left: controlPanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 8

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            LabelTextField {
                text: qsTr("Shutdown Countdown")
            }

            LabelTextField {
                text: qsTr("Running Countdown")
            }

            LabelTextField {
                text: qsTr("Powerup Counter")
            }

            Rectangle {
                color: "gray"
                Layout.preferredHeight: 1
                Layout.fillWidth: true
            }

            ListView {
                model: LogModel { }
                delegate: Text {
                    text: display
                }

                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    BusyIndicator {
        running: BigeyeLite.linkStatus == BigeyeLite.Disconnected
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 32
    }
}
