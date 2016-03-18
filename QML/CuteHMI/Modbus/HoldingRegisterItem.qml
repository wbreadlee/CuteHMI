import QtQuick 2.0
import CuteHMI.Modbus 1.0

Item
{
	property var device
	property int address
	property int encoding: ModbusHoldingRegister.INT16

	visible: false
	implicitWidth: 50
	implicitHeight: 50
	anchors.verticalCenter: parent.verticalCenter
	anchors.horizontalCenter: parent.horizontalCenter

	Component.onCompleted : {
		if (parent.valueChanged !== undefined)
			parent.valueChanged.connect(requestValue)
		device.r[address].valueUpdated.connect(updateValue)
		parent.enabled = false
	}

	Component.onDestruction: {
		device.r[address].valueUpdated.disconnect(updateValue)
	}

	function requestValue()
	{
		parent.enabled = false
		device.r[address].requestValue(parent.value, encoding)
	}

	function updateValue()
	{
		// Value may get updated right after doing request to a different value or it may get updated by a different controller.
		// This may cause subsequent emission of valueChanged signal, so disconnect it temporarily.
		if (parent.valueChanged !== undefined)
			parent.valueChanged.disconnect(requestValue)
		parent.value = device.r[address].value(encoding)
		if (parent.valueChanged !== undefined)
			parent.valueChanged.connect(requestValue)
		parent.enabled = true
	}
}

