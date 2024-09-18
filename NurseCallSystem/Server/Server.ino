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
#include <ESPAsyncWebServer.h> //
//#include <WiFiClient.h>
//#include <WiFiAP.h>
//#include <esp_wifi.h>

//Pinos da placa
#define LED 2 //Pino do Led da placa
#define LED1 18 //Pino do LED 1
#define LED2 19 //Pino do LED 2
#define LED3 21 //Pino do LED 3

//Protótipo das funções
bool ReadWifiData();
void GetData(String data);
void TurnOnLed(int led);
void SendDataToApp(int pacient, String timestamp, String clientMAC, String serverMAC);

String data; //Buffer de dados
//Dados que serão recebidos do cliente
int pacient; //Número correspondente ao paciente que realizou o chamado
String timestamp; //Data e Horário do chamado no formato DD-MM-YY HH:MM:SS
String esp32MAC; //Endereço MAC do servidor  

String clientMAC; //Endereço MAC do cliente

//Credenciais da rede
const char* ssid  = "SEVERINO_01"; //Nome da rede WiFi
const char* password  = "a67a70l00"; //Senha da rede WiFi
AsyncWebServer server(80);  // Servidor rodando na porta 80
//WiFiServer server(80); //Servidor WiFi, inicializado na porta 80

void setup(){
  Serial.begin(115200); //Inicialização da serial
  //Incializa LEDs como saídas
  pinMode(LED, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  //Inicializa variavéis
  pacient = 0;
  data = timestamp = clientMAC = "";
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
  server.on("/Salute", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Oi");
  });
  //Rota para obter o endereço MAC do servidor
  server.on("/MacAddress", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", esp32MAC);
  });
  //Rota para realizar uma chamada
  server.on("/NewCall", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println(request->value());
    request->send(200, "text/plain", "Lâmpada ligada com sucesso");  
  }
  //Inicia o servidor
  server.begin(); 
  Serial.println("Server Initiated!");
  //Se tudo tiver dado certo, liga o LED da placa
  digitalWrite(LED, HIGH);
}

void loop(){
  //Nada
}

/*void loop(){
  WiFiClient client; //Cliente WiFi
  
  client = server.available(); //Verifica se há clientes
  if(client){ //Se um cliente se conectou
    Serial.println("New client connected");
    //Lê dados do cliente
    while(client.connected()){ //Enquanto o cliente está conectado
      if(client.connected()){ //Se o cliente ainda estiver conectado
        data = client.readStringUntil('\r'); //Realiza a leitura dos dados recebidos do cliente através de 'data'  
        Serial.println(data);
        break; //Saí do loop      
      }
      else{ //Senão
        Serial.println("Lost Connection to Client");
        break; //Saí do loop de repetição    
      }
    }
    //Envia resposta ao cliente
    if(data == "Hello! Please inform MAC address"){ //Se recebeu uma solicitação de endereço MAC
      client.println(serverMAC); //Envia endereço MAC
      data = client.readStringUntil('\r'); //Recebe a resposta do cliente
      Serial.println(data); 
    }
    else{ //Senão, recebeu uma requisição
      client.println("Data received successfuly!"); //Responde ao cliente
      GetData(data); //Separa dados
      TurnOnLed(pacient); //Acende LED
      SendDataToApp(pacient, timestamp, clientMAC, serverMAC); //Envia dados para o App     
    }
    //Fecha a conexão com o cliente
    client.stop(); 
    Serial.println("Client disconnected");
  }
}

void GetData(String data){ //Função 'GetData', utilizada para separar a String recebida em dados individuais
  int commaIndex; //Índice da vírgula 

  commaIndex = data.indexOf(','); //Índice da primeira vírgula
  timestamp = data.substring(0, commaIndex); //Registra a data e horário do chamado em 'timestamp'
  data = data.substring(commaIndex + 1); //Índice da segunda vírgula
  clientMAC = data.substring(0, commaIndex); //Registra o endereço MAC do cliente em 'clientMAC'
  data = data.substring(commaIndex + 1); //Índice da terceira vírgula
  pacient = data.toInt(); //Registra o ID correspondente ao paciente que realizou o chamado em 'pacient'
}

void TurnOnLed(int led){ //Função 'TurnOnLed', utilizada para ligar os LEDs
  if(led == 1){ //Se foi o paciente 1
    digitalWrite(LED1, HIGH); //Acende LED 1    
  }
  else if(led == 2){ //Se foi o paciente 2
    digitalWrite(LED2, HIGH); //Acende LED 2    
  }
  else if(led == 3){ //Se foi o paciente 3
    digitalWrite(LED3, HIGH); //Acende LED 3   
  }
}

void SendDataToApp(int pacient, String timestamp, String clientMAC, String serverMAC){ //Função 'SendDataToApp', utilizada para enviar os dados para o App em React Native
  //Envia dados para a aplicação em React Native  
}*/
