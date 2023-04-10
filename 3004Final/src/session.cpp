#include "session.h"

//Intiialize CHALLENGE_THRESHOLDS as an empty QMap.
QMap<int, QMap<QString, float>> Session::CHALLENGE_THRESHOLDS = QMap<int, QMap<QString, float>>();

//Constructor for the Session class.
Session::Session(int challengeLevel, int breathPacerSpeed, QObject *parent):QObject(parent) {
    //Instantiate necessary variables.
    this->challengeLevel = challengeLevel;
    this->breathPacerSpeed = breathPacerSpeed;
    pulseData = QVector<float>();
    coherenceScore = 0;
    sessionLength = -1;                                     //Start it at -1 to update the session data at timestep 0
    sessionTimer = new QTimer(this);
    achievementScore = 0;
    coherenceLevel = "NA";
    this->levelChanged = false;

    //The keys in the QMap are "Low", "Medium" and "High".
    coherenceTimes = QMap<QString, int>();
    coherenceTimes.insert("Low", 0);
    coherenceTimes.insert("Medium", 0);
    coherenceTimes.insert("High", 0);

    //Initialize the CHALLENGE_THRESHOLDS QMap if it has not already been done.
    if (CHALLENGE_THRESHOLDS.isEmpty()) {
        initializeThresholds();
    }

    //Connect the QTimer's "timeout" signal to the Session's "updateSessionData" slot.
    connect(sessionTimer, &QTimer::timeout, this, &Session::updateSessionData);

}

//Destructor for the Session class.
Session::~Session() {
    delete sessionTimer;
}


/***IMPLEMENTING THE SLOTS FOR THE SESSION CLASS***/

/* Purpose: This slot is called in response to the selector button emitting a "pressed" signal when the user is on the session view and the
 * session has not yet begun.*/
void Session::beginSession() {
    updateSessionData();
    sessionTimer->start(1000);
}

/*Purpose: This slot is called in response to the 'sessionTimer' emitting a 'timeout' signal, which it does every second.*/
void Session::updateSessionData() {
    this->sessionLength += 1;
    emit getSensorReading(this->sessionLength);

    //Determine if a new coherence score needs to be computed.
    if((this->sessionLength % 5) == 0 && this->sessionLength != 0) {
        updateCoherence();
    }

    //Create a Log object to send all of the necessary information to the MainWindow.
    Log currentLog = Log();
    currentLog.setCoherenceScore(this->coherenceScore);
    currentLog.setCoherenceLevel(this->coherenceLevel);
    currentLog.setLevelChanged(this->levelChanged);
    currentLog.setSessionLength(this->sessionLength);
    currentLog.setAchievementScore(this->achievementScore);
    currentLog.setPulseData(this->pulseData);
    currentLog.setPacerSpeed(this->breathPacerSpeed);
    emit updateSessionDisplay(currentLog);
    this->levelChanged = false;                 //Ensures the device does not keep beeping in between calculating coherence scores
}

/*  Purpose: This slot is called in response to a Datagen object emitting a sendSensorReading signal. It adds a new reading to the 'pulseData'
 *  QVector.*/
void Session::updatePulseData(float reading) {
    pulseData.append(reading);
}

/* Purpose: Ends the Session when the selector button emits another "pressed" signal after the Session has already started. */
void Session::endSession() {
    sessionTimer->stop();

    //Create a new Log object and send it to the MainWindow.
    Log summaryLog = Log();
    summaryLog.setChallengeLevel(this->challengeLevel);
    summaryLog.setCoherenceTimes(this->coherenceTimes);
    summaryLog.setSessionLength(this->sessionLength);
    summaryLog.setAchievementScore(this->achievementScore);
    summaryLog.setPulseData(this->pulseData);
    emit sendSessionSummary(summaryLog);
}

/***IMPLEMENTING THE HELPER METHODS FOR THE SESSION CLASS***/

/*  Purpose: Every 5 seconds, this method updates the user's current coherence score, current achievement score and computes whether the
 *  current coherence score is considered "Low", "Medium" or "High", updating the 'coherenceTimes' QMap accordingly. The coherence scoring
 *  algorithm monitors only the most current 64 seconds of heart rhythm data.
*/
void Session::updateCoherence() {

    //Compute the period of motion of the last 64 seconds of heart rhythm data.
    int baselineValue = pulseData.at(0);                                        //The starting (baseline) value.
    int numCrosses = 0;                                                         //The number of times the wave crossed the midline.
    int dataSum = 0;                                                            //The sum of the data in the 64 second window.

    //Determine where to start iterating from to compute the coherence score.
    int i;
    if(sessionLength < 64) i = 1;
    else i = sessionLength - 64 + 1;

    QVector<float> recentData = QVector<float>();                               //Used to compute the normalized error.

    //Compute the number of times the pulse value is equal to the baselineValue.
    for (;i<=sessionLength;i++) {
        if(pulseData.at(i) == baselineValue) numCrosses++;
        dataSum += pulseData.at(i);
        recentData.append(pulseData.at(i));
    }

    //Calculate the period (cycles per minute).
    float period;
    if (sessionLength < 64) period = ((float) numCrosses / 2.0) / ((float) sessionLength / 60.0);
    else period = ((float) numCrosses / 2.0) / ((float) 64.0 / 60.0);

    //Calculate the coherence score.
    float error = computeNormalizedError(recentData, period, baselineValue);
    coherenceScore = qCeil((1.0 - error) * 16.0);

    //Update the achievement score.
    achievementScore += coherenceScore;

    QString newLevel;                                                       //The newly computed coherence level.

    //Determine the current coherence level.
    if (coherenceScore < CHALLENGE_THRESHOLDS[this->challengeLevel]["Low"]) newLevel = "Low";
    else if (coherenceScore > CHALLENGE_THRESHOLDS[this->challengeLevel]["High"]) newLevel = "High";
    else newLevel = "Medium";

    if(this->coherenceLevel.compare(newLevel) != 0) this->levelChanged = true;
    else this->levelChanged= false;

    this->coherenceLevel = newLevel;

    //Increment the value in 'coherenceTimes' by 5 seconds.
    coherenceTimes[this->coherenceLevel] += 5;
}

float Session::computeNormalizedError(QVector<float> data, float period, float vShift) {

    //We will compute the average error, minimum error and maximum error.
    float minError = std::numeric_limits<float>::max();
    float maxError = 0;
    float averageError = 0;

    //Compute the MSE between each data point and the corresponding point on a perfect sine function with the given 'period' and 'vShift'.
    for(int i=0;i<data.size();i++) {

        //The time at which the data point was observed.
        int seconds = sessionLength - data.size() + 1 + i;
        float currentError = (float) qPow((data.at(i) - (qSin(2.0 * (float) M_PI * period * (seconds / 60.0)) + vShift)), 2.0);
        if(currentError < minError) minError = currentError;
        if (currentError > maxError) maxError = currentError;
        currentError = currentError / data.size();
        averageError += currentError;
    }

    //Normalize the error.
    averageError = (float) (averageError - minError) / (float) (maxError - minError);

    //Increase the error if the period is not in the range of 3-15 cycles per minute.
    if (period < 3) averageError = qPow(averageError, (1.0 / ((float)(3 - period) * 8.0)));
    else if (period > 15) averageError = qPow(averageError, (1.0 / ((float) (period - 15) * 8.0)));

    return averageError;
}

/*  Purpose: This function is responsible for creating a QMap to store all of the threshold values separating "Low", "Medium" and "High"
    coherence at all 4 challenge levels.*/
void Session::initializeThresholds() {
    CHALLENGE_THRESHOLDS.insert(1, QMap<QString, float>());
    CHALLENGE_THRESHOLDS[1]["Low"] = 0.5;
    CHALLENGE_THRESHOLDS[1]["High"] = 0.9;

    CHALLENGE_THRESHOLDS[2] = QMap<QString, float>();
    CHALLENGE_THRESHOLDS[2]["Low"] = 0.6;
    CHALLENGE_THRESHOLDS[2]["High"] = 2.1;

    CHALLENGE_THRESHOLDS[3] = QMap<QString, float>();
    CHALLENGE_THRESHOLDS[3]["Low"] = 1.8;
    CHALLENGE_THRESHOLDS[3]["High"] = 4.0;

    CHALLENGE_THRESHOLDS[4] = QMap<QString, float>();
    CHALLENGE_THRESHOLDS[4]["Low"] = 4.0;
    CHALLENGE_THRESHOLDS[4]["High"] = 6.0;
}

int Session::getChallengeLevel(){return challengeLevel;}
int Session::getPacerSpeed(){return breathPacerSpeed;}

void Session::setChallengeLevel(int level){this->challengeLevel = level;}
void Session::setPacerSpeed(int speed){this->breathPacerSpeed = speed;}





