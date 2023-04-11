#ifndef PROFILE_H
#define PROFILE_H
#include "log.h"

/*The purpose of this class is to store the Session history and Battery level. It provides methods to access and modify these values
 as well.*/
class Profile{
  
  public:
    //Constructor
    Profile(int startingBattery);

    //Getters
    int getBatteryLevel();
    QVector<Log> getSessionHistory();

    //Setters
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
