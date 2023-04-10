#ifndef PROFILE_H
#define PROFILE_H
#include "log.h"

class Profile{
  
  public:
    Profile(int startingBattery);
    int getBatteryLevel();
    QVector<Log> getSessionHistory();
    void setBatteryLevel(int level);
    int addNewSession(Log session);
    int removeSession(int index);
    void resetDevice();
  
  private:
    //NOTE: All values here must be stored persistently.
    int batteryLevel;                           //Keeps track of the battery level, which is an int in interval [1, 100]
    QVector<Log> sessionHistory;                //History of all Sessions on the device.
};


#endif // PROFILE_H
