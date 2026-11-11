#pragma once
#include "controller.h"
#include "navigation.h"
#include "sensor.h"
#include "uav.h"
#include <QMainWindow>
class LabView;
class QLabel;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QPlainTextEdit;
class QTimer;
class QStackedWidget;
class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit MainWindow(QWidget* parent = nullptr);
    void appendLog(const QString& text);
  public slots:
    void tick();

  private:
    void selectMode(int mode);
    void run();
    void reset();
    void syncView();
    void setRunning(bool running);
    LabView* view_;
    QLabel *title_, *subtitle_, *status_, *result_, *step_;
    QComboBox *vehicle_, *navigation_, *scenario_;
    QSpinBox* satellites_;
    QDoubleSpinBox* altitude_;
    QCheckBox *sensorReady_, *gpsValid_, *tracked_, *motion_;
    QPushButton *start_, *pause_;
    QPlainTextEdit* log_;
    QStackedWidget* settings_;
    QTimer* timer_;
    int mode_ = 0;
    double progress_ = 0;
    double initialAltitude_ = 150;
    bool running_ = false;
    Quadcopter quad_;
    FixedWingUAV wing_;
    Gyroscope gyro_;
    Accelerometer accelerometer_;
    Barometer barometer_;
    GPSNavigation gps_;
    VisualNavigation visual_;
    SimpleController controller_;
};
