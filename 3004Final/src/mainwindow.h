#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QGraphicsTextItem>
#include "log.h"
#include "menu.h"
#include "profile.h"
#include "session.h"
#include "datagen.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;


    Menu* mainMenu;
    Menu* currMenu;                     //The current Menu we are on or the last Menu seen if not "displayingMenu"

    Profile* profile;                   //The underlying Profile object for the device.
    Session* currentSession;            //The underlying Session object for the device.
    Datagen* generator;                 //Used to generate basic periodic data.

    bool powerOn;
    bool sessionActive;                 //Whether or not there is currently a session running on the device.
    bool sensorOn;                      //Whether or not the sensor is on.

    //Whether or not a menu, session, slider or summary is currently being displayed.
    bool displayingMenu;
    bool displayingSession;
    bool displayingSlider;
    bool displayingSummary;

    bool increaseSetting;                               //Used to modify settings.
    bool backOrMenu;                                    //Prevents sa summary from being displayed if in a Session and press "Back" or "Menu"
    //Used for plotting pulse points.
    QPen* linePen;
    QGraphicsScene* scene;
    Log sessionSummary;                                 //Used for saving a Log of a Session.
    QTimer* batteryTimer;                               //Used for reducing the battery level after every X seconds.

    //Methods used to display the different views and configure the device.
    void initializeMainMenu();
    void clearScreen();
    void displayCurrMenu();
    void displaySessionView();
    void displaySettingView();
    void revertSessionView();
    void beginSession();
    void endSession();
    void plotHRVGraph(QVector<float> pulses);
    void changeSetting();
private slots:
    void plotPulsePoint(Log currentLog);
    void sensorStateChanged(const QString& text);
    void togglePowerOn();
    void navigateDownMenu();
    void navigateUpMenu();
    void navigateLeft();
    void navigateRight();
    void goToSubMenu();
    void goToMainMenu();
    void goBack();
    void displaySessionSummary(Log summary);
    void drainBattery();
    void rechargeBattery();
};
#endif // MAINWINDOW_H
