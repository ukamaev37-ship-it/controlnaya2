#include "labview.h"
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>
namespace {
const QColor bg("#20252b"), grid("#343d47"), ink("#e5e9ef"), muted("#9aa6b3"), blue("#6ba7e8"),
    red("#ed8179"), amber("#e1aa61");
void text(QPainter& p, double x, double y, const QString& s, int size = 14, QColor c = ink) {
    QFont f("DejaVu Sans");
    f.setPixelSize(size);
    p.setFont(f);
    p.setPen(c);
    p.drawText(QPointF(x, y), s);
}
void line(QPainter& p, QPointF a, QPointF b, QColor c = grid, double w = 1) {
    p.setPen(QPen(c, w));
    p.drawLine(a, b);
}
void polygon(QPainter& p, std::initializer_list<QPointF> points, QColor fill,
             QColor stroke = muted) {
    p.setBrush(fill);
    p.setPen(QPen(stroke, 1));
    p.drawPolygon(QPolygonF(QVector<QPointF>(points)));
}
void craft(QPainter& p, QPointF at, bool wing, double phase, double scale = 1) {
    p.save();
    p.translate(at);
    p.scale(scale, scale);
    if (wing) {
        polygon(p,
                {{0, -92},
                 {13, -15},
                 {128, 36},
                 {130, 51},
                 {12, 23},
                 {8, 77},
                 {45, 96},
                 {45, 106},
                 {0, 91},
                 {-45, 106},
                 {-45, 96},
                 {-8, 77},
                 {-12, 23},
                 {-130, 51},
                 {-128, 36},
                 {-13, -15}},
                QColor("#768694"));
        polygon(p, {{0, -92}, {13, -15}, {8, 77}, {0, 91}, {-8, 77}, {-13, -15}},
                QColor("#bac7d1"));
        polygon(p, {{0, -61}, {7, -27}, {-7, -27}}, QColor("#344f69"));
        line(p, {-119, 39}, {-40, 9}, blue, 3);
        line(p, {40, 9}, {119, 39}, blue, 3);
    } else {
        for (int y : {-1, 1})
            for (int x : {-1, 1}) {
                QPointF c(x * 91, y * 53);
                line(p, {0, 9}, c + QPointF(0, 9), QColor("#11171d"), 13);
                line(p, {0, 0}, c, QColor("#7f91a0"), 10);
                p.setBrush(QColor("#11171d"));
                p.setPen(QPen(muted, 1));
                p.drawEllipse(c, 15, 9);
                p.setBrush(QColor(107, 167, 232, 18));
                p.setPen(QPen(QColor("#5a7389"), 1));
                p.drawEllipse(c, 49, 24);
                p.save();
                p.translate(c);
                p.scale(1, .48);
                p.rotate(phase * 3400 + x * y * 45);
                p.setPen(Qt::NoPen);
                p.setBrush(QColor("#b6c5d0"));
                p.drawRoundedRect(QRectF(-44, -4, 88, 8), 4, 4);
                p.restore();
            }
        polygon(p, {{-29, -29}, {20, -29}, {36, 0}, {23, 37}, {-23, 37}, {-36, 0}},
                QColor("#526574"));
        polygon(p, {{-29, -29}, {20, -29}, {29, -4}, {15, 24}, {-20, 24}, {-28, -4}},
                QColor("#b6c5d0"));
        line(p, {-16, -15}, {12, -15}, blue, 4);
        p.setBrush(QColor("#11171d"));
        p.drawEllipse(QPointF(0, 29), 8, 6);
    }
    p.restore();
}
} // namespace
LabView::LabView(QWidget* p) : QWidget(p) {
    setMinimumSize(500, 320);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
void LabView::setMode(int v) {
    mode_ = std::clamp(v, 0, 3);
    update();
}
void LabView::setProgress(double v) {
    progress_ = std::clamp(v, 0., 1.);
    if (progress_ == 0)
        heightTrace_.clear();
    update();
}
void LabView::setFault(bool v) {
    fault_ = v;
    update();
}
void LabView::setFixedWing(bool v) {
    wing_ = v;
    update();
}
void LabView::setAltitude(double v) {
    altitude_ = v;
    if (std::isfinite(v) && v >= 0) {
        if (!heightTrace_.isEmpty() && heightTrace_.last().x() == progress_)
            heightTrace_.last().setY(v);
        else
            heightTrace_.append(QPointF(progress_, v));
    }
    update();
}
void LabView::setReadings(bool v) {
    ready_ = v;
    update();
}
void LabView::setNavigation(bool v, int s, bool valid) {
    visual_ = v;
    satellites_ = s;
    valid_ = valid;
    update();
}
void LabView::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), bg);
    const double scale = std::min(width() / 800., height() / 480.);
    p.translate((width() - 800 * scale) / 2, (height() - 480 * scale) / 2);
    p.scale(scale, scale);
    if (mode_ == 0) {
        for (int i = -8; i <= 8; ++i)
            line(p, {400 + i * 29., 190}, {400 + i * 110., 372});
        for (int i = 0; i < 8; ++i) {
            double y = 194 + std::pow(i / 7., 1.6) * 174;
            line(p, {0, y}, {800, y});
        }
        const double lift = std::sin(progress_ * 3.14159265);
        const double x = 400 + std::sin(progress_ * 6.28318) * 75;
        const double y = 252 - lift * 100;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 65));
        p.drawEllipse(QPointF(x, 310), 110 - lift * 22, 22 - lift * 5);
        craft(p, {x, y}, wing_, progress_, 1.15);
        text(p, 22, 30, wing_ ? "FixedWingUAV" : "Quadcopter", 16);
        text(p, 22, 52, "Вид аппарата · учебная схема", 13, muted);
        text(p, 610, 30,
             progress_ == 0    ? "На земле"
             : progress_ == 1  ? "Посадка завершена"
             : progress_ < .25 ? "Взлёт"
             : progress_ > .75 ? "Снижение"
                               : "Полёт",
             14, blue);
        line(p, {44, 335}, {79, 335}, blue, 2);
        line(p, {44, 335}, {44, 302}, muted, 2);
        text(p, 84, 340, "X", 12, muted);
        text(p, 39, 295, "Z", 12, muted);
        p.fillRect(QRectF(0, 380, 800, 100), QColor("#191e24"));
        line(p, {0, 380}, {800, 380});
        text(p, 22, 404, "Профиль испытания", 13, muted);
        QPainterPath path;
        path.moveTo(40, 448);
        if (wing_) {
            path.lineTo(225, 417);
            path.lineTo(560, 417);
            path.lineTo(760, 448);
        } else {
            path.lineTo(40, 417);
            path.lineTo(760, 417);
            path.lineTo(760, 448);
        }
        p.setPen(QPen(grid, 2));
        p.drawPath(path);
        auto dot = path.pointAtPercent(progress_);
        p.setBrush(blue);
        p.setPen(Qt::NoPen);
        p.drawEllipse(dot, 5, 5);
        text(p, 22, 470, "Взлёт", 13, muted);
        text(p, 370, 470, "Маршрут", 13, muted);
        text(p, 710, 470, "Посадка", 13, muted);
    } else if (mode_ == 1) {
        const QString names[] = {"Гироскоп", "Акселерометр", "Барометр"};
        const QString values[] = {"1.25 rad/s", "9.81 m/s²", "1012.8 hPa"};
        for (int i = 0; i < 3; ++i) {
            double top = 15 + i * 143;
            bool ok = ready_ || i != 0;
            text(p, 22, top + 24, names[i], 16);
            text(p, 22, top + 53, ok ? values[i] : "Нет данных", 18, ok ? blue : red);
            text(p, 22, top + 81, ok ? "Готов" : "Не готов", 13, ok ? muted : red);
            for (int j = 0; j < 5; ++j)
                line(p, {235, top + 15 + j * 22}, {778, top + 15 + j * 22});
            for (int j = 0; j < 10; ++j)
                line(p, {235 + j * 60., top + 15}, {235 + j * 60., top + 103});
            line(p, {235, top + 59}, {778, top + 59}, muted);
            QPainterPath path;
            for (int j = 0; j <= 180; ++j) {
                double yy = top + 59;
                if (ok)
                    yy -= std::sin(j * .13 + progress_ * 13 + i) * std::cos(j * .02 + i) * 28;
                QPointF q(235 + j * 3, yy);
                if (!j)
                    path.moveTo(q);
                else
                    path.lineTo(q);
            }
            p.setPen(QPen(ok ? blue : red, 1.7));
            p.drawPath(path);
            if (i < 2)
                line(p, {22, top + 138}, {778, top + 138});
        }
        text(p, 22, 467, "Осциллограммы иллюстративные. Показания датчиков постоянны по условию.",
             13, muted);
    } else if (mode_ == 2) {
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 7; ++col) {
                QRectF block(38 + col * 113, 55 + row * 65, 92, 44);
                p.fillRect(block, QColor("#293139"));
            }
        line(p, {0, 245}, {800, 245}, QColor("#3b4651"), 18);
        line(p, {350, 45}, {350, 402}, QColor("#3b4651"), 16);
        p.setPen(QPen(muted, 1, Qt::DashLine));
        p.drawLine(QPointF(0, 245), QPointF(800, 245));
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor("#4b6277"), 1, Qt::DashLine));
        p.drawEllipse(QPointF(400, 233), 140, 140);
        if (visual_) {
            p.setPen(QPen(valid_ ? blue : red, 2));
            for (int sx : {-1, 1})
                for (int sy : {-1, 1}) {
                    QPointF q(400 + sx * 73, 233 + sy * 58);
                    p.drawLine(q, q - QPointF(sx * 22, 0));
                    p.drawLine(q, q - QPointF(0, sy * 22));
                }
            text(p, 300, 330, valid_ ? "Цель захвачена" : "Цель потеряна", 15, valid_ ? blue : red);
        } else {
            int count = std::clamp(satellites_, 0, 20);
            for (int i = 0; i < count; ++i) {
                double a = i * 6.283185 / std::max(count, 1);
                QPointF q(400 + 255 * std::cos(a), 233 + 155 * std::sin(a));
                line(p, q, {400, 233}, valid_ ? QColor("#4d6b87") : grid);
                p.setBrush(bg);
                p.setPen(QPen(valid_ ? blue : red, 1.5));
                p.drawRect(QRectF(q.x() - 8, q.y() - 5, 16, 10));
                line(p, q - QPointF(16, 0), q + QPointF(16, 0), blue);
            }
        }
        craft(p, {400, 233}, false, progress_, .32);
        p.fillRect(QRectF(0, 0, 800, 43), bg);
        text(p, 22, 27,
             visual_ ? "VisualNavigation / сопровождение"
                     : "GPSNavigation / спутники: " + QString::number(satellites_),
             15);
        text(p, 650, 27, valid_ ? "VALID" : "INVALID", 14, valid_ ? blue : red);
        p.fillRect(QRectF(0, 418, 800, 62), QColor("#191e24"));
        text(p, 22, 444, "55.7500° N   37.6100° E", 16);
        text(p, 22, 466, "Локальная схема · не географическая карта", 12, muted);
        text(p, 610, 448, "Высота 150 м", 15, muted);
    } else {
        bool good = std::isfinite(altitude_) && altitude_ >= 0;
        double h = good ? altitude_ : 0;
        text(p, 22, 28, "Высота", 13, muted);
        text(p, 22, 57, good ? QString::number(h, 'f', 1) + " м" : "Невалидна", 23,
             good && !fault_ ? blue : red);
        text(p, 285, 28, "Ошибка до цели", 13, muted);
        text(p, 285, 57, good ? QString::number(200 - h, 'f', 1) + " м" : "—", 23);
        text(p, 585, 28, "Управление · k = 0.5", 13, muted);
        text(p, 585, 57, fault_ || !good ? "—" : QString::number((200 - h) * .5, 'f', 1), 23,
             fault_ ? red : ink);
        double low = 200, high = 200;
        for (const auto& q : heightTrace_) {
            low = std::min(low, q.y());
            high = std::max(high, q.y());
        }
        if (heightTrace_.isEmpty() && good) {
            low = std::min(low, h);
            high = std::max(high, h);
        }
        double pad = std::max(10., (high - low) * .12);
        low = std::max(0., low - pad);
        high += pad;
        auto y = [low, high](double v) { return 340 - (v - low) / (high - low) * 235; };
        for (int i = 0; i <= 4; ++i) {
            double v = low + (high - low) * i / 4;
            line(p, {70, y(v)}, {775, y(v)});
            text(p, 12, y(v) + 4, QString::number(v, 'f', 0), 12, muted);
        }
        for (int i = 0; i <= 4; ++i) {
            double x = 70 + 705 * i / 4.;
            line(p, {x, 105}, {x, 340});
            text(p, x - (i == 4 ? 32 : 10), 363, QString::number(i * 25) + "%", 12, muted);
        }
        text(p, 12, 94, "м", 12, muted);
        line(p, {70, y(200)}, {775, y(200)}, amber);
        text(p, 630, y(200) - 8, "Цель 200 м", 13, amber);
        QPainterPath path;
        bool first = true;
        for (const auto& q : heightTrace_) {
            QPointF pt(70 + 705 * q.x(), y(q.y()));
            if (first) {
                path.moveTo(pt);
                first = false;
            } else
                path.lineTo(pt);
        }
        p.setPen(QPen(fault_ ? red : blue, 2));
        p.drawPath(path);
        if (!heightTrace_.isEmpty()) {
            auto q = heightTrace_.last();
            p.setBrush(fault_ ? red : blue);
            p.drawEllipse(QPointF(70 + 705 * q.x(), y(q.y())), 4, 4);
        }
        text(p, 240, 390, "Ход демонстрации · не физическое время", 12, muted);
        p.fillRect(QRectF(0, 416, 800, 64), QColor("#191e24"));
        text(p, 22, 442, "Сенсоры  →  Навигация  →  Ошибка высоты  →  Регулятор", 15,
             fault_ ? red : ink);
        text(p, 22, 466,
             fault_ ? "Проверка не пройдена. Вычисление остановлено."
                    : "Autopilot / вызовы через абстрактные интерфейсы",
             12, muted);
    }
}
