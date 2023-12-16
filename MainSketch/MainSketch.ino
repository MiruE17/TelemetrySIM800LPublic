//861065064718803//
//^^IMEI^^//
#define SATU_MNT 60000UL
#define DUA_JAM 7200000UL
#define ENAM_JAM 21600000UL

#include <Sim800l.h>
#include <SoftwareSerial.h>
Sim800l Sim800l;
float mentah, v, arus, volarus, current, reading, processed, sigQuality;
char sendQ[160], bufferQ[128], bufferV[4], bufferI[4], vSimC[7], sigQualC[3];
int counter = 1;
char countStr[12];

char vBat[8], vMid[8], vTur[8], vLam[8];

bool sttsOprtr = false;
String noTelp, isiSms;
String query = "$P#";
char* noTelpC, queryC;
uint8_t index;
int i = 0;

void setup() {
  Serial.begin(9600);
  Sim800l.begin();
  pinMode(5, INPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);
  delay(7000);
  Serial.println("Initialization...");
  sttsOprtr = Sim800l.regOperator();
  while (sttsOprtr == false) {
    sttsOprtr = Sim800l.regOperator();
    if(sttsOprtr == false){
      Serial.println("Connecting...");
      delay(250);
    }
  }
  Serial.println("Registered to Operator!");
  initSMS();
  Serial.println("Initialization Complete.");
  digitalWrite(23, LOW);
}

void loop() {
  digitalWrite(6, HIGH);
  delay(250);
  digitalWrite(6, LOW);
  delay(250);
  digitalWrite(6, HIGH);
  delay(250);
  digitalWrite(6, LOW);
  delay(250);
  strcpy(sendQ, "");
  Serial.println("Entering Loop...");
  Serial.println("Reading Voltage...");
  for (int i = 0; i < 4; i++) {

    strcpy(bufferQ, "");

    // 1 = Battery(A5), 2 = Mid(A4), 3 = Turbin(A3), 4 = Lampu(A2)
    cekVolt(1);
    cekVolt(2);
    cekVolt(3);
    cekVolt(4);
    if (i == 3) {
      digitalWrite(23, HIGH);
      sigQuality = 0;
      while (sigQuality == 0) {
        sigQuality = cekSignal();
        Serial.print("Sinyal : ");
        Serial.println(sigQuality);
        delay(500);
        digitalWrite(7, HIGH);
        delay(100);
        digitalWrite(7, LOW);
        delay(100);
        digitalWrite(7, HIGH);
        delay(100);
        digitalWrite(7, LOW);
        delay(100);
      }
    } else {
      sigQuality = 0;
    }
    strcpy(sigQualC, "");
    dtostrf(sigQuality, 1, 0, sigQualC);
    countStr[0] = "\0";
    sprintf(countStr, "%d", counter);
    sprintf(bufferQ, "$%s*%s*%s*%s*%s*%s#", countStr, vBat, vMid, sigQualC, vTur, vLam);
    if (counter >= 2000000000) {
      counter = 1;
    } else {
      counter++;
    }
    strcat(sendQ, bufferQ);
    if (i != 3) {
      strcat(sendQ, "\n");
      //delay(1800000);
      delay(7500);
    }


    //delay(1800000);

  }

  Serial.println("SMS Content : ");
  Serial.println(sendQ);
  Serial.println("Sending SMS 1...");
  Sim800l.sendSms("+62878xxxxxxxx", sendQ);
  Serial.println("SMS 1 Sent.");
  delay(7000);
  Sim800l.sendSms("+62813xxxxxxxx", sendQ);
  //Sim800l.sendSms("+62852xxxxxxxx",sendQ);
  Serial.println("SMS 2 Sent.");
  delay(7000);
  digitalWrite(23, LOW);
  digitalWrite(7, HIGH);
  delay(250);
  digitalWrite(7, LOW);
  delay(250);
  digitalWrite(7, HIGH);
  delay(250);
  digitalWrite(7, LOW);
  delay(250);
  delay(1800000);
  //delay(DUA_JAM);


  //   delay(7000);
  //   Sim800l.sendSms("+62878xxxxxxxx","System Wakeup");
  //   delay(7000);
  //   Sim800l.sendSms("+62813xxxxxxxx","System Wakeup");
  //   delay(7000);
  //   digitalWrite(23,LOW);
  //   delay(ENAM_JAM);
  //   delay(8*60*60*1000);
}

void initSMS() {
  if (digitalRead(5) == HIGH) {
    digitalWrite(6, HIGH);
    delay(750);
    Sim800l.sendSms("+62878xxxxxxx", "Sistem Berhasil Dinyalakan");
    digitalWrite(6, LOW);
    delay(7000);
  } else if (digitalRead(5) == LOW) {
    digitalWrite(7, HIGH);
    delay(750);
    digitalWrite(7, LOW);
  }
}
void cekVolt(int cek) {
  if (cek == 1) {
    reading = 0;
    reading = analogRead(A5);
    processed = (reading * 10.0) / 1024.0;
    vBat[0] = "\0";
    dtostrf(processed, 4, 2, vBat);
  } else if (cek == 2) {
    reading = analogRead(A4);
    processed = (reading * 5.0) / 1024.0;
    vMid[0] = "\0";
    dtostrf(processed, 4, 2, vMid);
  } else if (cek == 3) {
    reading = analogRead(A3);
    processed = (reading * 20.0) / 1024.0;
    vTur[0] = "\0";
    dtostrf(processed, 4, 2, vTur);
  } else if (cek == 4) {
    reading = analogRead(A2);
    processed = (reading * 10.0) / 1024.0;
    vLam[0] = "\0";
    dtostrf(processed, 4, 2, vLam);
  }
}

float cekVoltSim() {
  String volt = Sim800l.voltRead();
  int i1 = volt.indexOf(',');
  int i2 = volt.indexOf(',', i1 + 1);

  String thirdValue = volt.substring(i2 + 1);
  Serial.println(thirdValue);
  String voltage = thirdValue.substring(0, 4);
  Serial.println(voltage);
  float voltFloat = voltage.toFloat();
  voltFloat = voltFloat / 1000;
  return voltFloat;
}

float cekSignal() {
  String sig = Sim800l.signalQuality();
  Serial.println(sig);
  String subSig = sig.substring(14, 17);
  float sigFloat = subSig.toFloat();
  float sigQual = 0;
  if ((sigFloat == 0) || (sigFloat == 99)) {
    sigQual = 0;
  } else if (sigFloat == 1) {
    sigQual = 1;
  } else if ((sigFloat >= 2) && (sigFloat <= 30)) {
    sigQual = 2;
  } else if (sigFloat == 31) {
    sigQual = 3;
  }
  return sigQual;
}
//String cekSignal(){
//  String sig = Sim800l.signalQuality();
//  return sig;
//}

//void operasi(){
//  arus = analogRead(A1);
//  volarus = arus*5/1023.0;
//  current = (volarus-2.5)/0.1;
//  dtostrf(current,3,2,bufferI);
//  mentah = analogRead(A0);
//  v = ((mentah * 5.0) / 1024.0) / (7500.0/(30000.0+7500.0));
//  dtostrf(v,3,2,bufferV);
//  sprintf(bufferQ, "Query Berhasil Diterima, Voltase = %s, Arus = %s", bufferV, bufferI);
//  sprintf(bufferQ, "Query Berhasil Diterima");
//  Sim800l.delAllSms();
//  index = 21;
//  isiSms = Sim800l.readSms(index);
//  Serial.println(isiSms);
//  Serial.print("Last Index Of : ");
//  Serial.println(isiSms.lastIndexOf(query));
//  if((isiSms.lastIndexOf(query)) > -1){
//    Serial.print("Tespoint");
//    noTelp = Sim800l.getNumberSms(index);
//    noTelpC = noTelp.c_str();
//    if(noTelpC != ""){
//    Sim800l.sendSms(noTelpC, bufferQ);
//    }
// }else{
//  isiSms = "";
//  noTelpC = "";
//  Sim800l.delAllSms();
// }
// Serial.println(Sim800l.readAllSms());
// Sim800l.delAllSms();isiSms="";
//}
