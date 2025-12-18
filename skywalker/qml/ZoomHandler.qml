import QtQuick

PinchHandler {
    signal released

    target: null
    rotationAxis.enabled: false
    xAxis.enabled: false
    yAxis.enabled: false

    onGrabChanged: (transition, point) => {
        if (transition === PointerDevice.UngrabPassive)
            released()
    }
}
