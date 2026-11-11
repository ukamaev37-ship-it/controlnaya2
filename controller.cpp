#include "controller.h"
#include <cmath>
#include <limits>
SimpleController::SimpleController(double coefficient) : coefficient_(coefficient) {}
double SimpleController::calculate_control(double error, double dt) {
    if (!std::isfinite(error) || !std::isfinite(dt) || dt <= 0 || !std::isfinite(coefficient_))
        return std::numeric_limits<double>::quiet_NaN();
    return coefficient_ * error;
}
QString SimpleController::controller_name() const {
    return "Simple";
}
