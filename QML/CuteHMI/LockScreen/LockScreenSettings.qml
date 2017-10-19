import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Qt.labs.settings 1.0

import CuteHMI.App 1.0
import CuteHMI.alpha.Controls 1.0

Item {
	id: root

	property alias lockScreenComponent: wizard.lockScreenComponent
	Frame {
		anchors.centerIn: parent
		padding: 20.0

		property alias changePasswordButton: changePasswordButton

		GridLayout {
			id: gridLayout
			rowSpacing: 40.0
			columnSpacing: 30.0
			columns: 2

			Label {
				text: qsTr("Ekran blokady:")
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}

			Switch {
				id: lockScreenSwitch

				Settings {
					id: checkedSettings
					category: "cutehmi_lockscreen_1"
					property alias activated: lockScreenSwitch.checked
				}

				Binding {
					target: CuteApp
					property: "idleMeasureEnabled"
					value: checkedSettings.activated
				}
			}

			Label {
				text: qsTr("Czas włączania blokady: ")
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}

			RealSpinBox {
				id: lockScreenTimeout
				from: 10
				to: 600
				value: 10
				stepSize: 10
				suffix: " s"

				Settings {
					category: "cutehmi_lockscreen_1"
					property alias timeout: lockScreenTimeout.value
				}
			}

			Label {
				text: qsTr("Zmiana hasła: ")
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
			}

			Button {
				id: changePasswordButton
				text: qsTr("zmień...")
				onClicked: wizardPopup.open()
			}
		}
	}

	Popup {
		id: wizardPopup

		x: 10
		y: 10
		width: parent.width - 2 * x
		height: parent.height - 2 * y
		closePolicy: Popup.NoAutoClose
		modal: true

		ChangePasswordWizard {
			id: wizard

			anchors.fill: parent
			lockScreenComponent: root.lockScreenComponent
			onFinished: wizardPopup.close()
		}
	}
}
