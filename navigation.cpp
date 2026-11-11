#include "navigation.h"
#include <QDebug>
#include <cmath>
Position GPSNavigation::get_position() const {
    return position_;
}
bool GPSNavigation::is_valid() const {
    return satellites_ >= 4 && valid_;
}
QString GPSNavigation::system_name() const {
    return "GPS";
}
void GPSNavigation::set_position(const Position& position) {
    position_ = position;
}
void GPSNavigation::set_satellites(int satellites) {
    satellites_ = satellites;
}
void GPSNavigation::set_valid(bool valid) {
    valid_ = valid;
}
int GPSNavigation::satellites() const {
    return satellites_;
}
Position VisualNavigation::get_position() const {
    return position_;
}
bool VisualNavigation::is_valid() const {
    return target_tracked_;
}
QString VisualNavigation::system_name() const {
    return "Visual";
}
void VisualNavigation::set_position(const Position& position) {
    position_ = position;
}
void VisualNavigation::set_target_tracked(bool tracked) {
    target_tracked_ = tracked;
}
bool valid_position(const Position& p) {
    return std::isfinite(p.latitude) && std::isfinite(p.longitude) && std::isfinite(p.altitude) &&
           p.latitude >= -90 && p.latitude <= 90 && p.longitude >= -180 && p.longitude <= 180 &&
           p.altitude >= 0;
}
void print_navigation_status(const NavigationSystem& navigation) {
    qDebug().noquote() << "Navigation system:" << navigation.system_name();
    const Position p = navigation.get_position();
    qDebug().noquote()
        << QString("Position: %1, %2, %3").arg(p.latitude).arg(p.longitude).arg(p.altitude);
    qDebug().noquote() << "Status:" << (navigation.is_valid() ? "Valid" : "Invalid");
}
