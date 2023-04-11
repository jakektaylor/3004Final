#include "log.h"

//Constructor for the Log class.
Log::Log(){
    this->date = QDateTime::currentDateTime();

    //This value indicates that a coherence score has not yet been computed.
    this->coherenceLevel = "NA";

    //This values should all be considered invalid (should not be displayed until they are set)
    this->challengeLevel = -1;
    this->breathPacerSpeed = -1;
    this->coherenceTimes = QMap<QString, int>();
    this->sessionLength = -1;
    this->achievementScore = -1;
    this->pulseData = QVector<float>();
    this->coherenceScore = -1;
    this->levelChanged = false;
}

/***Implementing the getter methods for the Log class***/
int Log::getChallengeLevel() {return this->challengeLevel;}
int Log::getPacerSpeed() {return this->breathPacerSpeed;}

//Purpose: Returns a string representation of the date time when the Session was recorded.
const QDateTime& Log::getDateTime() const{
    return this->date;
}

/*Purpose: This method is responsible for computing the percentage of time spent in "Low", "Medium" and "High" coherence and returning
 * it to the user.*/
QMap<QString, float> Log::getCoherenceDistribution() {

    //Create a new QMap containing the percentage of time spent at each coherence level.
    QMap<QString, float> timePercentages = QMap<QString, float>();
    if(this->coherenceTimes.isEmpty()) return timePercentages;
    int coherenceSum = 0;                                                                //The sum of time spent at all coherence levels.
    QString key;
    foreach(key, this->coherenceTimes.keys()) coherenceSum += this->coherenceTimes.value(key, 0);

    foreach(key, this->coherenceTimes.keys()) timePercentages[key] = ((float) this->coherenceTimes[key] / (float) coherenceSum) * 100.0;
    return timePercentages;
}

int Log::getSessionLength() {return this->sessionLength;}
float Log::getAchievementScore() {return this->achievementScore;}
QVector<float> Log::getPulseData() {return this->pulseData;}
float Log::getCoherenceScore() {return this->coherenceScore;}
QString Log::getCoherenceLevel() {return this->coherenceLevel;}
float Log::getAverageCoherence() {
    return this->achievementScore / (float)(qFloor(this->sessionLength / 5.0));         //Recall: The number of computed coherence scores
}                                                                                       //is qFloor(this->sessionLength/5.0)
bool Log::isLevelChanged() {return this->levelChanged;}

/***Implementing the setter methods for the Log class***/
void Log::setChallengeLevel(int level){this->challengeLevel = level;}
void Log::setPacerSpeed(int speed){this->breathPacerSpeed = speed;}
void Log::setCoherenceTimes(QMap<QString, int> times){this->coherenceTimes = times;}
void Log::setSessionLength(int length){this->sessionLength = length;}
void Log::setAchievementScore(float score){this->achievementScore = score;}
void Log::setPulseData(QVector<float> data){this->pulseData = data;}
void Log::setCoherenceScore(float score){this->coherenceScore = score;}
void Log::setCoherenceLevel(QString level){this->coherenceLevel = level;}
void Log::setLevelChanged(bool isChanged){this->levelChanged = isChanged;}


