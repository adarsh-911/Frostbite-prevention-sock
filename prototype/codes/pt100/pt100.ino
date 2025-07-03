#include <Adafruit_MAX31865.h>
#include <SimpleKalmanFilter.h>

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(10, 11, 12, 13);

#define RREF      430.0
#define RNOMINAL  100.0

const int size = 5;
const int ledr = D2;
const int ledy = D3;
const int ledg = D4;
float temps[size] = {0};
float temp, kal_temp, med_temp, exp_temp;
float sort_temp[size] = {0};

unsigned long prev_rst = 0;
unsigned long rstTtime = 5000;

SimpleKalmanFilter kalman(2, 2, 0.01);

float kalmanfilt(float temp) {
  return kalman.updateEstimate(temp);
}

float medianfilt(float temp) {
  //float sort_temp[size];
  /*
  for (int i = size - 1 ; i > 0 ; i--) {
    temps[i] = temps[i-1];
  }
  temps[0] = temp;
  */
  memcpy(sort_temp, temps, size);
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (sort_temp[j] > sort_temp[j+1]) {
        float buff = sort_temp[j];
        sort_temp[j] = sort_temp[j+1];
        sort_temp[j+1] = buff;
      }
    }
  }
  //bubbleSort(sort_temp, size);
  if (size % 2 == 0) {
    return (sort_temp[size/2 - 1] + sort_temp[size/2])/2;
  }
  else {
    return sort_temp[(size-1)/2];
  }
}

float expAvg() {
  float alpha = 0.3;
  float ema_temp;
  float sum;
  for (int j = 0 ; j < size - 1 ; j++) {
    sum += temps[j];
  }
  ema_temp = (alpha * sum/4) + ((1 - alpha) * temps[size - 1]);

  return ema_temp;
}

void bubbleSort(float arr[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j+1]) {
        float buff = arr[j];
        arr[j] = arr[j+1];
        arr[j+1] = buff;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  thermo.begin(MAX31865_2WIRE);
}

void loop() {
  uint16_t rtd = thermo.readRTD();
  float ratio = rtd;
  ratio /= 32768;
  //Serial.print("Resistance = "); Serial.println(RREF*ratio,8);

  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    thermo.clearFault();
  }

  else {
    temps[size] = {0};

    for (int j = 0 ; j < size ; j++) {
      temp = thermo.temperature(RNOMINAL, RREF);
      temps[j] = temp;
      kal_temp = kalmanfilt(temp);
      delay(1000);
    }

    exp_temp = expAvg();
    med_temp = medianfilt(temp);

    if (kal_temp < 20.0 && kal_temp > 10.0) 
    {
      digitalWrite(ledy, HIGH);
    }
    else {
      digitalWrite(ledy, LOW);
    }
    digitalWrite(ledg, kal_temp > 20 ? HIGH : LOW);
    //digitalWrite(ledy, kal_temp > 20 ? HIGH : LOW);
    digitalWrite(ledr, kal_temp < 10 ? HIGH : LOW);
    Serial.print("PT100 = "); Serial.println(kal_temp);
    //Serial.print("EMA = "); Serial.println(exp_temp);
    //Serial.print("Median = "); Serial.println(med_temp);

    SimpleKalmanFilter kalman(2, 2, 0.01);
  }
  Serial.println();

  delay(1000);
}