#include "sensor.h"
#include <QDebug>
#include <cmath>
#include <limits>
double Gyroscope::read() {
    return ready_ ? 1.25 : std::numeric_limits<double>::quiet_NaN();
}
bool Gyroscope::is_ready() const {
    return ready_;
}
QString Gyroscope::name() const {
    return "Gyroscope";
}
void Gyroscope::set_ready(bool ready) {
    ready_ = ready;
}
double Accelerometer::read() {
    return ready_ ? 9.81 : std::numeric_limits<double>::quiet_NaN();
}
bool Accelerometer::is_ready() const {
    return ready_;
}
QString Accelerometer::name() const {
    return "Accelerometer";
}
void Accelerometer::set_ready(bool ready) {
    ready_ = ready;
}
double Barometer::read() {
    return ready_ ? 1012.8 : std::numeric_limits<double>::quiet_NaN();
}
bool Barometer::is_ready() const {
    return ready_;
}
QString Barometer::name() const {
    return "Barometer";
}
void Barometer::set_ready(bool ready) {
    ready_ = ready;
}
bool check_sensors(const std::vector<Sensor*>& sensors) {
    qDebug().noquote() << "Checking sensors...";
    bool ready = !sensors.empty();
    for (Sensor* sensor : sensors) {
        if (!sensor) {
            qDebug().noquote() << "Error: sensor is not set";
            ready = false;
            continue;
        }
        if (!sensor->is_ready()) {
            qDebug().noquote() << sensor->name() + ": Not ready";
            ready = false;
            continue;
        }
        const double value = sensor->read();
        if (!std::isfinite(value)) {
            qDebug().noquote() << sensor->name() + ": Invalid value";
            ready = false;
        } else {
            qDebug().noquote() << sensor->name() + ": Ready, value=" + QString::number(value);
        }
    }
    qDebug().noquote() << (ready ? "All sensors are ready" : "sensor failure");
    return ready;
}
