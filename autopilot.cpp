#include "autopilot.h"
#include <QDebug>
#include <cmath>
Autopilot::Autopilot(UAV* vehicle, const std::vector<Sensor*>& sensors,
                     NavigationSystem* navigation, FlightController* controller)
    : vehicle_(vehicle), sensors_(sensors), navigation_(navigation), controller_(controller) {}
const CycleResult& Autopilot::last_result() const {
    return result_;
}
void Autopilot::abort_cycle(const QString& message) {
    result_.message = message;
    qDebug().noquote() << "Autopilot cycle aborted";
    qDebug().noquote() << message;
}
void Autopilot::run_cycle() {
    result_ = {};
    qDebug().noquote() << "Autopilot cycle started";
    if (!vehicle_) {
        abort_cycle("Error: vehicle is not set");
        return;
    }
    if (!check_sensors(sensors_)) {
        abort_cycle("Reason: sensor failure");
        return;
    }
    if (!navigation_) {
        abort_cycle("Error: navigation is not set");
        return;
    }
    if (!navigation_->is_valid()) {
        abort_cycle("Navigation data invalid");
        return;
    }
    const Position position = navigation_->get_position();
    if (!valid_position(position)) {
        abort_cycle("Error: invalid navigation position");
        return;
    }
    if (!controller_) {
        abort_cycle("Error: flight controller is not set");
        return;
    }
    result_.altitude = position.altitude;
    result_.error = 200.0 - position.altitude;
    result_.output = controller_->calculate_control(result_.error, 0.1);
    if (!std::isfinite(result_.output)) {
        abort_cycle("Error: invalid controller output");
        return;
    }
    qDebug().noquote() << "Navigation:" << navigation_->system_name();
    qDebug().noquote() << "Current altitude:" << result_.altitude << "m";
    qDebug().noquote() << "Target altitude: 200 m";
    qDebug().noquote() << "Altitude error:" << result_.error << "m";
    qDebug().noquote() << "Controller output:" << result_.output;
    result_.success = true;
    result_.message = "Autopilot cycle completed";
    qDebug().noquote() << result_.message;
}
