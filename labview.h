#pragma once
#include <QPointF>
#include <QVector>
#include <QWidget>
class LabView : public QWidget {
  public:
    explicit LabView(QWidget* parent = nullptr);
    void setMode(int value);
    void setProgress(double value);
    void setFault(bool value);
    void setFixedWing(bool value);
    void setAltitude(double value);
    void setReadings(bool value);
    void setNavigation(bool visual, int satellites, bool valid);

  protected:
    void paintEvent(QPaintEvent*) override;

  private:
    int mode_ = 0, satellites_ = 8;
    double progress_ = 0, altitude_ = 150;
    QVector<QPointF> heightTrace_;
    bool fault_ = false, wing_ = false, ready_ = true, visual_ = false, valid_ = true;
};
