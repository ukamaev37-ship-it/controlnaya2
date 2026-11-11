#pragma once
#include "controller.h"
#include "navigation.h"
#include "sensor.h"
#include "uav.h"
struct CycleResult {
    bool success = false;
    QString message;
    double altitude = 0, error = 0, output = 0;
};
class Autopilot {
  public:
    // Non-owning pointers: modules must outlive this autopilot.
    Autopilot(UAV* vehicle, const std::vector<Sensor*>& sensors, NavigationSystem* navigation,
              FlightController* controller);
    void run_cycle();
    const CycleResult& last_result() const;

  private:
    UAV* vehicle_;
    std::vector<Sensor*> sensors_;
    NavigationSystem* navigation_;
    FlightController* controller_;
    CycleResult result_;
    void abort_cycle(const QString& message);
};
