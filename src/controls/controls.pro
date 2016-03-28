TEMPLATE = lib
QT += qml quick

uri = Machinekit.Controls
include(../plugin.pri)

# Input
SOURCES +=

HEADERS += \
    plugin.h

SOURCES += \
    plugin.cpp

RESOURCES += \
    controls.qrc

QML_FILES = \
    Dial.qml \
    Gauge.qml \
    ColorPicker.qml \
    Knob.qml \
    Led.qml \
    LogChart.qml \
    RoundGauge.qml \
    SlideView.qml \
    SlidePage.qml \
    TemperatureSelector.qml \
    TooltipArea.qml \
    TouchButton.qml \
    ValueChart.qml \
    ValueModel.qml \
    VirtualJoystick.qml

include(Private/private.pri)

QML_INFRA_FILES = \
    $$QML_FILES \
    qmldir

QML_DESIGNER_FILES = \
    designer

QML_PROPERTY_EDITOR_FILES = \
    propertyEditorQmlSources/DialSpecifics.qml \
    propertyEditorQmlSources/DialSection.qml \
    propertyEditorQmlSources/GaugeSpecifics.qml \
    propertyEditorQmlSources/GaugeSection.qml \
    propertyEditorQmlSources/TextSection.qml \
    propertyEditorQmlSources/ColorPickerSection.qml \
    propertyEditorQmlSources/ColorPickerSpecifics.qml \
    propertyEditorQmlSources/LedSpecifics.qml \
    propertyEditorQmlSources/LedSection.qml \
    propertyEditorQmlSources/LogChartSection.qml \
    propertyEditorQmlSources/LogChartSpecifics.qml \
    propertyEditorQmlSources/RoundGaugeSection.qml \
    propertyEditorQmlSources/RoundGaugeSpecifics.qml \
    propertyEditorQmlSources/SmallTextSection.qml \
    propertyEditorQmlSources/ValueChartSection.qml \
    propertyEditorQmlSources/ValueChartSpecifics.qml \
    propertyEditorQmlSources/VirtualJoystickSection.qml \
    propertyEditorQmlSources/VirtualJoystickSpecifics.qml

include(../deployment.pri)
