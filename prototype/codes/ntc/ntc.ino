#include <Adafruit_MAX31865.h>
#include <SimpleKalmanFilter.h>

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(10, 11, 12, 13);

#define RREF      430.0
#define RNOMINAL  100.0

const int ntc_pin1 = A1;
const int ntc_pin2 = A0;
const int led1 = D2;
float ntc_vol1;
float ntc_vol2;
float rref2 = 4640;
float rref1 = 5580;

const int size = 5;
float temps[size] = {0};
float temps1[size] = {0};
float temps2[size] = {0};
float temp, kal_temp, kal_temp1, kal_temp2, med_temp, exp_temp;
double temp_ntc_c1, temp_ntc_c2;
float sort_temp[size] = {0};

unsigned long prev_rst = 0;
unsigned long rstTtime = 5000;

SimpleKalmanFilter kalman1(2, 2, 0.01);
SimpleKalmanFilter kalman2(2, 2, 0.01);
SimpleKalmanFilter kalman3(2, 2, 0.01);

float kalmanfilt1(float temp) {
  return kalman1.updateEstimate(temp);
}


float kalmanfilt2(float temp) {
  return kalman2.updateEstimate(temp);
}

float kalmanfilt3(float temp) {
  return kalman3.updateEstimate(temp);
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
    temps1[size] = {0};
    temps2[size] = {0};

    for (int j = 0 ; j < size ; j++) {
      temp = thermo.temperature(RNOMINAL, RREF);
      ntc();
      temps[j] = temp;
      temps1[j] = temp_ntc_c1;
      temps2[j] = temp_ntc_c2;

      kal_temp = kalmanfilt1(temp);
      kal_temp1 = kalmanfilt2(temp_ntc_c1);
      kal_temp2 = kalmanfilt3(temp_ntc_c2);
      delay(1000);
    }

    exp_temp = expAvg();
    med_temp = medianfilt(temp);

    if (kal_temp1 < 20.0) {
      digitalWrite(led1, HIGH);
    }
    else {
      digitalWrite(led1, LOW);
    }
  
    //Serial.print("Kalman PT100 = "); Serial.println(kal_temp);
    //Serial.print("EMA = "); Serial.println(exp_temp);
    //Serial.print("Median = "); Serial.println(med_temp);
    Serial.print("NTC1 = "); Serial.println(kal_temp1);
    Serial.print("NTC2 = "); Serial.println(kal_temp2);

    SimpleKalmanFilter kalman1(2, 2, 0.01);
    SimpleKalmanFilter kalman2(2, 2, 0.01);
    SimpleKalmanFilter kalman3(2, 2, 0.01);
  }
  Serial.println();

  delay(1000);
}

void ntc() {
  float ntc_vol_bits1 = analogRead(ntc_pin1);
  ntc_vol1 = (ntc_vol_bits1/1023)*3.3;
  float R1 = ((3.3/ntc_vol1) - 1)*(rref1);
  double temp_ntc1 = 1 / (0.0011291951569516656 + 0.00023411748218116476 * log(R1) + (8.769721422116814e-8 * log(R1) * log(R1) * log(R1)));
  temp_ntc_c1 = temp_ntc1 - 273.0;

  //delay(1000);
  
  float ntc_vol_bits2 = analogRead(ntc_pin2);
  ntc_vol2 = (ntc_vol_bits2/1023)*3.3;
  float R2 = ((3.3/ntc_vol2) - 1)*(rref2);
  double temp_ntc2 = 1 / (0.0011291951569516656 + 0.00023411748218116476 * log(R2) + (8.769721422116814e-8 * log(R2) * log(R2) * log(R2)));
  temp_ntc_c2 = temp_ntc2 - 273.0;
  
  //Serial.print("Resistance :"); Serial.print(R1);
  //Serial.println();

  //Serial.print("Temperature_ntc_1 :"); Serial.println(temp_ntc_c1);
  //Serial.println();
  //Serial.print("Temperature_ntc_2 :"); Serial.println(temp_ntc_c2);
  //Serial.println();
  //delay(1000);
}
