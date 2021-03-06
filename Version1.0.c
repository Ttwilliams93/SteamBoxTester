// 12.21.2018
// Troy Williams
// Steam Box Code Automation Project

// Include Necessary Libraries
#include <Controllino.h>

// Initialize Necessary Pins
// Inputs
int InductiveSensor = CONTROLLINO_AI0;
int AutomaticFunction = CONTROLLINO_AI1;
int ManualFunction = CONTROLLINO_AI2;
int ResetSwitch = CONTROLLINO_IN0;
int CO2Sensor = CONTROLLINO_AI3;
// Outputs
int S1SteamTube = CONTROLLINO_DO2;
int S2AirWipe = CONTROLLINO_DO1;
int S3Clamp = CONTROLLINO_DO0;
int TA340Power = CONTROLLINO_R9;
//int FanPower = CONTROLLINO_DO3;

//Varibles
int TA340Status;
int AutomaticStatus;
int ManualStatus;
int ISStatus;
int ResetSwitchStatus;
int CO2SensorStatus;
int AirWipeActivationLength = 3000; //millis
int ClampDelay = 750; //millis
//int FanPowerLevel=150; //0-255, do not put above 200
int AutomaticDelayLength = 10000;
unsigned long PreviousMillis;
// Fan Connects to 24v and 0v terminal blocks, always on  NO GOOD, NEED PWM OR 12V OUTPUT

// The switch in the back of unit will be power.  The switch on the front of the unit is for selection of the modes.  The button on the front of the unit will serve as the reset button.

/* The steambox has two major operation modes.  The first mode is automatic.  The wire will be inserted
    and then the clamp will activate after a specified delay.  The steam tube will also move forwards to bring
    the wire into the steam zone.  The clamp will remain locked until the CO2 laser indicates it is
    complete.  At this point the air wipe will turn on and the clamp will release so the user can remove the
    piece of wire. During this time the release button will remain active and the user will be able to
    press release at anytime to release the lock.  This button will restart
    the system and clear all variables.

    The other mode will be manual.  The user will stick the wire into the opening and the machine will clamp.
    The machine will remain clamped until the user presses the reset button, this will release the clamp and
    move the steam back into its home position.  It will also return the unit to the state where it is ready
    to grab the piece again.   This mode will ignore the CO2 laser results.
    Air wipe will turn off after about 2-3 seconds in both situations.
*/
void setup() {
  digitalWrite(TA340Power, HIGH); //Turn on the power to keyence sensor through a relay
  // Release all air solenoids
  digitalWrite(S1SteamTube, LOW);
  digitalWrite(S2AirWipe, LOW);
  digitalWrite(S3Clamp, LOW);
  // Initialize Pins
  pinMode(S1SteamTube, OUTPUT);
  pinMode(S2AirWipe, OUTPUT);
  pinMode(S3Clamp, OUTPUT);
  pinMode(TA340Power, OUTPUT);
  //pinMode(FanPower, OUTPUT);
  pinMode(InductiveSensor, INPUT);
  pinMode(AutomaticFunction, INPUT);
  pinMode(ManualFunction, INPUT);
  pinMode(ResetSwitch, INPUT);
  //analogWrite(FanPower, FanPowerLevel);
  Serial.begin(9600);
}

void loop() {
  // Determine Switch Function States
  ManualStatus = digitalRead(ManualFunction);
  AutomaticStatus = digitalRead(AutomaticFunction);
  ResetSwitchStatus = digitalRead(ResetSwitch);
  CO2SensorStatus = digitalRead(CO2Sensor);
  ISStatus = digitalRead(InductiveSensor);

  if (ManualStatus == HIGH && ISStatus == HIGH) {
    Delay(ClampDelay);
    ActivateSolenoids(S3Clamp, S1SteamTube);

    while (ResetSwitchStatus == LOW) {
      ResetSwitchStatus = digitalRead(ResetSwitch); //Need to debounce this switch,DO YOU THO? ITS IN A WHILE LOOP
    }

    ReleaseAndAirWipe();
    Delay(AirWipeActivationLength);
    digitalWrite(S2AirWipe, LOW);

    while (ISStatus == HIGH) {   //Stop from returning to loop until inductive sensor clears. - Doesn't seem necessary, just good practice.
      ISStatus = digitalRead(InductiveSensor);
    }
  }

  if (AutomaticStatus == HIGH && ISStatus == HIGH) {  //FUNCTIONS OFF A TIME DELAY OR CO2 SENSOR SIGNAL
    Delay(ClampDelay);
    ActivateSolenoids(S3Clamp, S1SteamTube);
    PreviousMillis = millis();

    while (AutomaticConditional(ResetSwitchStatus, AutomaticDelayLength, CO2SensorStatus)) {
      ResetSwitchStatus = digitalRead(ResetSwitch); //Need to debounce this switch, DO YOU THO? ITS IN A WHILE LOOP
      CO2SensorStatus = digitalRead(CO2Sensor);
    }

    ReleaseAndAirWipe();
    Delay(AirWipeActivationLength);
    digitalWrite(S2AirWipe, LOW);

    while (ISStatus == HIGH) {   //Stop from returning to loop until inductive sensor clears.
      ISStatus = digitalRead(InductiveSensor);
    }
  }
}

void ReleaseAndAirWipe() {
  digitalWrite(S3Clamp, LOW);
  digitalWrite(S1SteamTube, LOW);
  digitalWrite(S2AirWipe, HIGH);
}

void Delay(unsigned long DelayTime) {
  unsigned long PreviousMillis = millis();
  while ((unsigned long)(millis() - PreviousMillis) < DelayTime) {
    //Delay For passed through time
  }
}

void ActivateSolenoids(int Object1, int Object2) {
  digitalWrite(Object1, HIGH);
  digitalWrite(Object2, HIGH);
}

int AutomaticConditional(int Switch, int Delay, int Sensor) {
  if (
    Switch == HIGH ||
    Sensor == HIGH ||
    (unsigned long)(millis() - PreviousMillis) > Delay
  ) {
    return false;
  }
  
  return true
}
