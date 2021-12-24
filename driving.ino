//dependencies
extern "C"
{
#include "Tinn.h"
};

using namespace std;
#include <string>

//motor
#include <Wire.h>
#include <LOLIN_I2C_MOTOR.h>

//memory
#include <EEPROM.h>

//engine
#define PWM_FREQUENCY 1000
LOLIN_I2C_MOTOR motor(DEFAULT_I2C_MOTOR_ADDRESS); //I2C address 0x30 SEE NOTE BELOW

//sensor state
#define trigPin1 TX
#define echoPin1 D0 //R1
#define echoPin2 D5 //L1
#define echoPin3 D6 //L2
#define echoPin4 D7 //M
#define echoPin5 RX //R2

// params for the NN
#define IN_NODES 5
#define HID_NODES 7
#define OUT_NODES 2

////////////
//SETTINGS//
////////////
bool serial = true;
bool memory = true;

// variables for the duration of sound wave travel
long duration1;
long duration2;
long duration3;
long duration4;
long duration5;

// variables for the distance measurements
int distance1;
int distance2;
int distance3;
int distance4;
int distance5 = 0;

//which address we are going to write to in the EEPROM
int address = 0;

//whether or not we have crashed and should hence stope telemetrics
bool crashed = false;

//distance one 4bit measure is measured in
int edgedist = 18;

//pre trained biases to load into the neural network
float biases[] = {
    0.449327, 0.025995};

//pre trained weights to load into the neural network
float weights[] = {
  -1.353206, -1.144721, -1.534024, 4.552975, 4.313193, -3.452873, -3.525798, -1.213497, -5.115411, -3.141104, 2.816752, -3.988078, -4.040601, 3.185037, 2.281164, 1.142817, 3.582944, 2.883088, 3.877965, 3.575855, -1.444307, 0.216100, -0.085072, -0.573343, -0.102276, 1.347645, -2.953274, -1.953251, 5.553272, 5.076849, 1.233514, -2.774449, -4.557082, 1.605104, 0.452765, -6.916440, -6.596384, 3.146082, 5.136613, 1.361658, -5.477478, 3.917717, -2.063781, -0.178655, -4.185128, -2.750510, 3.857427, 3.385339, 3.657549
//    2.506542,
//    -6.167351,
//    3.076371,
//    9.333115,
//    4.089797,
//    0.301825,
//    0.458969,
//    -0.288023,
//    -0.089155,
//    0.376617,
//    -0.564076,
//    0.343862,
//    -0.399771,
//    -0.173858,
//    0.373769,
//    4.540301,
//    0.267299,
//    6.579165,
//    0.196806,
//    -4.433230,
//    -1.436299,
//    -1.257108,
//    1.385680,
//    0.921490,
//    -1.217654,
//    0.528074,
//    -0.156684,
//    0.634125,
//    3.112460,
//    1.306892,
//    6.221663,
//    -0.042466,
//    -0.407600,
//    -1.396802,
//    -2.716314,
//    -3.441118,
//    1.018185,
//    1.610596,
//    -5.823295,
//    8.275008,
//    -3.115408,
//    7.649559,
//    0.636142,
//    7.771691,
//    -3.960641,
//    -0.190234,
//    2.008600,
//    -4.902290,
//    -2.285367,
};

//an instance of tinn, short for tiny neural network
Tinn tinn;

//gets called once when running code at the beginning
void setup()
{
    //memory read
    if (serial == true)
    {
        for (int i = 0; i < 216; i = i + 2)
        {
            // TODO: is there a problem if memory is empty and we try to read it????
            int ste = EEPROM.read(i);
            int pow = EEPROM.read(i + 1);

            //if cycle completed
            if (i == 0 || ste == 214)
            {
                Serial.println("FILLED CYCLE DETECTED!!!!!!");
            }

            //if value reachted bounds, exit loop
            if (ste == 246)
            {
                break;
            }

            //if vehicle crashed, exit loop
            if (ste == 219 || pow == 219)
            {
                Serial.println("THE VEHICLE CRASHED!!!!!");
                break;
            }

            Serial.println("STEERING");
            Serial.println(ste);
            Serial.println("POWER");
            Serial.println(pow);
        }
    }

    //memory reset
    if (memory == true)
    {
        //setup memory
        EEPROM.begin(512);

        //we fill the memory with 246 values
        //this way if we read 246 we know we reached the end of output
        for (int i = 0; i < 512; i++)
        {
            EEPROM.write(i, 246);
        }

        //save
        EEPROM.commit();
    }

    //delay so the car doesn't take off out of nowhere
    delay(8000);

    //pinmode settings
    pinMode(trigPin1, OUTPUT); // Sets the trigPin as an OUTPUT

    // Sets the echoPins as INPUTS
    pinMode(echoPin1, INPUT);
    pinMode(echoPin2, INPUT);
    pinMode(echoPin3, INPUT);
    pinMode(echoPin4, INPUT);
    pinMode(echoPin5, INPUT);

    //Baudrate = 115200, note this while using serial monitor
    Serial.begin(115200);

    // Setup the neural network structures, as defined in defines
    tinn = xtbuild(IN_NODES, HID_NODES, OUT_NODES);

    //fill in the biases
    for (int i = 0; i < tinn.nb; i++)
        tinn.b[i] = biases[i];

    //fill in the weights
    for (int j = 0; j < tinn.nw; j++)
        tinn.w[j] = weights[j];

    //User Interface 
    if (serial == true)
    {
        Serial.println("VERSTAPPEN SETUP COMPLETE");
        Serial.println("Happy Racing Max!");
    }
}

float *predict_output(int dis1, int dis2, int dis3, int dis4, int dis5)
{

    //empty at first
    float verstappen_inputs[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

    //inputs are the 5 sensor distances
    verstappen_inputs[1] = dis4;
    verstappen_inputs[2] = dis2;
    verstappen_inputs[3] = dis3;
    verstappen_inputs[4] = dis1;
    verstappen_inputs[5] = 0;

    //feed the inputs to the neural network
    return xtpredict(tinn, verstappen_inputs);
}

//unused since 4bit conversion
int turn_to_binary(int dist)
{
    if (dist <= edgedist)
    {
        return 1;
    }
    return 0;
}

int turn_to_4bit(int dist)
{
    if (dist <= edgedist)
    {
        return 1;
    }
    else if (dist <= 2 * edgedist)
    {
        return 2;
    }
    else if (dist <= 3 * edgedist)
    {
        return 3;
    }
    return 4;
}

void loop()
{
//    //SENSOR SECTION
//    // Clears the trigPin condition
//    digitalWrite(trigPin1, LOW);
//    delayMicroseconds(2);
//    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
//    digitalWrite(trigPin1, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(trigPin1, LOW);

//    // Reads the echoPin, returns the sound wave travel time in microseconds
//    duration1 = pulseIn(echoPin1, HIGH);
//
//

    
    digitalWrite(trigPin1, LOW);
    delayMicroseconds(2);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);

    duration4 = pulseIn(echoPin4, HIGH, 100000);
    
    digitalWrite(trigPin1, LOW);
    delayMicroseconds(2);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);

    delayMicroseconds(100);

    duration2 = pulseIn(echoPin2, HIGH, 100000);    

    digitalWrite(trigPin1, LOW);
    delayMicroseconds(2);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);

    delayMicroseconds(100);

    duration1 = pulseIn(echoPin1, HIGH, 100000);  

  
    digitalWrite(trigPin1, LOW);
    delayMicroseconds(2);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);
//    
//    delayMicroseconds(50);
//    digitalWrite(trigPin1, LOW);
//    delayMicroseconds(2);
//    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
//    digitalWrite(trigPin1, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(trigPin1, LOW);
//    
      duration3 = pulseIn(echoPin3, HIGH);

//    delayMicroseconds(10);
//    // Clears the trigPin condition
//    digitalWrite(trigPin1, LOW);
//    delayMicroseconds(2);
//    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
//    digitalWrite(trigPin1, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(trigPin1, LOW);
//
//    // Reads the echoPin, returns the sound wave travel time in microseconds
//    duration1 = pulseIn(echoPin1, HIGH);    
//    digitalWrite(trigPin1, LOW);
//    delayMicroseconds(2);
//    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
//    digitalWrite(trigPin1, HIGH);
//    delayMicroseconds(10);
//    digitalWrite(trigPin1, LOW);
//    
//    duration5 = pulseIn(echoPin5, HIGH);

    // Calculating the distance
    distance4 = duration4 * 0.034 / 2;
    distance2 = duration2 * 0.034 / 2;
    distance1 = duration1 * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
    distance3 = duration3 * 0.034 / 2;

    //outlierdetection
    if (distance1 == 0)
    {
      distance1 = 200;
    }
    if (distance2 == 0)
    {
      distance2 = 200;
    }
    if (distance3 == 0)
    {
      distance3 = 200;
    }
    if (distance4 == 0)
    {
      distance4 = 200;
    }
    if (distance5 == 0)
    {
      distance5 = 200;
    }

//    //4bit conversion
    distance1 = turn_to_binary(distance1);
    distance2 = turn_to_binary(distance2);
    distance3 = turn_to_binary(distance3);
    distance4 = turn_to_binary(distance4);
    distance5  = turn_to_binary(distance5);

    //M, L, LL, R, RR
      Serial.println(distance4);
      Serial.println(distance2);
      Serial.println(distance1);
      Serial.println(distance3);
      Serial.println("Sensors end");

    //save output of the neural network into out, so we can use it to drive the actual car
    float *out = predict_output(distance1, distance2, distance3, distance4, distance5);

    //set frequency to hardcoded value
    motor.changeFreq(MOTOR_CH_BOTH, PWM_FREQUENCY);

    //mode = drive ahead
    motor.changeStatus(MOTOR_CH_B, MOTOR_STATUS_CW);

    //create steering value
    int steering = int(out[0] * 100);

    //steering
    if (out[0] == 0.0)
    {
        //placeholder code
        int i = 3;
    }
    else if (out[0] <= 0.5)
    {
        motor.changeStatus(MOTOR_CH_A, MOTOR_STATUS_CCW);
    }
    else if (out[0] > 0.5)
    {
        motor.changeStatus(MOTOR_CH_A, MOTOR_STATUS_CW);

        //make steering of range [0, 0.5]
        steering = steering - 0.5;
    }

    //make steering of range [0, 100]
    steering = steering * 20; //* 10s

    motor.changeDuty(MOTOR_CH_A, steering);

    //create power value
    int power = int(out[1] * 20);

    if (power == 0)
    {
        crashed = true;
    }

    //communicate power to motor
    motor.changeDuty(MOTOR_CH_B, power);

    //DEBUG output of neural network
    if (serial == true)
    {
        Serial.println("steering");
        Serial.println(steering);
        Serial.println("power");
        Serial.println(power);
    }
}
