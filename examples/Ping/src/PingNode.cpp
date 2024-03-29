/**
 * SmartIot Node for Pings.
 *
 */
#include "PingNode.hpp"

PingNode::PingNode(const char* id, const char* name, const int measurementInterval) : SmartIotNode(id, name, "ping") {
  _measurementInterval = (measurementInterval > MIN_INTERVAL) ? measurementInterval : MIN_INTERVAL;
  _lastMeasurement     = 0;
}

/**
 *
 */
void PingNode::printCaption() {
  SmartIot.getLogger() << cCaption << endl;
}

/**
 * Handles the received MQTT messages from SmartIot.
 *
 */
bool PingNode::handleInput(const String& value) {

  printCaption();
  SmartIot.getLogger() << cIndent << "〽 handleInput value=" << value << endl;

  setProperty(cPing).send(cPong);

  return true;
}

/**
 *
 */
void PingNode::loop() {
  if (millis() - _lastMeasurement >= _measurementInterval * 1000UL || _lastMeasurement == 0) {
    _lastMeasurement = millis();

    SmartIot.getLogger() << "〽 Sending ping 'hello': " << getId() << endl;

    setProperty(cPing).send(cHello);
  }
}

/**
 *
 */
void PingNode::onReadyToOperate() {}

/**
 *
 */
void PingNode::setup() {
  printCaption();

  advertise(cPing).setName("ping").setRetained(true).setDatatype("string").settable();
}
