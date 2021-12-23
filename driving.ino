//dependencies
extern "C"{
  #include "Tinn.h"
};

using namespace std;
#include <string>
#include <Wire.h>
#include <LOLIN_I2C_MOTOR.h>

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
#define IN_NODES  5
#define HID_NODES 7
#define OUT_NODES 2

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
int distance5;

//distance one 4bit measure is measured in
int edgedist = 11;

//pre trained biases to load into the neural network
float biases[] = {
  0.449327, 0.025995
};

//pre trained weights to load into the neural network
float weights[] = {
  -1.353206, -1.144721, -1.534024, 4.552975, 4.313193, -3.452873, -3.525798,
  -1.213497, -5.115411, -3.141104, 2.816752, -3.988078, -4.040601, 3.185037,
  2.281164, 1.142817, 3.582944, 2.883088, 3.877965, 3.575855, -1.444307,
  0.216100, -0.085072, -0.573343, -0.102276, 1.347645, -2.953274, -1.953251,
  5.553272, 5.076849, 1.233514, -2.774449, -4.557082, 1.605104, 0.452765,
  -6.916440, -6.596384, 3.146082, 5.136613, 1.361658, -5.477478, 3.917717,
  -2.063781, -0.178655, -4.185128, -2.750510, 3.857427, 3.385339, 3.657549
};

//an instance of tinn, short for tiny neural network
Tinn tinn ;

//gets called once when running code at the beginning
void setup() {

  //delay so the car doesn't take off out of nowhere
  delay(10000);
  
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
  tinn = xtbuild(IN_NODES, HIDDEN_NODES, OUT_NODES);

  //fill in the biases
  for(int i=0; i < tinn.nb; i++)
    tinn.b[i] = biases[i];

  //fill in the weights
  for(int j=0; j < tinn.nw; j++)
    tinn.w[j] = weights[j];

  //User Interface
  Serial.println("VERSTAPPEN SETUP COMPLETE");
  Serial.println("Happy Racing Max!");
}

float* predict_output(int dis1, int dis2, int dis3, int dis4, int dis5) {

  //empty at first
  float verstappen_inputs[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };

  //inputs are the 5 sensor distances
  verstappen_inputs[1] = dis1;
  verstappen_inputs[2] = dis2;
  verstappen_inputs[3] = dis3;
  verstappen_inputs[4] = dis4;
  verstappen_inputs[5] = dis5;

  //feed the inputs to the neural network
  return xtpredict(tinn,verstappen_inputs);
}

//unused since 4bit conversion
int turn_to_binary(int dist) {
  if (dist <= edgedist)
  {
    return 1;
  }
  return 0;
}

int turn_to_4bit(int dist) {
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

void loop() {
  Serial.println("test1");
  //SENSOR SECTION
  // Clears the trigPin condition
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  Serial.println("test2");
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration1 = pulseIn(echoPin1, HIGH);
  duration2 = pulseIn(echoPin2, HIGH);
  duration3 = pulseIn(echoPin3, HIGH);
  duration4 = pulseIn(echoPin4, HIGH);
  duration5 = pulseIn(echoPin5, HIGH);
  
  // Calculating the distance
  distance1 = duration1 * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  distance2 = duration2 * 0.034 / 2;
  distance3 = duration3 * 0.034 / 2;
  distance4 = duration4 * 0.034 / 2;
  distance5 = duration5 * 0.034 / 2;
  
  //TODO: outlierdetection for all 5??

  //4bit conversion
  distance1 = turn_to_4bit(distance1);
  distance2 = turn_to_4bit(distance2);
  distance3 = turn_to_4bit(distance3);
  distance4 = turn_to_4bit(distance4);
  distance5 = turn_to_4bit(distance5);
  
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
    steering = steering - 0.5
  }

  //make steering of range [0, 100]
  steering = steering * 2

  //TODO: compenstate steering for being in half ranges (write code for both ways)
  
  if (out[0] != 0)
  {
      motor.changeDuty(MOTOR_CH_A, steering);
  }

  //create power value
  int power = int(out[1] * 100); 

  //DEBUG output of neural network
  Serial.println("steering");
  Serial.println(steering);
  Serial.println("power");
  Serial.println(power);

  motor.changeDuty(MOTOR_CH_B, power);
}
