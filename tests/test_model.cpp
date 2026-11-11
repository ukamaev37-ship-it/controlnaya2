#include "autopilot.h"
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>
static QStringList messages;
static void capture(QtMsgType, const QMessageLogContext&, const QString& message) {
    messages << message;
}
static void check(bool condition, const char* label) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        std::exit(1);
    }
}
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    qInstallMessageHandler(capture);
    static_assert(std::is_abstract<UAV>::value && std::has_virtual_destructor<UAV>::value);
    static_assert(std::is_abstract<Sensor>::value && std::has_virtual_destructor<Sensor>::value);
    static_assert(std::is_abstract<NavigationSystem>::value &&
                  std::has_virtual_destructor<NavigationSystem>::value);
    static_assert(std::is_abstract<FlightController>::value &&
                  std::has_virtual_destructor<FlightController>::value);
    Quadcopter quad;
    FixedWingUAV wing;
    run_flight_test(quad);
    run_flight_test(wing);
    check(messages.contains("Quadcopter: vertical takeoff") &&
              messages.contains("Quadcopter: vertical landing"),
          "quad flight");
    check(messages.contains("Fixed-wing UAV: runway takeoff") &&
              messages.contains("Fixed-wing UAV: gliding landing"),
          "wing flight");
    Gyroscope gyro;
    Accelerometer accel;
    Barometer baro;
    std::vector<Sensor*> sensors{&gyro, &accel, &baro};
    check(gyro.read() == 1.25 && accel.read() == 9.81 && baro.read() == 1012.8, "sensor readings");
    check(check_sensors(sensors), "sensor ready");
    gyro.set_ready(false);
    check(!check_sensors(sensors), "sensor failure");
    gyro.set_ready(true);
    check(!check_sensors({nullptr}) && !check_sensors({}), "missing sensors");
    GPSNavigation gps;
    VisualNavigation visual;
    SimpleController controller;
    gps.set_satellites(3);
    check(!gps.is_valid(), "three satellites invalid");
    gps.set_satellites(4);
    check(gps.is_valid(), "four satellites valid");
    gps.set_valid(false);
    check(!gps.is_valid(), "GPS invalid flag");
    gps.set_valid(true);
    visual.set_target_tracked(false);
    check(!visual.is_valid(), "target lost");
    print_navigation_status(visual);
    check(messages.contains("Status: Invalid"), "invalid navigation output");
    visual.set_target_tracked(true);
    print_navigation_status(gps);
    check(messages.contains("Position: 55.75, 37.61, 150") && messages.contains("Status: Valid"),
          "navigation output");
    Autopilot pilot(&quad, sensors, &gps, &controller);
    pilot.run_cycle();
    check(pilot.last_result().success && pilot.last_result().error == 50 &&
              pilot.last_result().output == 25,
          "normal cycle");
    check(messages.contains("Controller output: 25"), "normal output");
    baro.set_ready(false);
    pilot.run_cycle();
    check(!pilot.last_result().success && pilot.last_result().message == "Reason: sensor failure",
          "scenario 2");
    baro.set_ready(true);
    gps.set_valid(false);
    pilot.run_cycle();
    check(pilot.last_result().message == "Navigation data invalid", "scenario 3");
    gps.set_valid(true);
    Autopilot noController(&quad, sensors, &gps, nullptr);
    noController.run_cycle();
    check(noController.last_result().message == "Error: flight controller is not set",
          "scenario 4");
    gps.set_position({55.75, 37.61, -1});
    pilot.run_cycle();
    check(pilot.last_result().message == "Error: invalid navigation position", "scenario 5");
    for (Position p : {Position{91, 0, 150}, Position{0, 181, 150},
                       Position{0, 0, std::numeric_limits<double>::infinity()},
                       Position{std::numeric_limits<double>::quiet_NaN(), 0, 150}}) {
        gps.set_position(p);
        pilot.run_cycle();
        check(!pilot.last_result().success, "invalid coordinates");
    }
    gps.set_position({55.75, 37.61, 150});
    pilot.run_cycle();
    check(pilot.last_result().success, "recovery");
    Autopilot noVehicle(nullptr, sensors, &gps, &controller);
    noVehicle.run_cycle();
    check(noVehicle.last_result().message == "Error: vehicle is not set", "null vehicle");
    Autopilot noNav(&quad, sensors, nullptr, &controller);
    noNav.run_cycle();
    check(noNav.last_result().message == "Error: navigation is not set", "null navigation");
    Autopilot noSensor(&quad, {nullptr}, &gps, &controller);
    noSensor.run_cycle();
    check(noSensor.last_result().message == "Reason: sensor failure", "null sensor");
    Autopilot visualPilot(&wing, sensors, &visual, &controller);
    visualPilot.run_cycle();
    check(visualPilot.last_result().success, "replaceable modules");
    check(std::isnan(controller.calculate_control(50, 0)), "invalid dt");
    qInstallMessageHandler(nullptr);
    qInfo() << "Model tests passed: all five required scenarios, polymorphism, boundaries, nulls, "
               "recovery.";
    return 0;
}
