#ifndef LOG_H
#define LOG_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QtMath>

/*  Purpose: This class is meant to */
class Log {

    public:
        //Constructor and destructor
        Log();


        //Getter methods
        const QDateTime& getDateTime() const;
        int getChallengeLevel();
        int getPacerSpeed();
        QMap<QString, float> getCoherenceDistribution();
        int getSessionLength();
        float getAchievementScore();
        QVector<float> getPulseData();
        float getCoherenceScore();
        QString getCoherenceLevel();
        float getAverageCoherence();
        bool isLevelChanged();

        //Setter methods.
        void setChallengeLevel(int level);
        void setPacerSpeed(int speed);
        void setCoherenceTimes(QMap<QString, int> times);
        void setSessionLength(int length);
        void setAchievementScore(float score);
        void setPulseData(QVector<float> data);
        void setCoherenceScore(float score);
        void setCoherenceLevel(QString level);
        void setLevelChanged(bool isChanged);
    private:
        QDateTime date;                                 //The date the session was recorded.
        int challengeLevel;                             //The challenge level used for the session.
        int breathPacerSpeed;                           //The breath pacer speed used for the session.
        QMap<QString, int> coherenceTimes;              //Times spent in "Low", "Medium" and "High" coherence (in seconds).
        int sessionLength;                              //The total length of the session (in seconds) so far.
        float achievementScore;                         //The current achievement score.
        QVector<float> pulseData;                       //Pulse data from the Session.
        float coherenceScore;                           //The current coherence score.
        QString coherenceLevel;                         //The current coherence level.
        bool levelChanged;                              //Whether or not the coherence level changed (device should beep if it did)
};

#endif // LOG_H
