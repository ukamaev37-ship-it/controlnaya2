#include "autopilot.h"
#include "mainwindow.h"
#include <QApplication>
#include <QPointer>
#include <cstdio>
static QPointer<MainWindow> window;
static void messages(QtMsgType, const QMessageLogContext&, const QString& message) {
    const auto bytes = message.toLocal8Bit();
    std::fprintf(stderr, "%s\n", bytes.constData());
    if (window)
        window->appendLog(message);
}
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("Полётная лаборатория · Контрольная 02");
    app.setStyle("Fusion");
    if (app.arguments().contains("--console")) {
        Quadcopter quad;
        FixedWingUAV wing;
        run_flight_test(quad);
        run_flight_test(wing);
        Gyroscope gyro;
        Accelerometer accel;
        Barometer baro;
        std::vector<Sensor*> sensors{&gyro, &accel, &baro};
        check_sensors(sensors);
        gyro.set_ready(false);
        check_sensors(sensors);
        gyro.set_ready(true);
        GPSNavigation gps;
        VisualNavigation visual;
        print_navigation_status(gps);
        gps.set_valid(false);
        print_navigation_status(gps);
        gps.set_valid(true);
        print_navigation_status(visual);
        visual.set_target_tracked(false);
        print_navigation_status(visual);
        SimpleController controller;
        Autopilot ok(&quad, sensors, &gps, &controller);
        ok.run_cycle();
        gyro.set_ready(false);
        ok.run_cycle();
        gyro.set_ready(true);
        gps.set_valid(false);
        ok.run_cycle();
        gps.set_valid(true);
        Autopilot missing(&quad, sensors, &gps, nullptr);
        missing.run_cycle();
        gps.set_position({55.75, 37.61, -1});
        ok.run_cycle();
        return 0;
    }
    MainWindow mainWindow;
    window = &mainWindow;
    const auto previous = qInstallMessageHandler(messages);
    mainWindow.show();
    const int result = app.exec();
    qInstallMessageHandler(previous);
    window.clear();
    return result;
}
