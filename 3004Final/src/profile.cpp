#include "profile.h"

//Constructor
Profile::Profile(int startingBattery) {
    this->batteryLevel = startingBattery;
    this->sessionHistory = QVector<Log>();
}

//Getter methods
int Profile::getBatteryLevel() {return this->batteryLevel;}
QVector<Log> Profile::getSessionHistory() {return this->sessionHistory;}

//Setter methods

//Used to recharge the battery
void Profile::setBatteryLevel(int level){
    //Make sure the battery level is within a valid range.
    if(level > 100) level = 100;
    else if (level < 0) level = 0;
    this->batteryLevel = level;
}

//Used to add a new Session to the history of Sessions QVector
int Profile::addNewSession(Log session){
    for(int i =0;i<this->sessionHistory.size();i++){
        //Ensures a given Session is not added again if we press "Keep Summary" when viewing it after already adding it.
        if(this->sessionHistory.at(i).getDateTime() == session.getDateTime()) return -1;
    }
    this->sessionHistory.append(session);
    return 0;
}

//Used to remove a Session from the history of Sessions QVector.
int Profile::removeSession(int index){
    if(index < this->sessionHistory.size()){
        this->sessionHistory.removeAt(index);
        return 0;
    }
    return -1;
}
void Profile::resetDevice() {this->sessionHistory = QVector<Log>();}
