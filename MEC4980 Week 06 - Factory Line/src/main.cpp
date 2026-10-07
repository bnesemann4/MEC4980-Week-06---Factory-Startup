#include <P1AM.h>
#include <math.h>

int modInput = 1;
int modOutput = 2;
int modAnalogIn = 3;

int pulseKey = 1;
int pinLB1 = 2;
int pinLB2 = 3;
int pinLBW = 4;
int pinLBR = 5;
int pinLBB = 6;

int linkageDistances[] = {3, 7, 12};
int valvePins[] = {3, 4, 5};

enum MachineStates {
  REST,
  SENSE,
  TRACKER,
  ACTUATE,
  COUNT
};

enum Colors {
  WHITE,
  RED,
  BLUE,
  COLORCOUNT
};

MachineStates mState = REST;
Colors targetColor = WHITE;

void setup(){
  Serial.begin(115200);
  while (!P1.init()){ 
  }
}

void TurnEverythingOff() {
   for (int i = 1; i < 6; i++) {
    P1.writeDiscrete(LOW,modOutput,i);
  }
}

void ToggleConveyor(bool onOffState) {
  P1.writeDiscrete(onOffState,modOutput,1);
}

void ToggleCompressor(bool onOffState) {
  P1.writeDiscrete(onOffState,modOutput,2);
}

void UpdateRobotArmOutputs() {
  for (int i = 4; i < 7; i++) {
    P1.writeDiscrete(!P1.readDiscrete(modInput,i),modOutput,i+2);
  }
}


int channelTwo;
int color = 0;
int linkageCount = 0;
bool prevKeyState = false;
bool currentState = false;
int distanceToMove = 8;
int defWrongColor = 10000;
int currentColor = 10000;

void loop(){
  UpdateRobotArmOutputs();
  switch (mState)
  {
  case MachineStates::REST:
    TurnEverythingOff();
    if (!P1.readDiscrete(modInput,pinLB1)) {
      mState = MachineStates::SENSE;
    }
    break;
  case MachineStates::SENSE:
    currentColor = min(currentColor,P1.readAnalog(modAnalogIn,1));
    Serial.print("Sense state, min color: ");
    Serial.println(currentColor);
    ToggleConveyor(HIGH);
    if (!P1.readDiscrete(modInput,pinLB2)) {
      mState = MachineStates::TRACKER;
      if (currentColor < 3000) {
        targetColor = Colors::WHITE;
      } else if (currentColor < 4800) {
        targetColor = Colors::RED;
      } else {
        targetColor = Colors::BLUE;
      }
    }
    break;
  case MachineStates::TRACKER:
    ToggleCompressor(HIGH);
    Serial.print("Tracking state");
    currentState = bool(P1.readDiscrete(modInput,pulseKey));
    if (!prevKeyState && currentState) {
      linkageCount++;
    }
    prevKeyState = currentState;
    if (linkageCount > linkageDistances[int(targetColor)]) {
      mState = MachineStates::ACTUATE;
    }
    break;
  case MachineStates::ACTUATE:
    Serial.print("Actuate state");  
    ToggleConveyor(LOW);
    P1.writeDiscrete(HIGH,modOutput,valvePins[(int)targetColor]);
    delay(1000);
    mState = MachineStates::REST;
    currentColor = defWrongColor;
    linkageCount = 0;
    break;
  case MachineStates::COUNT:
    break;
  default:
    Serial.print("Default State");
    break;
  }
/*	Serial.print("pulse, 1, 2, W, R, B, color: ");
  for (int i = 1; i < 7; i++) {
    channelTwo = P1.readDiscrete(modInput,i);
	  Serial.println(channelTwo);
    Serial.print(", ");
  }
  color = P1.readAnalog(modAnalogIn, 1);
  Serial.println(color);

  for (int i = 1; i < 6; i++) {
    Serial.print("Running device ");
    Serial.println(i);
    P1.writeDiscrete(HIGH,modOutput,i);
    delay(1000);
    P1.writeDiscrete(LOW,modOutput,i);
  } */


}