#include "uav.h"
#include <QDebug>
void Quadcopter::takeoff() {
    qDebug().noquote() << "Quadcopter: vertical takeoff";
}
void Quadcopter::land() {
    qDebug().noquote() << "Quadcopter: vertical landing";
}
QString Quadcopter::type() const {
    return "Quadcopter";
}
void FixedWingUAV::takeoff() {
    qDebug().noquote() << "Fixed-wing UAV: runway takeoff";
}
void FixedWingUAV::land() {
    qDebug().noquote() << "Fixed-wing UAV: gliding landing";
}
QString FixedWingUAV::type() const {
    return "Fixed-wing UAV";
}
void run_flight_test(UAV& vehicle) {
    qDebug().noquote() << "Vehicle:" << vehicle.type();
    vehicle.takeoff();
    vehicle.land();
    qDebug().noquote() << "Flight test completed";
}
