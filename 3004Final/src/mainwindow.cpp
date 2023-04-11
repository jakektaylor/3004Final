#include "mainwindow.h"
#include "ui_mainwindow.h"

//Construtor and destructor for the MainWindow class.
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Display a blank screen initially.
    this->clearScreen();
    ui->pulseIcon->setVisible(false);

    //Initialize the MainWindow's member variables.
    this->powerOn = false;
    this->sessionActive = false;
    this->backOrMenu = false;
    this->increaseSetting = false;

    //For drawing pulse points.
    this->linePen = new QPen(QColor("black"));
    this->linePen->setWidth(2);

    //Connect the QPushButtons on the device's signals to their corresponding slots.
    connect(ui->powerButton, &QPushButton::pressed, this, &MainWindow::togglePowerOn);
    connect(ui->upButton, &QPushButton::pressed, this, &MainWindow::navigateUpMenu);
    connect(ui->downButton, &QPushButton::pressed, this, &MainWindow::navigateDownMenu);
    connect(ui->leftButton, &QPushButton::pressed, this, &MainWindow::navigateLeft);
    connect(ui->rightButton, &QPushButton::pressed, this, &MainWindow::navigateRight);
    connect(ui->selectorButton, &QPushButton::pressed, this, &MainWindow::goToSubMenu);
    connect(ui->menuButton, &QPushButton::pressed, this, &MainWindow::goToMainMenu);
    connect(ui->backButton, &QPushButton::pressed, this, &MainWindow::goBack);
    connect(ui->sensorBox, &QComboBox::currentTextChanged, this, &MainWindow::sensorStateChanged);

    //Create a new profile.
    profile = new Profile(100);

    //Setup for the batteryBar.
    ui->batteryBar->setValue(profile->getBatteryLevel());
    batteryTimer = new QTimer(this);
    connect(batteryTimer, &QTimer::timeout, this, &MainWindow::drainBattery);
    connect(ui->rechargeButton, &QPushButton::pressed, this, &MainWindow::rechargeBattery);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete batteryTimer;
    delete profile;
    delete linePen;
    if(currentSession != NULL) delete currentSession;
    if(generator != NULL) delete generator;
    if(mainMenu != NULL) delete mainMenu;
    if(scene!=NULL) delete scene;

}

/***IMPLEMENTING THE SLOTS FOR THE MAINWINDOW CLASS***/

/*Purpose: This slot is called whenever the 'batteryTimer' emits a 'timeout' signal.*/
void MainWindow::drainBattery() {
    //Reduce the battery level by 5
    profile->setBatteryLevel(profile->getBatteryLevel() - 5);
    ui->batteryBar->setValue(profile->getBatteryLevel());

    //Turn off the device if the battery level reaches 0.
    if(profile->getBatteryLevel() == 0 && this->powerOn) togglePowerOn();
}

/*Purpose: This slot is called whenever the 'Recharge Battery' button is pressed in the UI.*/
void MainWindow::rechargeBattery() {
    //Refill battery to 100%
    profile->setBatteryLevel(100);
    ui->batteryBar->setValue(profile->getBatteryLevel());
}

/*Purpose: This slot is called whever the QComboBox in the UI labeled 'Sensor' has its value changed.*/
void MainWindow::sensorStateChanged(const QString& text) {
    if(!QString::compare(text, "Off", Qt::CaseInsensitive)) {   //The Sensor is off
            ui->pulseIcon->setVisible(false);

            //Ends an active session if the sensor is removed.
            if(this->sessionActive) endSession();
    } else{                                                     //The sensor is on
        ui->pulseIcon->setVisible(true);
    }
}

/*  Purpose: This method is called when the power button in the UI is pressed. If the device is off, it turns it on. If it
    is on, it turns it off.
*/
void MainWindow::togglePowerOn() {
    if(!this->powerOn) {//Turn on the device
        ui->powerOffView->setVisible(false);
        this->powerOn = true;
        this->sessionActive = false;

        //CHANGE SESSION AND GENERATOR CONSTRUCTOR PARAMS. TO CHANGE COHERENCE (DEFAULT IS LOW)
        currentSession = new Session(3, 10);            //Create a new underlying Session.
        generator = new Datagen("Low");                 //Low coherence to start.
        connect(currentSession, &Session::getSensorReading, generator, &Datagen::getSensorReading);
        connect(generator, &Datagen::sendSensorReading, currentSession, &Session::updatePulseData);
        connect(currentSession, &Session::updateSessionDisplay, this, &MainWindow::plotPulsePoint);
        connect(currentSession, &Session::sendSessionSummary, this, &MainWindow::displaySessionSummary);

        //Create and display the main menu.
        mainMenu = new Menu("Main Menu", {"Start New Session", "Settings", "History"}, NULL);
        this->initializeMainMenu();                         //Creates the tree of menus, with root 'mainMenu'.

        //Display the main menu
        currMenu = mainMenu;
        displayingMenu = true;
        displayingSession = false;
        displayingSummary = false;
        displayingSlider = false;
        this->displayCurrMenu();

        this->batteryTimer->start(30000);               //Start draining the battery every 30s that the device is on.
    }else{//Turn off the device
        ui->powerOffView->setVisible(true);
        this->batteryTimer->stop();
        this->powerOn = false;
        this->backOrMenu = false;
        if(this->displayingSession) endSession();
        mainMenu = NULL;
        currMenu = NULL;
        sessionSummary = Log();
    }

}

/*Purpose: Navigates down a menu, called when the down button in the UI is pressed.*/
void MainWindow::navigateDownMenu() {
    if(this->displayingMenu) {
        int currIndex = ui->menuWidget->currentRow();
        if(currIndex == (currMenu->getLists().size() - 1)) return;
        ui->menuWidget->setCurrentRow(++currIndex);
    }
}

/*Purpose: Navigates up a menu, called when the up button in the UI is pressed.*/
void MainWindow::navigateUpMenu() {
    if(this->displayingMenu) {
        int currIndex = ui->menuWidget->currentRow();
        if(currIndex == 0) return;
        ui->menuWidget->setCurrentRow(--currIndex);
    }
}

/*  Purpose: If the user is on a summary screen, moves left in the QListWidget allowing the user to choose whether to save the Session
    summary. If the user is on a slider screen changing a Setting, it reduces the value of the Setting. This method is called whenever
    the left arrow button in the UI is pressed.
*/
void MainWindow::navigateLeft() {
    int currIndex = ui->keepSummary->currentRow();                          //The currently selected index.
    //Used to choose to keep or delete a Session summary.
    if(this->displayingSummary && currIndex > 0) {
        ui->keepSummary->setCurrentRow(--currIndex);
    } else if(this->displayingSlider) {

        //Reduce the value of the Setting if it is not already at its minimum value.
        if(ui->settingValue->value() > ui->settingValue->minimum()) {
            this->increaseSetting = false;
            changeSetting();
        }
    }
}

/*  Purpose: If the user is on a summary screen, moves right in the QListWidget allowing the user to choose whether to save the Session
    summary. If the user is on a slider screen changing a Setting, it increases the value of the Setting. This method is called whenever
    the right arrow button in the UI is pressed.
*/
void MainWindow::navigateRight() {
    int currIndex = ui->keepSummary->currentRow();                          //The currently selected index.
    //Used to choose to keep or delete a Session summary.
    if(this->displayingSummary && currIndex < (ui->keepSummary->count()-1)) {
        ui->keepSummary->setCurrentRow(++currIndex);
    } else if(this->displayingSlider) {

        //Increase the value of the Setting if it is not already at its maximum value.
        if(ui->settingValue->value() < ui->settingValue->maximum()){
            this->increaseSetting = true;
            changeSetting();
        }
    }
}

/*  Purpose: This method is called whenever the selector button in the UI is pressed. The functionality that occurs depends on the
 *  current view that the device is displaying.
*/
void MainWindow::goToSubMenu() {

    //Handles the case where we are currently displaying a Menu
    if(this->displayingMenu) {
        int subMenuIndex = ui->menuWidget->currentRow();

        //Handles case where there user has selected a sub menu
        if(currMenu->getSubMenuAt(subMenuIndex) != NULL) {
            currMenu = currMenu->getSubMenuAt(subMenuIndex);
            displayCurrMenu();
        } else {

            //Handles the case where the user has chosen to go to the Session view.
            if (currMenu->getLists().at(subMenuIndex) == "Start New Session") {

                //Add code here to start a new Session.
                this->displayingMenu = false;
                this->displayingSession = true;
                displaySessionView();
            }

            //Handles the case where the user has chosen to modify a Setting.
            else if (currMenu->getMenuName() == "Settings") {
                this->displayingMenu=false;
                this->displayingSlider=true;
                displaySettingView();
            }

            //Handles the case where the user has chosen to review a Session summary.
            else if(currMenu->getMenuName() == "Review Session History") {
                this->displayingMenu = false;
                this->displayingSummary = true;
                displaySessionSummary(profile->getSessionHistory().at(subMenuIndex));
            }

            //Handles the case where the user has chosen whether or not to clear all Session data on the device.
            else if(currMenu->getMenuName() == "Clear Session History") {
                if(ui->menuWidget->currentRow() == 0) {
                    profile->resetDevice();
                    mainMenu->getSubMenuAt(2)->getSubMenuAt(0)->clear();
                }
                goBack();
            }
        }
    }

    //Handles the case where we are displaying a Session.
    else if(this->displayingSession) {
        if (this->sessionActive) {
            endSession();
        }
        else if(!QString::compare(ui->sensorBox->currentText(), "On", Qt::CaseInsensitive)) beginSession();
    }

    //Handles the case where we are displaying a slider to modify a Setting.
    else if (this->displayingSlider) {
        goBack();
    }

    //Handles the case where we are displaying a Session summary.
    else {

        //If the user selected keep, then keep the summary.
        if(ui->keepSummary->currentRow() == 0){
            int failure = profile->addNewSession(this->sessionSummary);     //Ensures Session Logs are not added more than once
            if(!failure) {
                QString dateString = this->sessionSummary.getDateTime().toString("Session dd:MM:yyyy hh:mm:ss");
                mainMenu->getSubMenuAt(2)->getSubMenuAt(0)->addListItem(dateString);
            }
        } else {    //Remove the summary
            int failure = profile->removeSession(ui->menuWidget->currentRow());
            if(!failure) mainMenu->getSubMenuAt(2)->getSubMenuAt(0)->removeitemAt(ui->menuWidget->currentRow());
        }

        //Need to reset all of the Session widgets
        this->revertSessionView();

        //Return the user to the main menu
        this->displayingSummary = false;
        this->displayingMenu = true;
        currMenu = mainMenu;
        displayCurrMenu();
    }
}

/*Purpose: Returns the user to the main Menu whenever the 'Menu' button in the UI is pressed.*/
void MainWindow::goToMainMenu() {
    if(this->displayingSession) {
        //Discard whatever was recorded or changed during the session.
        this->revertSessionView();
        this->backOrMenu = true;
        endSession();
    }
    this->displayingMenu = true;
    currMenu = mainMenu;
    displayCurrMenu();
}

/*Purpose: Sends the user to the previous screen they were on.*/
void MainWindow::goBack() {
    //Different behaviour if a Session is being displayed.
    if(this->displayingMenu) {
        if(currMenu->getPrevMenu()!=NULL) {
            this->displayingMenu = true;
            currMenu = currMenu->getPrevMenu();
            displayCurrMenu();
        }
    } else{
        if(this->displayingSession) {
            //Discard whatever was recorded or changed during the Session.
            this->revertSessionView();
            this->backOrMenu = true;
            endSession();
        } else if (this->displayingSummary) this->displayingSummary = false;
        else if(this->displayingSlider) this->displayingSlider = false;

        //If displaying a Summary, Session or Slider, simply go back to the last seen Menu.
        this->displayingMenu = true;
        displayCurrMenu();
    }
}

/*  Purpose: This method is called whenever the currentSession emits an updateSessionDisplay signal. It updates all of the metrics
    and the graph in the Session view while a Session is active.*/
void MainWindow::plotPulsePoint(Log currentLog) {

    /*Update the breath pacer*/
    if(currentLog.getSessionLength() != 0) {
        if((currentLog.getSessionLength() % currentLog.getPacerSpeed()) <= qCeil((float)currentLog.getPacerSpeed() / 2.0) &&
                (currentLog.getSessionLength() % currentLog.getPacerSpeed()) != 0) {
            ui->breathPacer->setValue(ui->breathPacer->value() + qCeil(100.0 / (float) qFloor((float)currentLog.getPacerSpeed() / 2.0)));
            ui->breathPacer->setFormat("Breathe In");
        } else  {
            ui->breathPacer->setValue(ui->breathPacer->value() - qCeil(100.0 / (float) qCeil((float)currentLog.getPacerSpeed() / 2.0)));
            ui->breathPacer->setFormat("Breathe Out");
        }
    }

    /**Plot a new point on the graph.**/
    plotHRVGraph(currentLog.getPulseData());

    /**Update the Session length.**/
    QString time = QDateTime::fromTime_t(currentLog.getSessionLength()).toUTC().toString("mm:ss");
    ui->lengthNumber->display(time);

    /**Show the coherence data if it is available (will be -1 if not)**/
    if(currentLog.getCoherenceScore() != -1) ui->coherenceNumber->display(currentLog.getCoherenceScore());
    if(currentLog.getAchievementScore() != -1) ui->acheivementNumber->display(currentLog.getAchievementScore());
    if(QString::compare(currentLog.getCoherenceLevel(), "NA", Qt::CaseInsensitive)) {

        //Sets the light to red for 'Low', blue for 'Medium' and green for 'High'.
        if(!QString::compare(currentLog.getCoherenceLevel(), "low", Qt::CaseInsensitive)) ui->coherenceLight->setStyleSheet("background-color: red;");
        else if (!QString::compare(currentLog.getCoherenceLevel(), "medium", Qt::CaseInsensitive)) ui->coherenceLight->setStyleSheet("background-color: blue;");
        else ui->coherenceLight->setStyleSheet("background-color:green;");
    }

    //Print the word ***BEEP*** if a new coherence level is reached.
    if(currentLog.isLevelChanged()) qInfo("********************BEEP********************");
}

/*Purpose: This method is responsible for displaying a summary of a session once endSession() has been called.*/
void MainWindow::displaySessionSummary(Log summary) {
    if(!this->backOrMenu) {//Ensures that a summary is not displayed if the back button was pressed
        this->displayingSummary = true;
        this->sessionSummary = summary;                     //Used to save the Session data.
        this->clearScreen();
        this->displaySessionView();
        ui->breathPacer->setVisible(false);
        ui->summaryView->setVisible(true);

        /*Display the average coherence score*/
        ui->coherenceLabel->setText("Average\nCoherence");
        ui->coherenceNumber->display((double) summary.getAchievementScore() / qFloor( (float) summary.getSessionLength() / 5.0));

        /*Display the Session Length*/
        QString time = QDateTime::fromTime_t(summary.getSessionLength()).toUTC().toString("mm:ss");
        ui->lengthNumber->display(time);

        /*Display the achievement score*/
        ui->acheivementNumber->display(summary.getAchievementScore());

        /*Display the challenge level*/
        ui->challengeLabel->setText(QString("Challenge Level: %1").arg(summary.getChallengeLevel()));

        /*Display the percentages of time in "Low", "Medium" and "High" coherence.*/
        ui->lowLabel->setText(QString("Low: %1%").arg(summary.getCoherenceDistribution()["Low"]));
        ui->mediumLabel->setText(QString("Medium: %1%").arg(summary.getCoherenceDistribution()["Medium"]));
        ui->highLabel->setText(QString("High: %1%").arg(summary.getCoherenceDistribution()["High"]));

        /*Plot the final HRV graph*/
        plotHRVGraph(summary.getPulseData());

        //Select the leftmost option in the set of options that decide whether to keep the summary that is displayed.
        ui->keepSummary->setCurrentRow(0);
    } else this->backOrMenu = false;             //Reset it to false.
}

/***INITIALIZING THE HELPER METHODS FOR THE MAINWINDOW CLASS***/

/*Purpose: This method is resonsible for initializing the tree of Menus on the device.*/
void MainWindow::initializeMainMenu() {

    //Create the Settings menu.
    Menu* settings = new Menu("Settings", {"Challenge Level", "Breath Pacer Speed"}, mainMenu);

    //Create the History (logs) menu.
    Menu* history = new Menu("History", {"Review Session History", "Clear Session History"}, mainMenu);
    Menu* reviewHistory = new Menu("Review Session History", {}, history);
    Menu* clearHistory = new Menu("Clear Session History", {"Yes", "No"}, history);

    history->addSubMenu(reviewHistory);
    history->addSubMenu(clearHistory);
    mainMenu->addSubMenu(NULL);            //NULL is used to indicate that the "Start New Session" menu entry does not lead to a menu.
    mainMenu->addSubMenu(settings);
    mainMenu->addSubMenu(history);
}

/*  Purpose: This method is responsible for clearing the screen so that a new view may be displayed. */
void MainWindow::clearScreen() {

    for(int i=0; i<ui->coherenceStack->count();i++) {
        ui->coherenceStack->itemAt(i)->widget()->setVisible(false);
        ui->lengthStack->itemAt(i)->widget()->setVisible(false);
        ui->acheivementStack->itemAt(i)->widget()->setVisible(false);
    }
    ui->menuLabel->setVisible(false);
    ui->sessionView->setVisible(false);
    ui->summaryView->setVisible(false);
    ui->settingsView->setVisible(false);
    ui->menuWidget->setVisible(false);
}

/*Purpose: Resets all of the widgets related to the Session metrics.
 * Always call this before calling clearScreen() after a Session has been run.*/
void MainWindow::revertSessionView() {
    ui->coherenceLabel->setText("Coherence");
    ui->coherenceNumber->display(0);
    ui->acheivementNumber->display(0);
    ui->lengthNumber->display(QDateTime::fromTime_t(0).toUTC().toString("mm:ss"));
    ui->breathPacer->setVisible(true);
    ui->breathPacer->setValue(0);
    ui->breathPacer->setFormat("Breath Pacer");
}

/*  Purpose: This method is responsible for displaying the menu stored in the 'currMenu' variable.*/
void MainWindow::displayCurrMenu() {

    this->clearScreen();

    ui->menuLabel->setText(currMenu->getMenuName());
    ui->menuLabel->setVisible(true);

    ui->menuWidget->clear();
    ui->menuWidget->addItems(currMenu->getLists());
    ui->menuWidget->setCurrentRow(0);
    ui->menuWidget->setVisible(true);
}

/*  Purpose: This method is responsible for displaying the Session view where the user can start a Session.*/
void MainWindow::displaySessionView() {

    //Display the coherence score, session length and achievement score in the top bar.
    this->clearScreen();
    ui->sessionView->setVisible(true);
    for(int i=0; i<ui->coherenceStack->count();i++) {
        ui->coherenceStack->itemAt(i)->widget()->setVisible(true);
        ui->lengthStack->itemAt(i)->widget()->setVisible(true);
        ui->acheivementStack->itemAt(i)->widget()->setVisible(true);
    }

    //Create the scene.
    QGraphicsView* graph = ui->hrvGraph;
    graph->setAlignment(Qt::AlignLeft);                 //Ensures the graph starts being drawn on the left side of the screen.
    this->scene = new QGraphicsScene(graph);
    scene->setSceneRect(0, 0, 320, 160);                //Stops the scene from moving around.
    graph->setScene(scene);
}

/*  Purpose: This method is responsible for beginning a Session, measuring the user's heart rate and using it to compute the
    coherence related metrics.
*/
void MainWindow::beginSession(){
    this->sessionActive = true;
    this->currentSession->beginSession();
}

/*  Purpose: This method is responsible for stopping the Session object from receiving pulse data and
 *  creating a new underlying Session object for the device.
*/
void MainWindow::endSession(){
    this->currentSession->endSession();
    ui->coherenceLight->setStyleSheet("");              //Turns off the coherence light.
    this->sessionActive = false;
    this->displayingSession = false;

    //Create a new session object to be the underlying Session on the device.
    this->currentSession=new Session(3, 10);
    connect(currentSession, &Session::getSensorReading, generator, &Datagen::getSensorReading);
    connect(generator, &Datagen::sendSensorReading, currentSession, &Session::updatePulseData);
    connect(currentSession, &Session::updateSessionDisplay, this, &MainWindow::plotPulsePoint);
    connect(currentSession, &Session::sendSessionSummary, this, &MainWindow::displaySessionSummary);
}

/*  Purpose: This method is responsible for plotting the pulse data given in the argument 'pulseData' on a QGraphicsScene.
    It is used both to display the pulseData that is obtained during a Session and to display the Log of pulseData when the
    user is viewing the Session history.
*/
void MainWindow::plotHRVGraph(QVector<float> pulseData) {
    float currentWidth = scene->sceneRect().right();

    for(int i=0;i<pulseData.size();i++) {
        //Expand the Scene window if the line is near the right end of it.
        if (i*10 > currentWidth - 50) scene->setSceneRect(0, 0, currentWidth + 320, 160);

        float pulse = pulseData.at(i);
        scene->addEllipse(i * 10, -4 * pulse + 400, 1, 1, *linePen);
        if(i >=1) {
            float previousPulse = pulseData.at(i-1);
            scene->addLine(i * 10, -4 * pulse + 400,
                                 (i-1) * 10, -4 * previousPulse + 400, *linePen);
        }
        if (i % 5 == 0) {
            //Display the time.
            QString time = QDateTime::fromTime_t(i).toUTC().toString("mm:ss");
            QGraphicsTextItem* text = scene->addText(time);
            text->setScale(0.8);
            text->setPos((i * 10) - 15, 130);
        }
    }
}

/*  Purpose: The purpose of this method is to display a QSlider on screen that allows the user to change
    the current 'Challenge Level' or 'Breath Pacer Speed'. When the slider is displayed, the user simply uses
    the left and right arrow buttons to move the slider. They can press the 'Menu' button, the "Back" button or
    the selector button on the UI to return from the slider view.
*/
void MainWindow::displaySettingView() {
    ui->settingsView->setVisible(true);

    //Displays the slider where the user is able to change the challenge level.
    if(ui->menuWidget->currentRow() == 0){
        ui->menuLabel->setText("Challenge Level");
        ui->settingValue->setMinimum(1);
        ui->settingValue->setMaximum(4);
        ui->settingValue->setValue(currentSession->getChallengeLevel());
        ui->settingText->setText(QString::number(currentSession->getChallengeLevel()));

    }
    //Displays the slider where the user is able to change the pacer speed.
    else {
        ui->menuLabel->setText("Breath Pacer Speed");
        ui->settingValue->setMinimum(1);
        ui->settingValue->setMaximum(30);
        ui->settingValue->setValue(currentSession->getPacerSpeed());
        ui->settingText->setText(QString::number(currentSession->getPacerSpeed()));
    }
}

/*Purpose: This method is responsible for allowing the user to change the value of a Setting when viewing the slider.*/
void MainWindow::changeSetting() {

    //Handles the case where the user is changing the challenge level
    if(this->displayingSlider && ui->menuWidget->currentRow() == 0) {
        if(this->increaseSetting) currentSession->setChallengeLevel(currentSession->getChallengeLevel() + 1);
        else currentSession->setChallengeLevel(currentSession->getChallengeLevel() - 1);

        ui->settingValue->setValue(currentSession->getChallengeLevel());
        ui->settingText->setText(QString::number(currentSession->getChallengeLevel()));
    }

    //Handles the case where the user is changing the breath pacer speed
    else if (this->displayingSlider) {
        if(this->increaseSetting) currentSession->setPacerSpeed(currentSession->getPacerSpeed() + 1);
        else currentSession->setPacerSpeed(currentSession->getPacerSpeed() - 1);

        ui->settingValue->setValue(currentSession->getPacerSpeed());
        ui->settingText->setText(QString::number(currentSession->getPacerSpeed()));
    }
}



