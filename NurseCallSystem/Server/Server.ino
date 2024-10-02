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
#include <HTTPClient.h>

//Pinos da placa
#define LED 2 //Pino do Led da placa
#define LED1 18 //Pino do LED 1
#define LED2 19 //Pino do LED 2
#define LED3 21 //Pino do LED 3

//Protótipo das funções
void HandleSalute();
void HandleMacAddress();
void HandleCall();
void GetData(String data);
void TurnOnLed(int priority);
void SendDataToApp(String pacient, String priority, String timestamp, String serverMAC, String clientMAC);


//Dados que serão recebidos do cliente
int pacient; //Número correspondente ao paciente que realizou o chamado
int callPriority, currentPriority; //Prioridade do chamado
String timestamp; //Data e Horário do chamado no formato DD-MM-YY HH:MM:SS
String esp32MAC; //Endereço MAC do servidor  
String clientMAC; //Endereço MAC do cliente

//Credenciais da rede
const char* ssid  = "SEVERINO_01"; //Nome da rede WiFi
const char* password  = "a67a70l00"; //Senha da rede WiFi
const char* serverUrl = "http://192.168.0.224:3000/data"; //URL do servidor
WebServer server(80);  // Servidor rodando na porta 80

void setup(){
  Serial.begin(115200); //Inicialização da serial
  //Incializa LEDs como saídas
  pinMode(LED, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  //Inicializa variavéis
  pacient = callPriority = currentPriority = 0;
  timestamp = clientMAC = "";
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

void HandleCall(){ // Função que lida com o POST na rota /PostData
  String message, payload;
  
  if(server.hasArg("plain")){ // Verifica se há dados no corpo da requisição
    message = server.arg("plain"); // Pega o conteúdo do corpo da requisição
    Serial.println("Data received: " + message);
    server.send(200, "text/plain", "Data received sucessfully!"); // Responde ao cliente
    GetData(message); //Separa os dados
    TurnOnLed(callPriority); //Acende o LED do paciente
    SendDataToApp(String(pacient), String(callPriority), timestamp, esp32MAC, clientMAC); //Envia dados para o App
    Serial.println("PACIENT: " + String(pacient));
    Serial.println("PRIORIDADE: " + String(callPriority));
    Serial.println("TIMESTAMP: " + timestamp);
    Serial.println("SERVER MAC: " + esp32MAC);
    Serial.println("CLIENT MAC: " + clientMAC);
  }
  else{ //Senão
    // Responde com um erro se não houver dados no corpo da requisição
    server.send(400, "text/plain", "Error");  
  }  
}

void GetData(String data){ //Função 'GetData', utilizada para separar a String recebida em dados individuais
  int commaIndex; //Índice da vírgula 

  commaIndex = data.indexOf(','); //Índice da primeira vírgula
  timestamp = data.substring(0, commaIndex); //Registra a data e horário do chamado em 'timestamp'
  data = data.substring(commaIndex + 1); //Atualiza 'data'
  
  commaIndex = data.indexOf(','); //Índice da segunda vírgula
  clientMAC = data.substring(0, commaIndex); //Registra o endereço MAC do cliente em 'clientMAC'
  data = data.substring(commaIndex + 1); //Atualiza 'data'

  commaIndex = data.indexOf(','); //Índice da terceira vírgula
  pacient = data.toInt(); //Registra o ID correspondente ao paciente que realizou o chamado em 'pacient'
  data = data.substring(commaIndex + 1); //Atualiza 'data'
  
  callPriority = data.toInt();
}


void TurnOnLed(int priority){ //Função 'TurnOnLed', utilizada para ligar os LEDs
  if(priority > currentPriority){
    if(priority == 1){
      Serial.println("TURNING ON LED 1");
      digitalWrite(LED1, HIGH); //Acende LED 1
      digitalWrite(LED2, LOW); //Apaga LED 2
      digitalWrite(LED3, LOW); //Apaga LED 3
    }
    else if(priority == 2){
      Serial.println("TURNING ON LED 2");
      digitalWrite(LED2, HIGH); //Acende LED 2
      digitalWrite(LED1, LOW); //Apaga LED 1
      digitalWrite(LED3, LOW); //Apaga LED 3  
    }
    else if(priority == 3){
      Serial.println("TURNING ON LED 3");
      digitalWrite(LED3, HIGH); //Acende LED 3
      digitalWrite(LED1, LOW); //Apaga LED 1
      digitalWrite(LED2, LOW); //Apaga LED 2     
    }
  }
  currentPriority = priority;
  /*if(led == 1){ //Se foi o paciente 1
    Serial.println("TURNING ON LED 1");
    digitalWrite(LED1, HIGH); //Acende LED 1    
  }
  else if(led == 2){ //Se foi o paciente 2
    Serial.println("TURNING ON LED 2");
    digitalWrite(LED2, HIGH); //Acende LED 2    
  }
  else if(led == 3){ //Se foi o paciente 3
    Serial.println("TURNING ON LED 3");
    digitalWrite(LED3, HIGH); //Acende LED 3   
  }*/
}

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
}
