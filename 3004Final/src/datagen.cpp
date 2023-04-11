#include "datagen.h"

//Constructor and destructor for the Datagen class.
Datagen::Datagen(QString coherence, QObject* parent): QObject(parent)
{
    this->amplitude = 10;
    this->vShift = 85;
    if(!QString::compare(coherence, "Low", Qt::CaseInsensitive)) this->period = 25;
    else this->period = 10;

    QDateTime dateTime = QDateTime::currentDateTime();
    this->generator = new QRandomGenerator(dateTime.currentSecsSinceEpoch());
}

//Destructor for the Datagen class.
Datagen::~Datagen() {
    delete generator;
}

/* Purpose: This function is responsible for returning a value from the sine wave with the given frequency and vertical shift after the
    given number of seconds has elapsed.*/
void Datagen::getSensorReading(float seconds) {

    float reading = applyRandomNoise(this->amplitude * qSin(2.0 * (float) M_PI * this->period * (seconds / 60.0))) + this->vShift;
    //Sends the Sensor reading to the current Session object.
    emit sendSensorReading(reading);
}

/*Purpose: This function is responsible for changing the period of the sine wave that is used to generate values by the Datagen object. */
void Datagen::setPeriod(float period) {
    this->period = period;
}


/*Purpose: this method is a helper method used to apply random noise to the pulse readings. (NO NOISE HAS BEEN ADDED YET)*/
float Datagen::applyRandomNoise(float reading) {
    return (float) reading * (float) generator->generateDouble();

}
