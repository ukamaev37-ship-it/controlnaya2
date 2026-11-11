#pragma once
#include <QString>
struct Position {
    double latitude;
    double longitude;
    double altitude;
};
class NavigationSystem {
  public:
    virtual Position get_position() const = 0;
    virtual bool is_valid() const = 0;
    virtual QString system_name() const = 0;
    virtual ~NavigationSystem() = default;
};
class GPSNavigation final : public NavigationSystem {
  public:
    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;
    void set_position(const Position& position);
    void set_satellites(int satellites);
    void set_valid(bool valid);
    int satellites() const;

  private:
    Position position_{55.75, 37.61, 150};
    int satellites_ = 8;
    bool valid_ = true;
};
class VisualNavigation final : public NavigationSystem {
  public:
    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;
    void set_position(const Position& position);
    void set_target_tracked(bool tracked);

  private:
    Position position_{55.75, 37.61, 150};
    bool target_tracked_ = true;
};
bool valid_position(const Position& position);
void print_navigation_status(const NavigationSystem& navigation);
