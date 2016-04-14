import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

Rectangle {
    id: root

    Component.onCompleted: {
        //ctor
        sldVolume.value = m_device.GetVolume();
        chkFFT.checked = m_device.GetFFT();
    }

    Rectangle {
        property int d: 100
        id: square
        width: d
        height: d
        anchors.centerIn: parent
        color: "red"
        NumberAnimation on rotation { from: 0; to: 360; duration: 2000; loops: Animation.Infinite; }
    }

    Text {
        anchors.centerIn: parent
        text: "Qt Quick running in a widget"
    }

    ColumnLayout {
            id: buttonLayout
            //anchors.fill: parent
            //anchors.margins: margin
            anchors.bottom: parent

            Button {
                   id: btnStart
                   text: "Start"

                   onClicked: {
                       console.log(btnStart.text + " clicked")
                       m_device.Start();
                   }
            }
            Button {
                   id: btnStop
                   text: "Stop"
                   Layout.minimumHeight: 30
                   Layout.fillWidth: true

                   onClicked: {
                       console.log(btnStop.text + " clicked")
                       m_device.Stop();
                   }
            }
            RowLayout
            {
                Label {
                    id: lblVolume
                    text: "Volume"
                }
                Slider {
                     id: sldVolume

                     Layout.fillWidth: true

                     minimumValue: 0
                     maximumValue: 1000
                     stepSize: 10
                     updateValueWhileDragging: true
                }
                CheckBox {
                    id: chkFFT
                    text: "FFT"

                    onCheckedChanged: {
                        //m_device.GetBuffer(as)[]
                        m_device.SetCalcFFT(chkFFT.checked);
                    }
                }

            }
    }
}
