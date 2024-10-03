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
//#include <HTTPClient.h>

//Pinos da placa
#define LED 2 //Pino do Led da placa
#define LED1 18 //Pino do LED 1
#define LED2 19 //Pino do LED 2
#define LED3 21 //Pino do LED 3

//Protótipo das funções
void HandleSalute();
void HandleMacAddress();
void HandleCall();
void TurnLedOn(int priority);
//void SendDataToApp(String pacient, String priority, String timestamp, String serverMAC, String clientMAC);


//Variavéis globais
String esp32MAC; //Endereço MAC do esp32
int currentPriority; //Prioridade atual

//Credenciais da rede
const char* ssid  = ""; //Nome da rede WiFi
const char* password  = ""; //Senha da rede WiFi
const char* serverUrl = ""; //URL do servidor
//const char* serverUrl = "http://192.168.0.224:3000/data"; //URL do servidor
WebServer server(80);  // Servidor rodando na porta 80

void setup(){
  Serial.begin(115200); //Inicialização da serial
  //Incializa LEDs como saídas
  pinMode(LED, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  //Inicializa variavéis globais
  currentPriority = 0;
  esp32MAC = "";
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
  //Rota para obter o endereço MAC do servidor
  server.on("/MacAddress", HTTP_GET, HandleMacAddress);
  //Rota para realizar uma chamada
  server.on("/NewCall", HTTP_POST, HandleCall);
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
  server.send(200, "text/plain", "Hello!");  
}

void HandleMacAddress(){ // Função que responde à rota GET /MacAddress
  server.send(200, "text/plain", esp32MAC); 
}

void HandleCall(){ // Função 'HandleCall', que lida com a requisição POST
  String message, payload;
  StaticJsonDocument<200> doc; //Parse do JSON recebido
  DeserializationError error;
  int priority;
  String pacient, timestamp, clientMAC;

  //Inicializa variavéis internas
  priority = 0;
  message = payload = timestamp = clientMAC = "";
  pacient = ""; //Quarto onde o paciente está localizado
  //pacient = "215";
  if(server.hasArg("plain")){ //Verifica se há dados na requisição
    message = server.arg("plain"); //Registra os dados da requisição
    Serial.println("Data received: " + message);
    error = deserializeJson(doc, message);
    if(!error){
    //Extrair e armazenar os dados nas variavéis
    if(doc["pacient"] == 1){ //Se o Paciente 1 realizou a requisição
      pacient += 'A';    
    }
    else if(doc["pacient"] == 2){ //Se o Paciente 2 realizou a requisição
      pacient += 'B';
    }
    priority = doc["priority"];
    timestamp = doc["timestamp"].as<String>();
    clientMAC = doc["clientMAC"].as<String>();
    server.send(200, "application/json", "{\"status\":\"sucess!\"}"); // Enviar resposta ao cliente     
    Serial.println("!NEW CALL!");
    Serial.println("PACIENTE: " + pacient);
    Serial.println("PRIORIDADE: " + String(priority));
    Serial.println("TIMESTAMP: " + timestamp);
    Serial.println("CLIENT MAC: " + clientMAC);
    Serial.println("SERVER MAC: " + esp32MAC);
    Serial.print("Turning ON ");
    TurnLedOn(priority);
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

void TurnLedOn(int priority){ //Função 'TurnLedOn', utilizada para ligar os LEDs
  if(priority > currentPriority){
    if(priority == 1){
      Serial.println("LED 1");
      digitalWrite(LED1, HIGH); //Acende LED 1
      digitalWrite(LED2, LOW); //Apaga LED 2
      digitalWrite(LED3, LOW); //Apaga LED 3
    }
    else if(priority == 2){
      Serial.println("LED 2");
      digitalWrite(LED2, HIGH); //Acende LED 2
      digitalWrite(LED1, LOW); //Apaga LED 1
      digitalWrite(LED3, LOW); //Apaga LED 3  
    }
    else if(priority == 3){
      Serial.println("LED 3");
      digitalWrite(LED3, HIGH); //Acende LED 3
      digitalWrite(LED1, LOW); //Apaga LED 1
      digitalWrite(LED2, LOW); //Apaga LED 2     
    }
    currentPriority = priority; //Atualiza a prioridade
  }
}

/*

void SendDataToApp(String pacient, String priority, String timestamp, String serverMAC, String clientMAC){
  HTTPClient http;
  String payload;
  int httpResponseCode;
  String response;
  
  if(WiFi.status() == WL_CONNECTED){ //Se estiver conectado ao WiFi
    http.begin(serverUrl); //Conecta ao servidor
    http.addHeader("Content-Type", "application/json"); //Adiciona o cabeçalho
    // Criar o JSON a ser enviado
    payload = "{";
    payload += "\"pacient\":" + pacient + ",";
    payload += "\"priority\":" + priority + ",";
    payload += "\"timestamp\":\"" + timestamp + "\",";
    payload += "\"serverMAC\":\"" + serverMAC + "\",";
    payload += "\"clientMAC\":\"" + clientMAC + "\"";
    payload += "}";
    httpResponseCode = http.POST(payload); // Enviar a solicitação POST
    if(httpResponseCode > 0){
      response = http.getString();
      Serial.println("Server response: " + response);
    }
    else{
      Serial.println("Error: " + String(httpResponseCode));
    }
    http.end();
  }  
}*/
