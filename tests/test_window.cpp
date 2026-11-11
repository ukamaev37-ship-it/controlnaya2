#include "mainwindow.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QtTest>
static MainWindow* logWindow = nullptr;
static void capture(QtMsgType, const QMessageLogContext&, const QString& message) {
    if (logWindow)
        logWindow->appendLog(message);
}
struct LogCapture {
    QtMessageHandler previous;
    explicit LogCapture(MainWindow& w) : previous(qInstallMessageHandler(capture)) {
        logWindow = &w;
    }
    ~LogCapture() {
        qInstallMessageHandler(previous);
        logWindow = nullptr;
    }
};
class WindowTest : public QObject {
    Q_OBJECT
    void mode(MainWindow& w, int index) {
        for (auto* b : w.findChildren<QPushButton*>("navButton"))
            if (b->property("mode").toInt() == index) {
                b->click();
                return;
            }
        QFAIL("Navigation button missing");
    }
  private slots:
    void compactLayoutAndJournal() {
        MainWindow w;
        w.resize(1100, 760);
        w.show();
        QTest::qWait(20);
        for (int i = 0; i < 4; ++i) {
            mode(w, i);
            QTest::qWait(10);
            auto* title = w.findChild<QLabel*>("title");
            QVERIFY(title->width() >= title->fontMetrics().horizontalAdvance(title->text()));
            auto* controls = w.findChild<QWidget*>("controls");
            for (auto* child : controls->findChildren<QWidget*>()) {
                if (!child->isVisible() ||
                    (!qobject_cast<QComboBox*>(child) && !qobject_cast<QCheckBox*>(child) &&
                     !qobject_cast<QPushButton*>(child)))
                    continue;
                const QRect bounds(child->mapTo(controls, QPoint(0, 0)), child->size());
                QVERIFY2(controls->rect().contains(bounds), qPrintable(child->objectName()));
            }
            QVERIFY(w.grab().save(QString("preview-compact-%1.png").arg(i)));
        }
        auto* toggle = w.findChild<QPushButton*>("toggleLog");
        auto* log = w.findChild<QPlainTextEdit*>("log");
        toggle->click();
        QVERIFY(!log->isVisible());
        toggle->click();
        QVERIFY(log->isVisible());
    }
    void flightAndPause() {
        MainWindow w;
        LogCapture captureLog(w);
        w.show();
        QTest::qWait(30);
        auto* start = w.findChild<QPushButton*>("start");
        auto* pause = w.findChild<QPushButton*>("pause");
        start->click();
        QVERIFY(!start->isEnabled());
        QVERIFY(pause->isEnabled());
        pause->click();
        QCOMPARE(w.findChild<QLabel*>("status")->text(), QString("ПАУЗА"));
        pause->click();
        for (int i = 0; i < 170; ++i)
            w.tick();
        QVERIFY(start->isEnabled());
        QVERIFY(!pause->isEnabled());
        QVERIFY(w.grab().save("preview-flight.png"));
        w.findChild<QComboBox*>("vehicle")->setCurrentIndex(1);
        start->click();
        for (int i = 0; i < 75; ++i) {
            w.tick();
        }

        QVERIFY(w.grab().save("preview-wing.png"));
        w.findChild<QPushButton*>("reset")->click();
        QVERIFY(start->isEnabled());
    }
    void sensorAndNavigationFailures() {
        MainWindow w;
        LogCapture captureLog(w);
        w.show();
        QTest::qWait(20);
        mode(w, 1);
        w.findChild<QCheckBox*>("sensorReady")->setChecked(false);
        w.findChild<QPushButton*>("start")->click();
        QVERIFY(w.findChild<QLabel*>("status")->text().contains("ОТКАЗ"));
        QVERIFY(w.grab().save("preview-sensor-failure.png"));
        w.findChild<QCheckBox*>("sensorReady")->setChecked(true);
        w.findChild<QPushButton*>("start")->click();
        for (int i = 0; i < 170; ++i) {
            w.tick();
        }
        QVERIFY(w.grab().save("preview-sensors.png"));
        mode(w, 2);
        w.findChild<QSpinBox*>("satellites")->setValue(3);
        w.findChild<QPushButton*>("start")->click();
        QVERIFY(w.findChild<QLabel*>("status")->text().contains("ОТКАЗ"));
        w.findChild<QSpinBox*>("satellites")->setValue(4);
        w.findChild<QPushButton*>("start")->click();
        for (int i = 0; i < 170; ++i) {
            w.tick();
        }
        QVERIFY(w.grab().save("preview-gps.png"));
        w.findChild<QComboBox*>("navigation")->setCurrentIndex(1);
        w.findChild<QCheckBox*>("tracked")->setChecked(false);
        w.findChild<QPushButton*>("start")->click();
        QVERIFY(w.findChild<QLabel*>("status")->text().contains("ОТКАЗ"));
        w.findChild<QCheckBox*>("tracked")->setChecked(true);
        w.findChild<QPushButton*>("start")->click();
        for (int i = 0; i < 170; ++i) {
            w.tick();
        }
        QVERIFY(w.grab().save("preview-visual.png"));
    }
    void integrationScenarios() {
        MainWindow w;
        LogCapture captureLog(w);
        w.show();
        QTest::qWait(20);
        mode(w, 3);
        auto* start = w.findChild<QPushButton*>("start");
        start->click();
        QVERIFY(w.findChild<QLabel*>("result")->text().contains("25.0"));
        QVERIFY(
            w.findChild<QPlainTextEdit*>("log")->toPlainText().contains("Controller output: 25"));
        for (int i = 0; i < 80; ++i) {
            w.tick();
        }
        QVERIFY(w.grab().save("preview-autopilot.png"));
        for (int i = 0; i < 90; ++i) {
            w.tick();
        }

        for (int i = 1; i < 5; ++i) {
            w.findChild<QComboBox*>("scenario")->setCurrentIndex(i);
            start->click();
            QVERIFY(w.findChild<QLabel*>("status")->text().contains("ОТКАЗ"));
            QVERIFY(start->isEnabled());
        }
        QVERIFY(w.grab().save("preview-autopilot-failure.png"));
        w.findChild<QPushButton*>("reset")->click();
        w.findChild<QCheckBox*>("motion")->setChecked(false);
        start->click();
        QVERIFY(start->isEnabled());
        QCOMPARE(w.findChild<QLabel*>("status")->text(), QString("ПРОВЕРКА ЗАВЕРШЕНА"));
        w.resize(1100, 760);
        QTest::qWait(20);
        QVERIFY(w.grab().save("preview-compact.png"));
    }
};
QTEST_MAIN(WindowTest)
#include "test_window.moc"
