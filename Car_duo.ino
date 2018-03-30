#include <SoftwareSerial.h>
#include <Servo.h>
/*при използване на тази библиотека <Servo.h> се изключва функцията analogWrite() за пинове 9 и 10

*/
//========== КОНСТАНТИ ==========
const int maxPWM = 255;
const int minPWM = 100;

const int maxPWMleft = 200;
const int maxPWMright = 200;   
const int minPWMleft = 130;
const int minPWMright = 130;                    
int deadZone = 10;


const int lSpeed = 11;
const int lForward = 12;
const int lBackward = 13;
const int rSpeed = 8;
const int rForward = 9;
const int rBackward = 10;
const int trigPin = 6;
const int echoPin = 7; 
const int servoPin = 5;
const int servoForward = 90;
const int servoRight = 10;
const int servoLeft = 170;

Servo myservo;




//========== ПРОМЕНЛИВИ ==========
SoftwareSerial BTSerial(2, 3); // Arduino's RX|TX
byte rxByte = 0;
int x, y = 0;

int leftMotor,leftMotorScaled = 0;
float leftMotorScale = 0;

int rightMotor,rightMotorScaled = 0;
float rightMotorScale = 0;

float maxMotorScale = 0;
int distance_right = 0;
int distance_left = 0;
int reach;
bool servoEnable = 1;



//========== ИНИЦИАЛИЗАЦИЯ ==========
void setup(){
  //Serial.begin(9600);
  BTSerial.begin(9600);
  pinMode(lForward, OUTPUT);
  pinMode(lBackward, OUTPUT);
  pinMode(rForward, OUTPUT);
  pinMode(rBackward, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  myservo.attach(servoPin); 
}




//========== ГЛАВНА ПРОГРАМА ==========
void loop(){
if (analogRead(A0)<512){
  //РЪЧЕН РЕЖИМ УПРАВЛЕНИЕ ПРЕЗ bluetooth
  // ---четене на bluetooth---
  if(BTSerial.available())
  {
    rxByte = BTSerial.read();

    // проверка дали полученото число е стойност за ос X (1..100)
    if((rxByte >= 1) && (rxByte <= 100))
    {
      x = rxByte;
      // коригиране на X да е в обхват -100 .. +100
      x = (x - 50) *2;
    }
    
    // проверка дали полученото число е стойност за ос Y (101..200)
    if((rxByte >= 101) && (rxByte <= 200))
    {
      y = rxByte;
      // коригиране на Y да е в обхват -100 .. +100
      y = (y - 150) *2;
    }


    // показване на крайните стойности на X и Y на компютъра (за проверка)
     //Serial.print(x); Serial.print(", "); Serial.println(y);
  }
  
  // прибиране на сензора за близост
  if (servoEnable==1){
    myservo.write(servoRight);
    servoEnable = 0;
    delay(1000);
    myservo.detach(); 
  }
   

  // ---управление на електро-мотора---

  //задаване на Y за газ и посока напред/назад;
  //задаване на X за завиване наляво/надясно;
  //инвертиране на Y и намаляване чувствителността на X
  leftMotor  = -(y) + x/2;
  rightMotor = -(y) - x/2;

  //разпределяне на заданието за скорост на моторите
  leftMotorScale =  leftMotor/100.0;
  leftMotorScale = abs(leftMotorScale);
  rightMotorScale =  rightMotor/100.0;
  rightMotorScale = abs(rightMotorScale);

  maxMotorScale = max(leftMotorScale,rightMotorScale);
  maxMotorScale = max(1,maxMotorScale);

  leftMotorScaled = constrain(leftMotor/maxMotorScale,-100,100);
  rightMotorScaled = constrain(rightMotor/maxMotorScale,-100,100);


  //управление на ляв мотор:
  if(abs(leftMotorScaled)>deadZone)
  {
    if (leftMotorScaled > 0)
    {
      //напред
      digitalWrite(lBackward,0);
      digitalWrite(lForward,1);
      //analogWrite(lSpeed,abs(leftMotorScaled));
      analogWrite(lSpeed,map(abs(leftMotorScaled),0,100,minPWM,maxPWM));
    }
    else 
    {
      //назад
      digitalWrite(lBackward,1);
      digitalWrite(lForward,0);
      //analogWrite(lSpeed,abs(leftMotorScaled)); 
      analogWrite(lSpeed,map(abs(leftMotorScaled),0,100,minPWM,maxPWM));
    }
  }  
  else 
  {
    //стоп
    digitalWrite(lBackward,0);
    digitalWrite(lForward,0);
    analogWrite(lSpeed,0); 
  } 

  //управление на десен мотор:
  if(abs(rightMotorScaled)>deadZone)
  {
    if (rightMotorScaled > 0)
    {
      //напред
      digitalWrite(rBackward,0);
      digitalWrite(rForward,1);
      //analogWrite(rSpeed,abs(rightMotorScaled));
      analogWrite(rSpeed,map(abs(rightMotorScaled),0,100,minPWM,maxPWM));
    }
    else 
    {
      //назад
      digitalWrite(rBackward,1);
      digitalWrite(rForward,0);
      //analogWrite(rSpeed,abs(rightMotorScaled));
      analogWrite(rSpeed,map(abs(rightMotorScaled),0,100,minPWM,maxPWM));
    }
  }  
  else 
  {
    //стоп
    digitalWrite(rBackward,0);
    digitalWrite(rForward,0);
    analogWrite(rSpeed,0); 
  } 

} 

//АВТОНОМЕН РЕЖИМ

else{
  
   if (servoEnable==0){
    myservo.attach(servoPin); 
    servoEnable = 1;
    
  }
  
   distance_left = 0;
   distance_right = 0;
  
  myservo.write(servoForward);
  delay(500);

  reach = getDistance();
//  BTSerial.println(reach);

// придвижване напред 1
  if(reach > 30 && reach <= 150){  
      
    move_forward(minPWMleft,minPWMright);
    delay(500);
    move_stop();
    
//    BTSerial.println("forward");
    }
 // придвижване напред 2
      if(reach > 150 ){  
      
    move_forward(maxPWMleft,maxPWMright);
    
//    BTSerial.println("forward");
  }

  if(reach >=10 && reach <=30){
    move_stop();
    myservo.write(servoLeft);
    delay(1050);
    distance_left = getDistance();
    
    myservo.write(servoRight);
    delay(1050);
    distance_right = getDistance();

  }
  
  if(distance_right > distance_left){
    turn_right();
//    BTSerial.println("right");
    move_stop();
  }
  if(distance_right < distance_left){
    turn_left();
//    BTSerial.println("left");
    move_stop();
  }
  if(reach < 10){
    move_stop();
//    BTSerial.println("stop");
//    BTSerial.println(getDistance());
    move_back();
  }
  
  
}



}





//========== ФУНКЦИИ ==========


int getDistance()
{
  int distance = 0;
  long duration = 0;
  //clears the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration*0.034/2; 
  return(distance);
}

void turn_right(){
    digitalWrite(lForward, 1);   
    digitalWrite(rForward, 0);    
    digitalWrite(lBackward,0);
    digitalWrite(rBackward,1);     
    analogWrite(lSpeed, maxPWMleft);
    analogWrite(rSpeed, maxPWMright);
    delay(300);

    
   }
   
void turn_left(){
    digitalWrite(lForward, 0);   
    digitalWrite(rForward, 1);    
    digitalWrite(lBackward,1);
    digitalWrite(rBackward,0);   
    analogWrite(lSpeed, maxPWMleft);
    analogWrite(rSpeed, maxPWMright);
    delay(300);
    }
    
void move_stop(){
    analogWrite(lSpeed, 0);
    analogWrite(rSpeed, 0);    
   }
   
void move_forward(int PWMleft, int PWMright){
    digitalWrite(lForward, 1);   
    digitalWrite(rForward, 1);    
    digitalWrite(lBackward,0);
    digitalWrite(rBackward,0);    
    analogWrite(lSpeed, PWMleft);
    analogWrite(rSpeed, PWMright);
   }
   
void move_back(){
    digitalWrite(lForward, 0);   
    digitalWrite(rForward, 0);    
    digitalWrite(lBackward,1);
    digitalWrite(rBackward,1);    
    analogWrite(lSpeed, maxPWMleft);
    analogWrite(rSpeed, maxPWMright);
    delay(500);
    analogWrite(lSpeed, 0);
    analogWrite(rSpeed, 0);
   }
