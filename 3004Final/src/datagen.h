#ifndef DATAGEN_H
#define DATAGEN_H

#include <QtMath>
#include <QObject>
#include <QRandomGenerator>
#include <QDateTime>

class Datagen: public QObject
{
    Q_OBJECT

    public:
        //Constructor and destructor
        Datagen(QString coherence, QObject* parent=0);
        ~Datagen();
    signals:
        void sendSensorReading(float reading);      //Used to send a new sensor reading to update a Session object.

    public slots:
        void getSensorReading(float seconds);
        void setPeriod(float period);

    private:
         float amplitude;                        //The amplitude of the Sine wave.
         float period;                           //The period of the waves to be measured. The period is recorded as the number of cycles per minute.
         float vShift;                           //Shifts the wave up to resemble real heartrate values.
         QRandomGenerator* generator;            //Random number generator

         //Helper methods
         float applyRandomNoise(float reading);
};

#endif // DATAGEN_H
