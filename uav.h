#pragma once
#include <QString>
class UAV {
  public:
    virtual void takeoff() = 0;
    virtual void land() = 0;
    virtual QString type() const = 0;
    virtual ~UAV() = default;
};
class Quadcopter final : public UAV {
  public:
    void takeoff() override;
    void land() override;
    QString type() const override;
};
class FixedWingUAV final : public UAV {
  public:
    void takeoff() override;
    void land() override;
    QString type() const override;
};
void run_flight_test(UAV& vehicle);
