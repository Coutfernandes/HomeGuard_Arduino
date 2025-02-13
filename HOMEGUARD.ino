#include <Wire.h>  // Biblioteca para Comunicação I2C (não utilizada no código atual, mas pode ser necessária para comunicação com dispositivos I2C)

// Parâmetros do sensor de temperatura
#define Vin 5.0       // Tensão de alimentação do circuito
#define T0 298.15     // Temperatura de referência em Kelvin (25 °C)
#define Rt 10000      // Resistor fixo no divisor de tensão (em ohms)
#define R0 10000      // Resistência do termistor a 25 °C (em ohms)
#define T1 273.15     // Temperatura de referência inferior em Kelvin (0 °C)
#define T2 373.15     // Temperatura de referência superior em Kelvin (100 °C)
#define RT1 35563     // Resistência do termistor em T1 (em ohms)
#define RT2 549       // Resistência do termistor em T2 (em ohms)

float beta = 0.0;          // Coeficiente beta do termistor
float Rinf = 0.0;          // Constante calculada do termistor
float TempKelvin = 0.0;    // Temperatura medida em Kelvin
float TempCelsius = 0.0;   // Temperatura medida em Celsius
float Vout = 0.0;          // Tensão de saída do divisor de tensão
float Rout = 0.0;          // Resistência calculada do termistor

// Sensor de luz
const int ldrPin = A1;     // Pino analógico conectado ao LDR

// Parâmetros do sensor de corrente
float tensaoAC = 220.000;  // Tensão da rede elétrica (em volts)
float correnteRms;         // Corrente RMS calculada (em amperes)
float potencia;            // Potência calculada (em watts)
float energia = 0;         // Energia acumulada (em kilojoules)
float sensibilidade = 0.100; // Sensibilidade do sensor de corrente
 
void setup() {
  Serial.begin(9600);      // Inicializa a comunicação serial a 9600 bps
  setupTempSensor();       // Configura o sensor de temperatura
  setupLightSensor();      // Configura o sensor de luz
  setupCurrentSensor();    // Configura o sensor de corrente
}

void loop() {
  readTempSensor();        // Lê e exibe dados do sensor de temperatura
  readLightSensor();       // Lê e exibe dados do sensor de luz
  readCurrentSensor();     // Lê e exibe dados do sensor de corrente
  delay(500);              // Aguarda 500 ms antes de repetir o loop
}

// Funções do sensor de temperatura
void setupTempSensor() {
  beta = (log(RT1 / RT2)) / ((1 / T1) - (1 / T2)); // Calcula o coeficiente beta
  Rinf = R0 * exp(-beta / T0);                    // Calcula a constante Rinf
  pinMode(13, OUTPUT);                            // Configura o pino 13 como saída (buzzer)
  pinMode(12, OUTPUT);                            // Configura o pino 12 como saída (LED)
  delay(100);                                     // Aguarda 100 ms
}

void readTempSensor() {
  Vout = Vin * ((float)(analogRead(0)) / 1024.0); // Lê a tensão analógica e converte para voltagem
  Rout = (Rt * Vout / (Vin - Vout));              // Calcula a resistência do termistor
  TempKelvin = (beta / log(Rout / Rinf));         // Converte a resistência para temperatura em Kelvin
  TempCelsius = TempKelvin - 273.15;             // Converte Kelvin para Celsius

  // Exibe as temperaturas no Monitor Serial
  Serial.print("Temperatura em Celsius: ");
  Serial.print(TempCelsius);
  Serial.print("                                                  Temperatura em Kelvin: ");
  Serial.println(TempKelvin);
  
  // Ativa o alarme se a temperatura exceder 33 °C
  if (TempCelsius > 33.00) {
    tone(13, 450 * pow(5.0, (constrain(int(TempCelsius), 35, 127) - 57) / 12.0), 1000); // Emite som proporcional à temperatura
    digitalWrite(12, HIGH); // Liga o LED
    delay(100);
    digitalWrite(12, LOW);  // Desliga o LED
    delay(100);
  } else {
    noTone(13);             // Desativa o som do buzzer
  }
}

// Funções do sensor de luz
void setupLightSensor() {
  pinMode(ldrPin, INPUT);   // Configura o pino do LDR como entrada
  pinMode(2, OUTPUT);       // Configura o pino 2 como saída (indicador de problema)
}

void readLightSensor() {
  int luminosidade = analogRead(ldrPin); // Lê o valor analógico do LDR

  // Verifica se a luz está adequada
  Serial.print(luminosidade);
  if (luminosidade > 400) {
    Serial.println("                                                     Luz do quarto normal!"); // Exibe mensagem de status normal
  } else {
    Serial.println("                                                     Luz do quarto pode estar com problema!!"); // Indica possível problema na iluminação
    digitalWrite(2, HIGH); // Liga o indicador de problema
    delay(100);
    digitalWrite(2, LOW);  // Desliga o indicador
    delay(100);
  }
}

// Funções do sensor de corrente elétrica
void setupCurrentSensor() {
  pinMode(A4, INPUT); // Configura o pino A4 como entrada para o sensor de corrente
}

void readCurrentSensor() {
  correnteRms = calculaCorrente(filtroDaMedia()); // Calcula a corrente RMS

  // Ignora leituras próximas de zero (ruído)
  if (correnteRms > 0.05) { // Ajuste 0.05 para o limiar adequado
    potencia = abs(correnteRms * tensaoAC);         // Calcula a potência elétrica
    energia += (potencia * 1.2 / 1000);             // Calcula a energia acumulada em kJ

    // Exibe os valores de corrente, potência e energia no Monitor Serial
    Serial.println("---- Leitura do Sensor de Corrente ----");
    Serial.print("Tensao:    ");
    Serial.print(tensaoAC, 3);
    Serial.println(" V");
    
    Serial.print("Corrente:  ");
    Serial.print(correnteRms, 3);
    Serial.println(" A");

    Serial.print("Potencia:  ");
    Serial.print(potencia, 3);
    Serial.println(" W");

    Serial.print("Energia:   ");
    Serial.print(energia, 3);
    Serial.println(" kJ");
    Serial.println("----------------------------------------");
  } else {
    Serial.println("Nenhum consumo detectado!");
  }
}

float calculaCorrente(int sinalSensor) {
  float corrente = (float)(sinalSensor) * (5.000) / (1023.000 * sensibilidade);
  return abs(corrente); // Garante que o valor seja sempre positivo
}

int filtroDaMedia() {
  long somaDasCorrentes = 0, mediaDasCorrentes;
  for (int i = 0; i < 1000; i++) {
    somaDasCorrentes += pow((analogRead(A1) - 509), 2); // Soma os valores quadrados para cálculo RMS
    delay(1); // Pequeno atraso para leitura
  }
  mediaDasCorrentes = sqrt(somaDasCorrentes / 1000); // Calcula a média quadrática
  if (mediaDasCorrentes == 1) // Remove ruído na leitura
    return 0;
  return mediaDasCorrentes;
}
