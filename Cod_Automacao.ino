#include <WiFi.h> // Inclui a biblioteca para conexão Wi-Fi
#include <IOXhop_FirebaseESP32.h> // Inclui a biblioteca Firebase para ESP32
#include <ArduinoJson.h> // Inclui a biblioteca ArduinoJson para manipulação de JSON
#include <OneWire.h> // Inclui a biblioteca OneWire para uso com DS18B20
#include <DallasTemperature.h> // Inclui a biblioteca DallasTemperature para uso com sensores de temperatura DS18B20

#define WIFI_SSID "Bernardo_WiFi" // Define o nome da rede Wi-Fi
#define WIFI_PASSWORD "001234500" // Define a senha da rede Wi-Fi
#define FIREBASE_HOST "https://test-codmorango-default-rtdb.firebaseio.com/" // Define a senha da rede Wi-Fi
#define FIREBASE_AUTH "ug99JgW7nJXjDwoPTminNukogqaeGVAjMQz7MN70" // Define a chave de autenticação do Firebase

// Bibliotecas para sensores e controle de atuadores
OneWire oneWire(26); // Cria uma instância da biblioteca OneWire para o pino 26 (DS18B20)
DallasTemperature sensors(&oneWire); // Cria uma instância da biblioteca DallasTemperature para uso com sensores DS18B20

int motorAbre = 5; // Define o pino para o motor DC que abre a estufa
int motorFecha = 23; // Define o pino para o motor DC que fecha a estufa
int bombaqua = 14; // Define o pino para a bomba de água
int ldr = 34; // Define o pino para o sensor de luminosidade (LDR)
const int umid = 39; // Define o pino para o sensor de umidade do solo

void setup() {
  Serial.begin(115200); // Inicializa a comunicação serial a uma taxa de 115200 bps
  Serial.println();

  // Inicialize o sensor de temperatura
  sensors.begin();

  pinMode(motorAbre, OUTPUT); // Define o pino do motorAbre como saída
  pinMode(motorFecha, OUTPUT); // Define o pino do motorFecha como saída
  pinMode(bombaqua, OUTPUT); // Define o pino da bomba de água como saída
  pinMode(ldr, INPUT); // Define o pino do LDR como entrada
  pinMode(umid, INPUT); // Define o pino do sensor de umidade do solo como entrada

  // Inicia com os motores desligados
  digitalWrite(motorAbre, HIGH); // Define o pino do motorAbre como alto (desligado)
  digitalWrite(motorFecha, HIGH); // Define o pino do motorFecha como alto (desligado)
  digitalWrite(bombaqua, HIGH); // Define o pino da bomba de água como alto (desligado)

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Inicia a conexão Wi-Fi com as credenciais definidas
  while(WiFi.status() != WL_CONNECTED){
    Serial.println("Conectando ao WiFi..."); // Imprime mensagem durante a conexão
    delay(1000); // Espera 1 segundo antes de tentar novamente
  }

  Serial.println("WiFi conectado!"); // Imprime mensagem quando a conexão Wi-Fi é bem-sucedida

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // Inicia a conexão com o banco de dados Firebase
}

void loop() {
  // Leitura da temperatura do DS18B20
  sensors.requestTemperatures(); // Solicita a leitura da temperatura ao sensor DS18B20
  int temperatura = sensors.getTempCByIndex(0); // Lê a temperatura do sensor DS18B20
  Serial.print("Valor do sensor de temperatura = "); // Imprime uma mensagem informativa
  Serial.println(temperatura); // Imprime o valor da temperatura lida
  Firebase.setInt("/Temperatura", temperatura); // Envia a temperatura para o banco de dados Firebase

  // Leitura da luminosidade com o LDR
  int lumi = analogRead(ldr); // Lê o valor analógico do LDR
  int luminosidade = map(lumi, 4095, 0, 0, 100); // Mapeia o valor da luminosidade
  Serial.print("Valor do sensor de luminosidade = "); // Imprime uma mensagem informativa
  Serial.println(luminosidade);  // Imprime o valor da luminosidade lida
  Serial.println("%"); // Imprime o símbolo de porcentagem
  Firebase.setInt("/Luminosidade", luminosidade); // Envia a luminosidade para o banco de dados Firebase

  // Leitura da umidade do solo
  int umidadeSolo = analogRead(umid); // Lê o valor analógico do sensor de umidade do solo
  int umidade = map(umidadeSolo, 4095, 0, 0, 100); // Mapeia o valor da umidade
  Serial.print("Valor sensor de umidade do solo = "); // Imprime uma mensagem informativa
  Serial.print(umidade); // Imprime o valor da umidade lida
  Serial.println("%"); // Imprime o símbolo de porcentagem
  Firebase.setInt("/Umidade", umidade); // Envia a umidade para o banco de dados Firebase

  // Ligação manual dos motores
  bool motorA = Firebase.getBool("/MotorOpen"); // Lê o estado do motor de abertura manual
  bool motorF = Firebase.getBool("/MotorClose"); // Lê o estado do motor de fechamento manual
  bool bomb = Firebase.getBool("/Bombaqua"); // Lê o estado da bomba de água

  // Verifica o estado da estufa
  bool estufaAberta = Firebase.getBool("/EstadoEstufa"); // Lê o estado atual da estufa
  bool modoManual = Firebase.getBool("/ModoManual"); // Lê o modo de operação (manual ou automático)

  // Setagem dos parâmetros
  bool temperaturaIdeal = (temperatura >= 13 && temperatura <= 26); // Verifica se a temperatura está na faixa ideal
  bool valorL = (luminosidade >= 0); // Verifica se a luminosidade é maior ou igual a zero

  // Verificação dos parâmetros e controle dos atuadores
  if (modoManual) {
    if (motorA && !estufaAberta || bomb) {
      // Botão de abrir pressionado manualmente
      digitalWrite(motorAbre, LOW); // Liga o motor DC para abrir
      delay(5000); // Aguarda por 5 segundos
      digitalWrite(motorAbre, HIGH); // Desliga o motor DC para abrir
      Serial.println("Estufa aberta manualmente"); // Imprime uma mensagem informativa
      digitalWrite(bombaqua, LOW); // Liga a bomba de água
      delay(5000); // Aguarda por 5 segundos
      digitalWrite(bombaqua, HIGH); // Desliga a bomba de água
      Serial.println("Bomba da agua ligada manualmente"); // Imprime uma mensagem informativa
      Firebase.setBool("/EstadoEstufa", true); // Atualiza o estado da estufa no Firebase
      Firebase.setBool("/MotorOpen", false); // Atualiza o estado do motor de abertura no Firebase
      Firebase.setBool("/MotorClose", false); // Atualiza o estado do motor de fechamento no Firebase
      Firebase.setBool("/Bombaqua", false); // Atualiza o estado da bomba de água no Firebase
    }
    else if (motorF && estufaAberta || bomb) {
      // Botão de fechar pressionado manualmente
      digitalWrite(motorFecha, LOW); // Liga o motor DC para fechar
      delay(5000); // Aguarda por 5 segundos
      digitalWrite(motorFecha, HIGH); // Desliga o motor DC para fechar
      Serial.println("Estufa fechada manualmente"); // Imprime uma mensagem informativa
      digitalWrite(bombaqua, LOW); // Liga a bomba de água
      delay(5000); // Aguarda por 5 segundos
      digitalWrite(bombaqua, HIGH); // Desliga a bomba de água
      Serial.println("Bomba da agua ligada manualmente"); // Imprime uma mensagem informativa
      Firebase.setBool("/EstadoEstufa", false); // Atualiza o estado da estufa no Firebase
      Firebase.setBool("/MotorOpen", false); // Atualiza o estado do motor de abertura no Firebase
      Firebase.setBool("/MotorClose", false); // Atualiza o estado do motor de fechamento no Firebase
      Firebase.setBool("/Bombaqua", false); // Atualiza o estado da bomba de água no Firebase
    }
  }
  else {
    if (valorL && temperaturaIdeal && !estufaAberta) {
      // Botão de abrir pressionado manualmente ou condição de temperatura para abrir
      digitalWrite(motorAbre, LOW); // Liga o motor DC para abrir
      delay(5000); // Aguarda por 5 segundos
      digitalWrite(motorAbre, HIGH); // Desliga o motor DC para abrir
      Serial.println("Estufa aberta");  // Imprime uma mensagem informativa
      Firebase.setBool("/EstadoEstufa", true); // Atualiza o estado da estufa no Firebase
      Firebase.setBool("/MotorOpen", false); // Atualiza o estado do motor de abertura no Firebase
    }
    else if ((!temperaturaIdeal || !valorL) && estufaAberta) {
      // Botão de fechar pressionado manualmente ou condição de temperatura para fechar
      digitalWrite(motorFecha, LOW); // Liga o motor DC para fechar
      delay(5000); // Aguarda por 5 segundos
      digitalWrite(motorFecha, HIGH); // Desliga o motor DC para fechar
      Serial.println("Estufa fechada"); // Imprime uma mensagem informativa
      Firebase.setBool("/EstadoEstufa", false); // Atualiza o estado da estufa no Firebase
      Firebase.setBool("/MotorClose", false); // Atualiza o estado do motor de fechamento no Firebase
    }
    else {
      digitalWrite(motorAbre, HIGH); // Desliga o motor DC de abrir
      digitalWrite(motorFecha, HIGH); // Desliga o motor DC de fechar
      Serial.println("Motor da estufa desligado"); // Imprime uma mensagem informativa
      Firebase.setBool("/MotorOpen", false); // Atualiza o estado do motor de abertura no Firebase
      Firebase.setBool("/MotorClose", false); // Atualiza o estado do motor de fechamento no Firebase
    }
  }

  if (!modoManual){
    // Defina um valor de umidade ideal para irrigação
    int umidadeIdeal = 70;  // Define o valor ideal de umidade
    if (umidade < umidadeIdeal || bomb) {
      digitalWrite(bombaqua, LOW); // Liga a bomba de água
      delay(5000); // Aguarda por 5 segundos
      digitalWrite(bombaqua, HIGH); // Desliga a bomba de água
      Serial.println("Bomba da agua ligada"); // Imprime uma mensagem informativa
      Firebase.setBool("/Bombaqua", false); // Atualiza o estado da bomba de água no Firebase
  } 
    else {
      digitalWrite(bombaqua, HIGH); // Desliga a bombaqua
      Serial.println("Bomba da agua desligada"); // Imprime uma mensagem informativa
    }
  }
  delay(5000); // Intervalo de leitura dos sensores
}
