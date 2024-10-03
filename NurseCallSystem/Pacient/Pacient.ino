/*
NOME DO PROJETO: NurseCallSystem - Pacient
AUTOR: Leonardo Severino - leoseverino0901@gmail.com
DATA: 
DESCRIÇÃO:
  Programa em Arduino que Realiza e registra chamadas de pacientes, Lendo o MAC address do ESP32, conectando-se a uma rede Wi-Fi e a um servidor para enviar os dados do paciente que acionou o chamado.As informações incluem o endereço MAC do ESP32, o número do paciente e o
timestamp do chamado.
*/

//Inclusão das bibliotecas
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <SPIFFS.h>
#include <FS.h>

//Pinos da placa
#define BUTTON1 18 //Pino do botão 1
#define BUTTON2 19 //Pino do botão 2
#define BUTTON3 21 //Pino do botão 3
#define LED 2 //Pino do Led

//Protótipo das funções
String HttpGet(String route);
void HttpPost(String route, String message);

//Variavéis globais
String esp32MAC; //Endereço MAC do esp32 
String lampMAC; //Endereço MAC do esp32 da lampada
int button1LastState, button2LastState, button3LastState; //Estado anterior dos botões
int button1State, button2State, button3State; //Estado atual do botões 

//Informacoes de DATA e HORA
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;

//Credenciais da rede
//const char* ssid  = ""; //Nome da rede WiFi
//const char* password  = ""; //Senha da rede WiFi
const char* ssid  = ""; //Nome da rede WiFi
const char* password  = ""; //Senha da rede WiFi
const char* lampIP  = ""; //Endereço IP da lampada

void setup(){
  Serial.begin(115200); //Inicialização da serial
  //Habilita o pull-up interno no pino dos botões
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  //Inicializa o LED como saída
  pinMode(LED, OUTPUT);
  //Inicializa variavéis globais
  esp32MAC = lampMAC = "";
  button1State = digitalRead(BUTTON1); //Inicializa estado do botão 1
  button2State = digitalRead(BUTTON2); //Inicializa estado do botão 2
  button3State = digitalRead(BUTTON3); //Inicializa estado do botão 3
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //Incializa DATA e HORA
  //Inicialização da partição SPIFFS
  if(SPIFFS.begin(false)){ //Se a inicialização ocorreu com sucesso
    Serial.println("SPIFFS initialized!");
  }
  else{ //Senão
    Serial.println("Fail do initialize SPIFFS");
    while (1); //Loop infinito
  }
  //Conexão na rede WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //Inicia a conexão WiFi na rede 'ssid' com a senha 'password'
  while(WiFi.status() != WL_CONNECTED){ //Enquanto não conecta na rede
    Serial.print(".");
    delay(1000); //Delay de 1s
  }
  Serial.println();
  Serial.println("WiFi connected.");
  //Imprime informacoes da rede
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //Leitura do endereço MAC do esp32
  esp32MAC = WiFi.macAddress();
  Serial.println("MAC Address: " + esp32MAC);
  // Realizar a requisição HTTP para o servidor
  Serial.println("Sending: Hello!");
  Serial.println("Server response: " + HttpGet("/Salute"));
  //Solicitar o endereço MAC do servidor
  Serial.println("Requesting MAC address");
  lampMAC = HttpGet("/MacAddress");
  Serial.println("Server response: " + lampMAC);
  //Se tudo tiver dado certo, liga o LED da placa
  digitalWrite(LED, HIGH);
}

void loop(){
  int button; //Botão pressionado pelo usuário
  int pacient; //Paciente que realizou a chamada
  int priority; //Prioridade do chamado
  String timestamp; //Data e horário do chamado
  String message; //Dados a serem enviados 

  //Inicializa variavéis internas
  button = priority = 0;
  timestamp = message = "";
  button = ButtonPressed(); //Verifica se houve um chamado
  if(button == 1 || button == 2 || button == 3){ //Se houve um chamado
    pacient = 0; //Registra em pacient o paciente que realizou o chamado
    timestamp = GetTime(); //Registra em 'timestamp' a data e horário do chamado
    priority = button; //Registra a prioridade do chamado
    Serial.println("PACIENT: 215A");
    Serial.println("PRIORITY: " + String(priority));
    Serial.println("TIMESTAMP: " + timestamp);
    Serial.println("CLIENT MAC: " + esp32MAC);
    Serial.println("LAMP MAC: " + lampMAC);
    Serial.println("Sending data to Server");
    //Concatena os dados no formato JSON
    message = "{\"pacient\":" + String(pacient) + //Paciente que realizou a chamada
              ",\"priority\":\"" + String(priority) + //Prioridade do chamado
              "\",\"timestamp\":\"" + timestamp + //timestamp do chamado
              "\",\"clientMAC\":\"" + esp32MAC + "\"}"; //endereço MAC do esp32
    HttpPost("/NewCall", message); //Envia dados ao servidor
  }
}

String HttpGet(String route){
  int httpResponseCode;
  String lampURL, response;
  HTTPClient http;
  
  if(WiFi.status() == WL_CONNECTED){ //Se o WiFi estiver conectado
    lampURL = "http://" + String(lampIP) + route;
    http.begin(lampURL);
    httpResponseCode = http.GET();
    if(httpResponseCode > 0){
      response = http.getString();
    }
    else{
      Serial.println("Error: " + String(httpResponseCode));
    }
    http.end();
  }
  else{
    Serial.println("WiFi not connected");
  }

  return response;
}
void HttpPost(String route, String message){
  int httpResponseCode;
  String lampURL, response;
  HTTPClient http;
  
  if(WiFi.status() == WL_CONNECTED){
    lampURL = "http://" + String(lampIP) + route;
    http.begin(lampURL);
    http.addHeader("Content-Type", "application/json"); // Definir o tipo de dado
    httpResponseCode = http.POST(message); // Enviar a requisição POST
    if(httpResponseCode > 0){
      response = http.getString();
      Serial.println("Server response: " + response);
    }
    else{
      Serial.println("Error: " + String(httpResponseCode));
    }
    http.end();
  }
  else{
    Serial.println("WiFi not connected");
  }
}

int ButtonPressed(){ //Função 'ButtonPressed', utilizada para verificar se um botão foi pressionado

  button1LastState = button1State; //Define o estado inicial do botão 1
  button2LastState = button2State; //Define o estado inicial do botão 2
  button3LastState = button3State; //Define o estado inicial do botão 3
  button1State = digitalRead(BUTTON1); //Atualiza o estado atual do botão 1
  button2State = digitalRead(BUTTON2); //Atualiza o estado atual do botão 2
  button3State = digitalRead(BUTTON3); //Atualiza o estado atual do botão 3
  if((button1LastState == HIGH and button1State == LOW) xor (button2LastState == HIGH and button2State == LOW) xor (button3LastState == HIGH and button3State == LOW)){ //Verifica se um botão foi pressionado
    if(button1State == LOW){ //Se o botão 1 foi pressionado
      return 1; //Retorna 1
    }
    else if(button2State == LOW){ //Se o botão 2 foi pressionado
      return 2; //Retorna Prioridade 2
    }
    else{ //Se o botão 3 foi pressionado
      return 3; //Retorna 3
    }
  }
  else { //Se nenhum botão foi pressionado
    return 0; //Retorna 0
  }
}

String GetTime(){ //Função 'GetTime', utilizada para ler data e hora atual
  struct tm timeinfo; //Informaçãoes de data e hora
  char local_time[20]; //buffer de dados

  //Registra DATA e HORA do chamado
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return String("00-00-00 00:00:00");
  }
  sprintf(local_time, "%04d-%02d-%02d %02d:%02d:%02d", //Registra DATA e HORA no formato YYYY-MM-DD HH:MM:SS na variavél 'local_time' 
    timeinfo.tm_year + 1900, //Ano(YYYY)
    timeinfo.tm_mon + 1, //Mês(MM)
    timeinfo.tm_mday, //Dia(DD) 
    timeinfo.tm_hour, //Hora(HH)
    timeinfo.tm_min, //Minutos(MM)
    timeinfo.tm_sec); //Segundos(SS)

  return String(local_time); //Retorna data e hora do chamado no formato String
}
