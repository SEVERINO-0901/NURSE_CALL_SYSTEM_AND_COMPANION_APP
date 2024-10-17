/*
NOME DO PROJETO: NurseCallSystem - Server
AUTOR: Leonardo Severino - leoseverino0901@gmail.com  
DATA: 
DESCRIÇÃO:
  Programa em Arduino que recebe e processa as chamadas de pacientes, conectando-se a uma rede WiFi e iniciando um servidor na porta 80. Quando um cliente se conecta, ele registra os dados recebidos de timestamp, endereço MAC do cliente e ID do paciente que realizou a chamada, 
retornando uma resposta ao paciente. Então, ele envia os dados para a aplicação em React native e aguarda que o usuário responda através desta.
*/

//Inclusão das bibliotecas
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include <HTTPClient.h>

//Pinos da placa
#define LED 2 //Pino do Led da placa
#define LED1 18 //Pino do LED 1
#define LED2 19 //Pino do LED 2
#define LED3 21 //Pino do LED 3

//Protótipo das funções
void HandleSalute();
void HandleCall();
void ReceiveOff();
void updateLED();
void TurnLedOn(int led);
void TurnLedsOff();
void SendData(String pacient, int priority, String timestamp, String serverMAC, String clientMAC);
void SaveData(String pacient, String timestamp, String clientMAC, String serverMAC);
bool ReadFile(String path);
bool WriteFile(String path, String message);
bool AppendFile(String path, String message);

//Variavéis globais
String esp32MAC; //Endereço MAC do esp32
int priorityA, priorityB;
bool pacient1Active, pacient2Active; 

//Credenciais da rede
const char* ssid; //Nome da rede WiFi
const char* password; //Senha da rede WiFi
const char* serverURL; //URL do servidor
WebServer server(80);  // Servidor rodando na porta 80

void setup(){
  Serial.begin(115200); //Inicialização da serial
  //Incializa LEDs como saídas
  pinMode(LED, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  //Inicializa variavéis globais
  esp32MAC = "";
  ssid  = "SEVERINO_01"; //Inicializa nome da rede
  password  = "a67a70l00"; //Inicializa senha da rede
  serverURL = "http://192.168.0.225:3000"; //Inicializa URL do servidor
  priorityA = priorityB = 0;
  pacient1Active = pacient2Active = false;
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
  //Rota que responde uma saudacao com um "Oi"
  server.on("/Salute", HTTP_GET, HandleSalute);
  //Rota para realizar uma chamada
  server.on("/NewCall", HTTP_POST, HandleCall);
  //Rota para receber dados 
  server.on("/receiveOff", HTTP_POST, ReceiveOff);
  //Inicialização da partição SPIFFS sem formatar
  if(SPIFFS.begin(false)){ //Se a inicialização ocorreu com sucesso
    Serial.println("SPIFFS initialized!");
  }
  else{ //Senão
    Serial.println("Fail do initialize SPIFFS");
    while (1); //Loop infinito
  }
  //Inicia o servidor
  server.begin(); 
  Serial.println("Server Initiated!");
  //Se tudo tiver dado certo, liga o LED da placa
  digitalWrite(LED, HIGH);
}

void loop(){
  //Mantém o servidor rodando
  server.handleClient();
}


void HandleSalute(){ // Função que responde à rota GET /Salute
  Serial.println("Returning: Hello!");
  server.send(200, "text/plain", "Hello!");  
}

void HandleCall(){ // Função 'HandleCall', que lida com a requisição POST
  String message, payload;
  StaticJsonDocument<200> doc; //Parse do JSON recebido
  DeserializationError error;
  int priority;
  String pacient, timestamp, clientMAC;

  //Inicializa variavéis internas
  priority = 0;
  pacient = ""; //Quarto onde o paciente está localizado
  message = payload = timestamp = clientMAC = "";
  if(server.hasArg("plain")){ //Verifica se há dados na requisição
    message = server.arg("plain"); //Registra os dados da requisição
    Serial.println("Data received: " + message);
    error = deserializeJson(doc, message);
    if(!error){
      //Extrair e armazenar os dados nas variavéis
      priority = doc["priority"];
      if(doc["pacient"] == 1){ //Se o Paciente 1 realizou a requisição
        pacient += 'A';
        priorityA = priority;
        pacient1Active = true;     
      }
      else if(doc["pacient"] == 2){ //Se o Paciente 2 realizou a requisição
        pacient += 'B';
        priorityB = priority;
        pacient2Active = true;
      }
      timestamp = doc["timestamp"].as<String>();
      clientMAC = doc["clientMAC"].as<String>();
      server.send(200, "application/json", "{\"Data received sucessfuly!\"}"); // Enviar resposta ao cliente     
      Serial.println("!NEW CALL!");
      updateLed();
      Serial.println("Sending data to app");
      SendData(pacient, priority, timestamp, esp32MAC, clientMAC); //Envia conteúdo ao App
      Serial.println("Saving data");
      SaveData(pacient, timestamp, clientMAC, esp32MAC, "/log.txt"); //Salva conteúdo no arquivo
    }
    else{
      Serial.println("Error!");
      server.send(400, "application/json", "{\"status\":\"JSON error\"}");  
    }
  }
  else{
    server.send(400, "text/plain", "Error: No data!");  
  }
}

void ReceiveOff(){
  String pacient;
  
  if(server.hasArg("pacient")){
    pacient = server.arg("pacient");
    if(pacient == "A"){
      pacient1Active = false;  
    }
    else if(pacient == "B"){
      pacient2Active = false;  
    }
    updateLED();
    server.send(200, "text/plain", "Data received");
  }
  else{
    server.send(400, "text/plain", "No pacient provided");  
  }
}

void updateLED(){
  if(pacient1Active == false && pacient2Active == false){
    TurnLedsOff();  
  }
  else{
    if(pacient1Active == true && pacient2Active == true){
      if(priorityA > priorityB){
        TurnLedOn(priorityA);      
      }
      else if(priorityB > priorityA){
        TurnLedOn(priorityB);  
      }
      else{
        TurnLedOn(priorityA);
        TurnLedOn(priorityB);    
      }
    }
    else if(pacient1Active == true && pacient2Active == false){
      TurnLedOn(priorityA);  
    }
    else{
      TurnLedOn(priorityB);  
    }
  }
}

void TurnLedOn(int led){ //Função 'TurnLedOn', utilizada para ligar os LEDs
    TurnLedsOff();
    if(led == 1){
      Serial.println("Turning ON LED 1");
      digitalWrite(LED1, HIGH); //Acende LED 1
    }
    else if(led == 2){
      Serial.println("Turning ON LED 2");
      digitalWrite(LED2, HIGH); //Acende LED 2  
    }
    else if(led == 3){
      Serial.println("Turning ON LED 3");
      digitalWrite(LED3, HIGH); //Acende LED 3     
    }
}

void TurnLedsOff(){ //Função 'TurnLedOn', utilizada para desligar os LEDs
  digitalWrite(LED1, LOW); //Apaga LED 1
  digitalWrite(LED2, LOW); //Apaga LED 2  
  digitalWrite(LED3, LOW); //Apaga LED 3     
}

void SaveData(String pacient, String timestamp, String clientMAC, String serverMAC, String path){
  String message; //Texto a ser gravado no arquivo

  if(!SPIFFS.exists(path)){ //Se o arquivo ainda não existir
    message = "PACIENT: " + pacient + '\n' +
              "TIMESTAMP: " + timestamp + '\n' +
              "CLIENT MAC: " + clientMAC + '\n' +
              "SERVER MAC: " + serverMAC + '\n' +
              "\r---------------------------------------\n"; //Registra os dados do chamado em 'message'
    //Escreve o arquivo
    if(WriteFile(path, message)){ //Se a escrita ocorrer com sucesso
      Serial.println("------------------LOG------------------");
      if(ReadFile(path)){ //Mostra na Serial o conteúdo do arquivo
        Serial.println();
      }
    }
  }
  else{ //Se o arquivo já existir
    message = "PACIENT: " + pacient + '\n' +
              "TIMESTAMP: " + timestamp + '\n' +
              "CLIENT MAC: " + clientMAC + '\n' +
              "SERVER MAC: " + serverMAC + '\n' +
              "\r---------------------------------------\n"; //Registra os dados do chamado em 'message'
    //Anexa conteúdo ao arquivo
    if(AppendFile(path, message)){ //Se a anexação ocorreu com sucesso
      Serial.println("--------------------LOG-------------------");
        if(ReadFile(path)){ //Mostra na Serial o conteúdo do arquivo
          Serial.println();
        }
    }
  }
}

bool ReadFile(String path){ //Função para ler conteúdo de um arquivo
  File file; //Arquivo a ser lido

  file = SPIFFS.open(path); //Abre o caminho do arquivo da SPIFFS e passa o retorno para 'file'
  if(!file){ //Se houver falha ao abrir o caminho
    return false; //Retorna FALSE
  }
  else{ //Senão
    while(file.available()){ //Enquanto houver algum byte disponível para leitura de um arquivo
      Serial.write(file.read()); //Escreve o conteúdo do arquivo no monitor serial
    }
    file.close(); //Fecha o arquivo
  }

  return true; //Retorna TRUE
}

bool WriteFile(String path, String message){ //Função para criar e escrever conteúdo em um arquivo
  File file; //Arquivo a ser criado

  file = SPIFFS.open(path, FILE_WRITE); //Abre o arquivo, no modo escrita, onde será gravado o conteúdo, e passa o retorno para 'file'
  if(!file){ //Se houver falha ao abrir o caminho
    return false; //Retorna FALSE
  }
  else{ //Senão
    if(!file.print(message)){ //Se a criação e escrita do conteúdo no arquivo falhar
      return false; //Retorna FALSE
    }
    else{ //Senão
      Serial.println("Data written successfully");
    }
  }
  file.close(); //Fecha o arquivo

  return true; //Retorna TRUE
}

bool AppendFile(String path, String message){ //Função para anexar conteúdo a um arquivo
  File file; //Arquivo ao qual será anexado o conteúdo

  file = SPIFFS.open(path, FILE_APPEND); //Abre o arquivo, no modo anexar, onde será adicionado conteúdo, e passa o retorno para 'file'
  if(!file){ //Se houver falha ao abrir o caminho
    return false; //Retorna FALSE
  }
  else{ //Senão
    if(!file.print(message)){ //Se a anexação do conteúdo ao arquivo der errado
      return false; //Retorna FALSE
    }
    else{ //Senão
      Serial.println("Data written successfully");
    }
  }
  file.close(); //Fecha o arquivo

  return true; //Retorna TRUE
}

void SendData(String pacient, int priority, String timestamp, String serverMAC, String clientMAC){
  HTTPClient http;
  int httpResponseCode;
  String message, response, route, localIP;
 
  route = "/sendCall";
  if(WiFi.status() == WL_CONNECTED){
    http.begin(serverURL + route);
    http.addHeader("Content-Type", "application/json");
    localIP = WiFi.localIP().toString();
    message = "{\"priority\":" + String(priority) + 
              ",\"timestamp\":\"" + timestamp + 
              "\",\"pacient\":\"" + pacient +
              "\",\"serverMAC\":\"" + serverMAC + 
              "\",\"clientMAC\":\"" + clientMAC + 
              "\",\"lampIP\":\"" + localIP + "\"}";    
    httpResponseCode = http.POST(message);
    if(httpResponseCode > 0){
      Serial.println("Data send sucessfuly!");
    }
    else{
      Serial.print("Error: ");
      Serial.println(httpResponseCode);
    }
    http.end();  // Fecha a conexão HTTP
  }
}
