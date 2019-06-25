/************************** Aerial Gripper Code ***********************/
/************************** Author: Jiawei Meng ***********************/
/*************************** Date: 14/06/2019 *************************/
/*** Description: This code is the first version for aerial gripper ***/

#include <DynamixelWorkbench.h>  

/******************** Definitions of parameters ***********************/

#if defined(__OPENCM904__)
  #define DEVICE_NAME "1"               //Dynamixel on Serial3(USART3)  <-OpenCM 485EXP or Dynamixel on Serial1(USART1)  <-OpenCM 9.04
#elif defined(__OPENCR__)
  #define DEVICE_NAME ""
#endif 

#define BAUDRATE  57600

#define IR_PIN 1                        //Define the pin of the IR sensor
#define SWITCHER_PIN 2                  //Define the pin of the switcher

#define TRIGGER 0                       //Define the state of the switcher
#define HOVER 1

DynamixelWorkbench dxl_wb;

int val_ir = 0;
int switcher = 0;

uint8_t dxl_id1 = 2;
uint8_t dxl_id2 = 1;

uint16_t model_number1 = 0;
uint16_t model_number2 = 0;


/************************* Set intial status **************************/
void setup() 
{ 
  Serial.begin(BAUDRATE);                                  //Set the Baud rate of the serial port
  
  while(!Serial);                                          // Wait for Opening Serial Monitor

  const char *log;                                         // The code start to initialise motor 1 and motor 2
  bool result = false;

  result = dxl_wb.init(DEVICE_NAME, BAUDRATE, &log);        //Intialise the OpenCM9.04 board
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to init the OpenCM9.04");
  }
  else
  {
    Serial.print("Succeeded to init OpenCM9.04: ");
    Serial.println(BAUDRATE);  
  }

  result = dxl_wb.ping(dxl_id1, &model_number1, &log);       //Ping the 1st DYNAMIXEL motor        
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to ping motor 1");
  }
  else
  {
    Serial.println("Succeeded to ping motor 1");
    Serial.print("id : ");
    Serial.print(dxl_id1);
    Serial.print(" model_number : ");
    Serial.println(model_number1);
  }

  result = dxl_wb.ping(dxl_id2, &model_number2, &log);        //Ping the 2nd DYNAMIXEL motor          
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to ping motor 2");
  }
  else
  {
    Serial.println("Succeeded to ping motor 2");
    Serial.print("id : ");
    Serial.print(dxl_id2);
    Serial.print(" model_number : ");
    Serial.println(model_number2);
  }

  result = dxl_wb.currentBasedPositionMode(dxl_id1, 200, &log);  //Set current-based position control for motor 1 with 30 mA current limitation
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to change current based position mode for motor 1");
  }
  else
  {
    Serial.println("Succeed to change current based position mode for motor 1");
  }

  result = dxl_wb.currentBasedPositionMode(dxl_id2, 200, &log);  //Set current-based position control for motor 2 with 30 mA current limitation
  if (result == false)
  {
    Serial.println(log);
    Serial.println("Failed to change current based position mode for motor 2");
  }
  else
  {
    Serial.println("Succeed to change current based position mode for motor 2");
  }

  dxl_wb.goalPosition(dxl_id1, (int32_t)0);           //Set the initial positions of motor 1

  dxl_wb.goalPosition(dxl_id2, (int32_t)920);         //Set the initial positions of motor 2
  
  pinMode(IR_PIN, INPUT_ANALOG);                      //Set port 1 to read the analog value of IR sensor

  pinMode(SWITCHER_PIN, INPUT_PULLDOWN);              //Set port 2 to read the value of the switcher ('0' is pushing state and '1' is idle state)

}


/************************* Main loop function *************************/
void loop() 
{ 
  val_ir = readIR();
  
  switcher = digitalRead(SWITCHER_PIN);

  Serial.print("switcher is ");
  
  Serial.println(switcher);

  // If the switcher is not closed, then the palm starts to open
  if(switcher == HOVER)
  {
    int lock_interval_1 = 0;
    int load_interval = 0;
  
    dxl_wb.goalPosition(dxl_id1, (int32_t)0);
    
    delay(1000);

//    dxl_wb.goalPosition(dxl_id2, (int32_t)820);
    
    for(int count = 0; count < 8; count++)                        //Locking the quick release device
    {
        dxl_wb.goalPosition(dxl_id1, (int32_t)lock_interval_1);
        delay(500);
        dxl_wb.goalPosition(dxl_id2, (int32_t)820);
        delay(500);
        lock_interval_1 = lock_interval_1 + 40;
    }
    
    delay(1000);

    while(switcher == HOVER)
    {
      load_interval = load_interval + 10;
      
      dxl_wb.goalPosition(dxl_id1, (int32_t)load_interval);
      
      switcher = digitalRead(SWITCHER_PIN);
    }
    
    // If the switcher is closed and an object is sensed within range, then the palm starts to close
    while(val_ir < 600)
    {
      val_ir = readIR();
    }
    dxl_wb.goalPosition(dxl_id2, (int32_t)920);
  
    delay(10000);
  }
}



/**************************** Read IR sensor **************************/
int readIR()
{
  int sensor_val = 0;
  
  for(int i = 0; i < 10; i++)
  {
    sensor_val = sensor_val + analogRead(IR_PIN);              //Read the value of IR sensor from 'IR_PIN'
  }
  
  sensor_val = sensor_val/10.0;
  
  Serial.print("IR sensor is ");
  
  Serial.println(sensor_val);

  return sensor_val;                                //Return the value for this function
}
