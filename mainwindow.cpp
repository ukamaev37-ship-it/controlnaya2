#include "mainwindow.h"
#include "autopilot.h"
#include "labview.h"
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>
namespace {
QLabel* label(const QString& text, const char* name = "", QWidget* parent = nullptr) {
    auto* l = new QLabel(text, parent);
    l->setObjectName(name);
    l->setWordWrap(true);
    l->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    if (QStringList{"appName", "edition", "title"}.contains(QString::fromLatin1(name)))
        l->setWordWrap(false);
    return l;
}
QPushButton* button(const QString& text, const char* name) {
    auto* b = new QPushButton(text);
    b->setObjectName(name);
    b->setCursor(Qt::PointingHandCursor);
    b->setMinimumHeight(30);
    return b;
}
QWidget* formPage(QFormLayout*& form) {
    auto* page = new QWidget;
    form = new QFormLayout(page);
    form->setContentsMargins(0, 0, 0, 0);
    form->setVerticalSpacing(8);
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    return page;
}
const char* titles[] = {"Лётные испытания", "Сенсорный контур", "Навигационное решение",
                        "Цикл автопилота"};
const char* subtitles[] = {"Взлёт, полёт и посадка двух типов БПЛА",
                           "Показания и проверка готовности датчиков",
                           "Достоверность координат и доступность источника",
                           "Контроль высоты и проверка отказных сценариев"};
} // namespace
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Контрольная 02 · Полётная лаборатория · Вариант 1");
    resize(1280, 860);
    setMinimumSize(1100, 760);
    setStyleSheet(R"(
        QWidget {font-family:'DejaVu Sans'; font-size:13px; color:#272d35;}
        QMainWindow, QWidget#root {background:#f1f2f4;}
        QWidget#appBar, QWidget#tabs {background:#ffffff; border-bottom:1px solid #d8dce1;}
        QLabel#appName {font-size:14px; font-weight:600;}
        QLabel#edition, QLabel#subtitle {color:#697480; font-size:12px;}
        QLabel#title {font-size:20px; font-weight:600;}
        QLabel#eyebrow {font-size:13px; font-weight:600; color:#454e59;}
        QLabel#result {font-size:13px; color:#334b64; padding:10px; background:#eaf0f6; border:1px solid #d8e2ec; border-radius:3px;}
        QLabel#result[failure="true"] {color:#973f38; background:#faeeec; border-color:#ecc9c5;}
        QLabel#status {color:#4d5c6b; font-size:12px;}
        QWidget#controls {background:#ffffff; border:1px solid #d8dce1;}
        QPushButton {background:#fafbfc; border:1px solid #c9ced5; border-radius:3px; padding:3px 12px;}
        QPushButton:hover {background:#eaf0f6; border-color:#9aafc5;}
        QPushButton:focus {border:2px solid #356ea5;}
        QPushButton:disabled {color:#929aa5; background:#f0f2f4; border-color:#dce0e5;}
        QPushButton#navButton {border:0; border-radius:0; border-bottom:3px solid transparent; padding:5px 22px; color:#5b6572;}
        QPushButton#navButton:checked {color:#235c94; border-bottom:3px solid #356ea5; background:#f4f7fb;}
        QPushButton#navButton:hover {background:#f0f3f7;}
        QPushButton#start {background:#356ea5; color:#ffffff; border:1px solid #356ea5; font-weight:600;}
        QPushButton#start:hover {background:#295d8f;}
        QPushButton#start:disabled {background:#b5c7d9; border-color:#b5c7d9;}
        QPushButton#toggleLog, QPushButton#saveLog, QPushButton#clearLog {min-height:22px; padding:0 8px; font-size:12px;}
        QComboBox, QSpinBox, QDoubleSpinBox {background:#ffffff; border:1px solid #c8cfd7; border-radius:3px; padding:4px 7px; min-height:22px;}
        QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {border:1px solid #356ea5;}
        QComboBox:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled {color:#89939f; background:#f3f4f6;}
        QCheckBox {spacing:7px;}
        QCheckBox:disabled {color:#929aa5;}
        QCheckBox::indicator {width:16px; height:16px;}
        QPlainTextEdit {background:#ffffff; color:#44505e; border:1px solid #d8dce1; padding:6px; font-family:'DejaVu Sans Mono'; font-size:11px;}
    )");
    auto* root = new QWidget;
    root->setObjectName("root");
    setCentralWidget(root);
    auto* outer = new QVBoxLayout(root);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    auto* appBar = new QWidget;
    appBar->setObjectName("appBar");
    auto* appLayout = new QHBoxLayout(appBar);
    appLayout->setContentsMargins(18, 10, 18, 10);
    appLayout->addWidget(label("Лётная лаборатория", "appName"));
    appLayout->addStretch();
    appLayout->addWidget(label("Контрольная работа № 2   /   Вариант 1", "edition"));
    outer->addWidget(appBar);
    auto* tabs = new QWidget;
    tabs->setObjectName("tabs");
    auto* tabLayout = new QHBoxLayout(tabs);
    tabLayout->setContentsMargins(12, 0, 12, 0);
    tabLayout->setSpacing(0);
    auto* group = new QButtonGroup(this);
    group->setExclusive(true);
    const QStringList names{"Аппараты", "Сенсоры", "Навигация", "Автопилот"};
    for (int i = 0; i < 4; ++i) {
        auto* b = button(names[i], "navButton");
        b->setCheckable(true);
        b->setProperty("mode", i);
        group->addButton(b, i);
        if (i == 0)
            b->setChecked(true);
        tabLayout->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, i] { selectMode(i); });
    }
    tabLayout->addStretch();
    outer->addWidget(tabs);
    auto* workspace = new QWidget;
    auto* work = new QVBoxLayout(workspace);
    work->setContentsMargins(18, 14, 18, 10);
    work->setSpacing(10);
    outer->addWidget(workspace, 1);
    auto* top = new QHBoxLayout;
    title_ = label("", "title");
    subtitle_ = label("", "subtitle");
    top->addWidget(title_);
    top->addSpacing(16);
    top->addWidget(subtitle_, 1);
    work->addLayout(top);
    auto* body = new QHBoxLayout;
    body->setSpacing(12);
    view_ = new LabView;
    view_->setObjectName("labView");
    body->addWidget(view_, 1);
    auto* controls = new QWidget;
    controls->setObjectName("controls");
    controls->setFixedWidth(300);
    auto* panel = new QVBoxLayout(controls);
    panel->setContentsMargins(16, 16, 16, 16);
    panel->setSpacing(10);
    panel->addWidget(label("Параметры испытания", "eyebrow"));
    settings_ = new QStackedWidget;
    settings_->setObjectName("settings");
    QFormLayout* f;
    auto* flight = formPage(f);
    vehicle_ = new QComboBox;
    vehicle_->setObjectName("vehicle");
    vehicle_->addItems({"Квадрокоптер", "Самолётный БПЛА"});
    f->addRow("Летательный аппарат", vehicle_);
    f->addRow(label("Квадрокоптер взлетает вертикально. Самолётному БПЛА нужна "
                    "полоса.\n\nИспытание: взлёт → полёт → посадка.",
                    "subtitle"));
    settings_->addWidget(flight);
    auto* sensors = formPage(f);
    sensorReady_ = new QCheckBox("Гироскоп готов");
    sensorReady_->setObjectName("sensorReady");
    sensorReady_->setChecked(true);
    f->addRow(sensorReady_);
    f->addRow(label("Эталонные показания", "eyebrow"));
    f->addRow(label("Гироскоп      1.25 rad/s\nАкселерометр  9.81 m/s²\nБарометр      1012.8 hPa"));
    f->addRow(label("Снимите флажок, чтобы проверить отказ. Неготовый сенсор не считывается.",
                    "subtitle"));
    settings_->addWidget(sensors);
    auto* nav = formPage(f);
    navigation_ = new QComboBox;
    navigation_->setObjectName("navigation");
    navigation_->addItems({"GPS", "Визуальная"});
    satellites_ = new QSpinBox;
    satellites_->setObjectName("satellites");
    satellites_->setRange(0, 20);
    satellites_->setValue(8);
    gpsValid_ = new QCheckBox("GPS разрешён");
    gpsValid_->setObjectName("gpsValid");
    gpsValid_->setChecked(true);
    tracked_ = new QCheckBox("Цель отслеживается");
    tracked_->setObjectName("tracked");
    tracked_->setChecked(true);
    f->addRow("Источник позиции", navigation_);
    f->addRow("Видимые спутники", satellites_);
    f->addRow(gpsValid_);
    f->addRow(tracked_);
    f->addRow(
        label("GPS требует ≥ 4 спутников.\nВизуальная навигация — захвата цели.", "subtitle"));
    settings_->addWidget(nav);
    auto* autoPage = formPage(f);
    scenario_ = new QComboBox;
    scenario_->setObjectName("scenario");
    scenario_->addItems({"01 · Штатный цикл", "02 · Отказ сенсора", "03 · Навигация невалидна",
                         "04 · Нет контроллера", "05 · Отрицательная высота"});
    altitude_ = new QDoubleSpinBox;
    altitude_->setObjectName("altitude");
    altitude_->setRange(0, 1000);
    altitude_->setValue(150);
    altitude_->setSuffix(" м");
    f->addRow("Проверочный сценарий", scenario_);
    f->addRow("Начальная высота", altitude_);
    f->addRow(label("Цель 200 м · шаг 0.1 с\nРегулятор: u = 0.5 × error", "subtitle"));
    f->addRow(label("Сценарий 05: высота −1 м.", "subtitle"));
    settings_->addWidget(autoPage);
    panel->addWidget(settings_, 1);
    result_ = label("", "result");
    result_->setMinimumHeight(46);
    panel->addWidget(result_);
    start_ = button("Запустить испытание", "start");
    panel->addWidget(start_);
    auto* actions = new QHBoxLayout;
    pause_ = button("Пауза", "pause");
    auto* resetButton = button("Сброс", "reset");
    actions->addWidget(pause_);
    actions->addWidget(resetButton);
    panel->addLayout(actions);
    motion_ = new QCheckBox("Анимация опыта");
    motion_->setObjectName("motion");
    motion_->setChecked(true);
    panel->addWidget(motion_);
    body->addWidget(controls);
    work->addLayout(body, 1);
    auto* stateRow = new QHBoxLayout;
    status_ = label("", "status");
    step_ = label("Учебная визуализация", "subtitle");
    step_->setWordWrap(false);
    stateRow->addWidget(status_, 1);
    stateRow->addWidget(step_);
    work->addLayout(stateRow);
    auto* journalHeader = new QHBoxLayout;
    auto* toggleLog = button("⌄  Журнал событий", "toggleLog");
    toggleLog->setCheckable(true);
    toggleLog->setChecked(true);
    journalHeader->addWidget(toggleLog);
    journalHeader->addStretch();
    auto* save = button("Сохранить журнал", "saveLog");
    auto* clear = button("Очистить", "clearLog");
    journalHeader->addWidget(save);
    journalHeader->addWidget(clear);
    work->addLayout(journalHeader);
    log_ = new QPlainTextEdit;
    log_->setObjectName("log");
    log_->setReadOnly(true);
    log_->setMaximumBlockCount(2000);
    log_->setFixedHeight(104);
    log_->setPlaceholderText("Выберите опыт и запустите проверку. Здесь появится вывод программы.");
    work->addWidget(log_);
    connect(toggleLog, &QPushButton::toggled, this, [this, toggleLog](bool visible) {
        log_->setVisible(visible);
        toggleLog->setText(visible ? "⌄  Журнал событий" : "›  Журнал событий");
    });
    timer_ = new QTimer(this);
    timer_->setInterval(30);
    connect(timer_, &QTimer::timeout, this, &MainWindow::tick);
    connect(start_, &QPushButton::clicked, this, &MainWindow::run);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::reset);
    connect(pause_, &QPushButton::clicked, this, [this] {
        if (timer_->isActive()) {
            timer_->stop();
            pause_->setText("Продолжить");
            status_->setText("ПАУЗА");
        } else if (running_) {
            timer_->start();
            pause_->setText("Пауза");
            status_->setText("ИСПЫТАНИЕ ВЫПОЛНЯЕТСЯ");
        }
    });
    connect(clear, &QPushButton::clicked, log_, &QPlainTextEdit::clear);
    connect(save, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getSaveFileName(this, "Сохранить журнал",
                                                          "control-02.txt", "Текст (*.txt)");
        if (path.isEmpty())
            return;
        QFile file(path);
        const auto data = log_->toPlainText().toUtf8();
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) ||
            file.write(data) != data.size())
            status_->setText("НЕ УДАЛОСЬ СОХРАНИТЬ ЖУРНАЛ");
        else
            status_->setText("ЖУРНАЛ СОХРАНЁН");
    });
    connect(vehicle_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this] { syncView(); });
    connect(navigation_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this] { syncView(); });
    connect(satellites_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] { syncView(); });
    for (auto* box : {sensorReady_, gpsValid_, tracked_})
        connect(box, &QCheckBox::toggled, this, [this] { syncView(); });
    connect(altitude_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this] { syncView(); });
    selectMode(0);
}
void MainWindow::appendLog(const QString& text) {
    log_->appendPlainText(text);
}
void MainWindow::setRunning(bool running) {
    running_ = running;
    settings_->setEnabled(!running);
    motion_->setEnabled(!running);
    start_->setEnabled(!running);
    pause_->setEnabled(running);
    pause_->setText("Пауза");
    if (!running)
        timer_->stop();
}
void MainWindow::selectMode(int mode) {
    setRunning(false);
    mode_ = mode;
    progress_ = 0;
    settings_->setCurrentIndex(mode);

    title_->setText(QString::fromUtf8(titles[mode]));
    title_->setMinimumWidth(title_->fontMetrics().horizontalAdvance(title_->text()) + 2);
    subtitle_->setText(QString::fromUtf8(subtitles[mode]));
    view_->setMode(mode);
    view_->setProgress(0);
    view_->setFault(false);
    result_->setProperty("failure", false);
    result_->style()->unpolish(result_);
    result_->style()->polish(result_);
    status_->setText("ГОТОВ К ИСПЫТАНИЮ");
    result_->setText("Ожидание запуска.\nПараметры можно изменить выше.");
    syncView();
}
void MainWindow::syncView() {
    view_->setFixedWing(vehicle_->currentIndex() == 1);
    view_->setReadings(sensorReady_->isChecked());
    const bool visual = navigation_->currentIndex() == 1;
    view_->setNavigation(visual, satellites_->value(),
                         visual ? tracked_->isChecked()
                                : (gpsValid_->isChecked() && satellites_->value() >= 4));
    satellites_->setEnabled(!visual);
    gpsValid_->setEnabled(!visual);
    tracked_->setEnabled(visual);
    view_->setAltitude(mode_ == 3 ? altitude_->value() : 150);
}
void MainWindow::reset() {
    setRunning(false);
    vehicle_->setCurrentIndex(0);
    navigation_->setCurrentIndex(0);
    scenario_->setCurrentIndex(0);
    sensorReady_->setChecked(true);
    gpsValid_->setChecked(true);
    tracked_->setChecked(true);
    satellites_->setValue(8);
    altitude_->setValue(150);
    selectMode(mode_);
    appendLog("— Параметры опыта восстановлены —");
}
void MainWindow::run() {
    progress_ = 0;
    view_->setProgress(0);
    view_->setFault(false);
    bool success = true;
    appendLog(QString("\n—— %1 · Вариант 1 ——").arg(title_->text()));
    if (mode_ == 0) {
        UAV& vehicle =
            vehicle_->currentIndex() == 0 ? static_cast<UAV&>(quad_) : static_cast<UAV&>(wing_);
        run_flight_test(vehicle);
        result_->setText("Взлёт и посадка\nпроверены через UAV&.");
    } else if (mode_ == 1) {
        gyro_.set_ready(sensorReady_->isChecked());
        success = check_sensors({&gyro_, &accelerometer_, &barometer_});
        result_->setText(success ? "3 / 3 сенсора готовы"
                                 : "Отказ гироскопа\nСчитывание запрещено.");
    } else if (mode_ == 2) {
        gps_.set_satellites(satellites_->value());
        gps_.set_valid(gpsValid_->isChecked());
        visual_.set_target_tracked(tracked_->isChecked());
        const NavigationSystem& nav = navigation_->currentIndex() == 0
                                          ? static_cast<const NavigationSystem&>(gps_)
                                          : static_cast<const NavigationSystem&>(visual_);
        print_navigation_status(nav);
        success = nav.is_valid();
        result_->setText(success ? "Позиция достоверна\n55.75° / 37.61° / 150 м"
                                 : "Навигация невалидна\nПозицию использовать нельзя.");
    } else {
        const int scenario = scenario_->currentIndex();
        Gyroscope gyro;
        Accelerometer accel;
        Barometer baro;
        GPSNavigation gps;
        gyro.set_ready(scenario != 1);
        gps.set_valid(scenario != 2);
        initialAltitude_ = scenario == 4 ? -1 : altitude_->value();
        gps.set_position({55.75, 37.61, initialAltitude_});
        Autopilot pilot(&quad_, {&gyro, &accel, &baro}, &gps,
                        scenario == 3 ? nullptr : &controller_);
        pilot.run_cycle();
        const auto& r = pilot.last_result();
        success = r.success;
        view_->setAltitude(initialAltitude_);
        result_->setText(success ? QString("Результат цикла:\nОшибка: %1 м · u: %2")
                                       .arg(r.error, 0, 'f', 1)
                                       .arg(r.output, 0, 'f', 1)
                                 : r.message);
    }
    result_->setProperty("failure", !success);
    result_->style()->unpolish(result_);
    result_->style()->polish(result_);
    view_->setFault(!success);
    if (!success) {
        setRunning(false);
        status_->setText("ОТКАЗ · ЦИКЛ ОСТАНОВЛЕН");
        return;
    }
    if (!motion_->isChecked()) {
        view_->setProgress(1);
        status_->setText("ПРОВЕРКА ЗАВЕРШЕНА");
        return;
    }
    setRunning(true);
    status_->setText("ИСПЫТАНИЕ ВЫПОЛНЯЕТСЯ");
    timer_->start();
}
void MainWindow::tick() {
    if (!running_)
        return;
    progress_ = qMin(1.0, progress_ + 0.006);
    view_->setProgress(progress_);
    if (mode_ == 3)
        view_->setAltitude(initialAltitude_ +
                           (200 - initialAltitude_) * (1 - std::exp(-4 * progress_)));
    if (progress_ >= 1) {
        setRunning(false);
        status_->setText("ПРОВЕРКА ЗАВЕРШЕНА");
    }
}
