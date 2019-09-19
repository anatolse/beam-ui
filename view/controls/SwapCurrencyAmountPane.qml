import QtQuick 2.11
import QtQuick.Layouts 1.3

Rectangle {
    property color borderColor: Style.swapCurrencyOptionsBorder
    property int borderSize: 0
    property color gradLeft: Style.swapCurrencyPaneGrLeftBEAM
    property color gradRight: Style.swapCurrencyPaneGrRight
    property string currencyIcon: ""
    property var currencyIcons: []
    property color stateIndicatorColor: Style.swapCurrencyStateIndicator
    property string valueStr: ""
    property bool isOk: true
    property int textSize: 16
    property color textColor: Style.content_main
    property var onClick: function() {}

    Layout.fillWidth: true
    height: 67
    radius: 10
    color: Style.background_main

    Rectangle {
        anchors.centerIn: parent
        width: parent.height
        height: parent.width
        radius: 10
        rotation: -90
        opacity: 0.3
        gradient: Gradient {
            GradientStop { position: 0.0; color: gradLeft }
            GradientStop { position: 1.0; color: gradRight }
        }
        border {
            width: borderSize
            color: borderColor
        }
    }
    Item {
        anchors.fill: parent
        anchors.margins: {
            left: 20
            right: 10
        }

        Item {
            id: currencyLogo
            width: childrenRect.width
            anchors.verticalCenter: parent.verticalCenter
            Image {
                anchors.verticalCenter: parent.verticalCenter
                source: currencyIcon
                visible: currencyIcon.length
            }

            Repeater {
                model: currencyIcons.length
                visible: currencyIcons.length
                
                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    x: parent.x + index * 15
                    source: currencyIcons[index]
                }
            }
        }

        SFText {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 20 + currencyLogo.width
            font.pixelSize: textSize
            color: textColor
            elide: Text.ElideRight
            text: valueStr
            visible: isOk
        }

        SFText {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 5 + currencyLogo.width
            rightPadding: 5
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            color: Style.validator_error
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            //% "Cannot connect to peer. Please check in the address in settings and retry."
            text: qsTrId("swap-beta-connection-error")
            visible: !isOk

            Component.onCompleted: {
                console.log(anchors.margins);
            }
        }

        Item {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: childrenRect.width
            visible: !isOk

            property int radius: 5

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left

                width: parent.radius * 2
                height: parent.radius * 2
                radius: parent.radius
                color: stateIndicatorColor
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: {
            if (onClick && typeof onClick === 'function') {
                onClick();
            }
        }
    }
}
