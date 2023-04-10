#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QDateTime>
#include <QtMath>
#include <limits>
#include "log.h"

class Session: public QObject {

    Q_OBJECT

    public:

        /*Stores the thresholds for "Medium" coehrence in each of the 4 challenge levels. Each entry will be of the following form:
         * {<challenge level>:{"Low": <lower bound of threshold>, "High": <upper bound of threshold>}}
        */
        static QMap<int, QMap<QString, float>> CHALLENGE_THRESHOLDS;

        //Constructor and Destructor.
        Session(int challengeLevel=1, int breathPacerSpeed = 10, QObject* parent=0);
        ~Session();

        //Getter methods
        int getChallengeLevel();
        int getPacerSpeed();

        //Setter methods
        void setChallengeLevel(int level);
        void setPacerSpeed(int speed);

    signals:
        void updateSessionDisplay(Log currentLog);
        void getSensorReading(float secondsElapsed);           //Gets a reading from the Datagen after 'secondsElapsed' seconds into the Session.
        void sendSessionSummary(Log summaryLog);
    public slots:
        void updateSessionData();
        void updatePulseData(float reading);
        void beginSession();
        void endSession();
    private:
        //SETTINGS RELATED
        int challengeLevel;                                     //A value between 1 and 4 that determines the thresholds between low,
                                                                //medium and high coherence.

        int breathPacerSpeed;                                   //A value between 1 and 30 indicating the time interval between each breath

        //METRICS RELATED
        QVector<float> pulseData;                               //Keeps track if the most recent 64 seconds of pulse data.
        float coherenceScore;                                   //The most recently computed coherence score
        int sessionLength;                                      //How long the session has been active for, in seconds.
        QTimer* sessionTimer;                                   //Used to keep track of time.
        float achievementScore;                                 //The current achievement score.
        QString coherenceLevel;                                 //One of "Low", "Medium", "High" or "NA" before the first coherence score is calculated.
        bool levelChanged;                                      //Whether or not a new coherence level was reached.
        QMap<QString, int> coherenceTimes;                      //Keeps track of the time spent in "Low", "Medium" and "High" coherence.

        //Private helper methods for the Session class.
        void updateCoherence();
        float computeNormalizedError(QVector<float> data, float period, float vShift);
        void initializeThresholds();

};

#endif // SESSION_H
