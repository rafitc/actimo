#include <Wire.h>
#include "Firebase_Arduino_WiFi101.h"

#define FIREBASE_HOST "xxxxxxxxxxxxxxxxxx.firebaseio.com" //Firebase Host URL
#define FIREBASE_AUTH "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" //Firebase Auth
#define WIFI_SSID "SSID" //SSID
#define WIFI_PASSWORD "PASSWORD" //PASSWORD

#define QUEUE_EMPTY -1
#define COUNT_MAX 900

#define BLUE_LED PIN_PD3
#define RED_LED PIN_PD0
#define GREEN_LED PIN_PD2


const int MPU6050_addr=0x68;
int16_t AccX,AccY,AccZ;

FirebaseData firebaseData;
String path = "/AccelermeterData";

int count ;
bool stopFlag = false;

typedef struct node {
  int value;
  struct node *next;
} node;

typedef struct {
  node *head;
  node *tail;
} queue;

void init_queue(queue *q) {
  q->head = NULL;
  q->tail = NULL;
}

bool enqueue(queue *q, int value) {
  node * newnode = malloc(sizeof(node));
  if (newnode == NULL) return false;
  newnode->value = value;
  newnode->next = NULL;

  if (q->tail != NULL) {
    q->tail->next = newnode;
  }
  q->tail = newnode;

  if (q->head == NULL) {
    q->head = newnode;
  }
  count++;
  return true;
}

int dequeue(queue *q) {
  if (q->head == NULL) return QUEUE_EMPTY;
  node *tmp = q->head;
  int result = tmp->value;
  q->head = q->head->next;
  if (q->head == NULL) {
    q->tail = NULL;
  }
  free(tmp);
  count--;
  return result;
}
queue q1, q2, q3;

void setup() {
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, HIGH);
  
  WiFi.setPins(
    PIN_PA7, // CS
    PIN_PF2, // IRQ
    PIN_PA1, // RST
    PIN_PF3 // EN
  );
  
    Serial2.begin(9600);
    int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED)
  {
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial2.print(".");
    digitalWrite(RED_LED, LOW);
    delay(300);
    digitalWrite(RED_LED, HIGH);
  }
  Serial2.println();
  Serial2.print("Connected with IP: ");
  IPAddress ip = WiFi.localIP();
  if(ip){
    digitalWrite(BLUE_LED, LOW);
  }
  Serial2.println(ip);
  Serial2.println();
  Serial2.println("Initialize MPU6050");
  
  Wire.begin();
  Wire.beginTransmission(MPU6050_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial2.println("Initialized !!! ");

  init_queue(&q1);
  init_queue(&q2);
  init_queue(&q3);

  Serial2.print("Connecting to Wi-Fi");

  //Provide the autntication data
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);
  
  delay(500);
}

void loop() {
  int t1, t2, t3;
  String jsonStr;
  Wire.beginTransmission(MPU6050_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_addr,14,true);
  AccX=Wire.read()<<8|Wire.read();
  AccY=Wire.read()<<8|Wire.read();
  AccZ=Wire.read()<<8|Wire.read();

  
  if (count <= 200 && !stopFlag) {
  Serial2.print(" Xraw = "); Serial2.print(AccX);
  Serial2.print(" Yraw = "); Serial2.print(AccY);
  Serial2.print(" Zraw = "); Serial2.println(AccZ);

    Serial2.println("Writing Data");
    enqueue(&q1, AccX);
    enqueue(&q2, AccY);
    enqueue(&q3, AccZ);
    delay(100);
  }
  else {
    digitalWrite(GREEN_LED, LOW);
    stopFlag = true;
    t1 = dequeue(&q1);
    t2 = dequeue(&q2);
    t3 = dequeue(&q3);
    if (t1 != QUEUE_EMPTY && t2 != QUEUE_EMPTY && t3 != QUEUE_EMPTY) {
      Serial2.print(count);
      Serial2.println(" th Value Pushed to firebase ");
      /*Serial2.print(t1);
      Serial2.print("\t");
      Serial2.print(t2);
      Serial2.print("\t");
      Serial2.println(t3);*/
      
      jsonStr = "{\"Count\":\""+String(count)+"\", \"Ax\":\""+String(t1)+"\", \"Ay\":\""+String(t2)+"\", \"Az\":\""+String(t3)+"\"}";
      Firebase.pushJSON(firebaseData, path + "/Accelerometer", jsonStr);
      
      digitalWrite(GREEN_LED, HIGH);
    }
  }
  if(count == 0 && stopFlag){
    stopFlag = false;
    Serial2.println("New session started !");
  }
}
