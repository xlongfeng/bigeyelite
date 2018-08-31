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
    id: procedure

    title: qsTr("Home")

    states: [
        State {
            name: "start"
        },
        State {
            name: "stop"
        }
    ]

    state: testCase.running ? "start" : "stop"

    AutoOnOffTest {
        id: testCase
    }

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
                enabled: procedure.state == "stop"

                label: Text {
                    font.pointSize: 12
                    text: qsTr("Power Off Duration")
                    leftPadding: 8
                }

                Column {
                    spacing: 4

                    LabelSpinBox {
                        id: powerOffDay
                        text: qsTr("Day")
                        from: 0
                        to: 15
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        id: powerOffHour
                        text: qsTr("Hour")
                        from: 0
                        to: 23
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        id: powerOffMinute
                        text: qsTr("Minute")
                        from: 0
                        to: 59
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        id: powerOffSecond
                        text: qsTr("Second")
                        from: 0
                        to: 59
                        stepSize: 1
                        value: 20
                    }
                }
            }

            GroupBox {
                enabled: procedure.state == "stop"

                label: Text {
                    font.pointSize: 12
                    text: qsTr("Power On Duration")
                    leftPadding: 4
                }

                Column {
                    spacing: 8

                    LabelSpinBox {
                        id: powerOnDay
                        text: qsTr("Day")
                        from: 0
                        to: 15
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        id: powerOnHour
                        text: qsTr("Hour")
                        from: 0
                        to: 23
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        id: powerOnMinute
                        text: qsTr("Minute")
                        from: 0
                        to: 59
                        stepSize: 1
                        value: 0
                    }

                    LabelSpinBox {
                        id: powerOnSecond
                        text: qsTr("Second")
                        from: 0
                        to: 59
                        stepSize: 1
                        value: 30
                    }
                }
            }

            Button {
                id: startButton
                enabled: {
                    BigeyeLite.linkStatus == BigeyeLite.Connected
                    &&  procedure.state == "stop"
                }
                text: qsTr("Start")
                width: parent.width
                onPressed: {
                    testCase.setPowerOffDuration(powerOffDay.value, powerOffHour.value, powerOffMinute.value, powerOffSecond.value)
                    testCase.setPowerOnDuration(powerOnDay.value, powerOnHour.value, powerOnMinute.value, powerOnSecond.value)
                    testCase.start()
                }
            }

            Button {
                id: stopButton
                enabled: {
                    BigeyeLite.linkStatus == BigeyeLite.Connected
                    && procedure.state == "start"
                }
                text: qsTr("Stop")
                width: parent.width
                onPressed: {
                    testCase.stop()
                }
            }
        }
    }

    Item {
        anchors.left: controlPanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 8

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            LabelTextField {
                label: qsTr("Power Off Countdown")
                text: testCase.powerOffCountdown
            }

            LabelTextField {
                label: qsTr("Power On Countdown")
                text: testCase.powerOnCountdown
            }

            LabelTextField {
                label: qsTr("Power On Count")
                text: testCase.powerOnCount
            }

            ProgressBar {
                indeterminate: true
                Layout.fillWidth: true
            }

            ListView {
                id: logView
                model: LogModel { onRowsInserted: logView.positionViewAtEnd() }
                delegate: RowLayout {
                    property var textColor: {
                        if (level == BigeyeLite.LoggerInfo)
                            return "green"
                        else if (level == BigeyeLite.LoggerFatal)
                            return "red"
                        else
                            return "black"
                    }
                    Text {
                        color: textColor
                        text: datetime
                    }
                    Text {
                        color: textColor
                        text: message
                    }
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
