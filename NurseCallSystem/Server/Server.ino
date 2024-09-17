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
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <esp_wifi.h>
#include <SPIFFS.h>
#include <FS.h>
#include <HTTPClient.h>

//Protótipo das funções
void ReadMACaddress();
String MACtoString(byte ar[]);
bool ReadWifiData();
void GetData(String data);

void SendToApp(String pacient, String timestamp, String clientMAC, String serverMAC);

String data; //Buffer de dados
//Dados que serão recebidos do cliente
int pacient; //Número correspondente ao paciente que realizou o chamado
String timestamp; //Data e Horário do chamado no formato DD-MM-YY HH:MM:SS
String clientMAC; //Endereço MAC do cliente
String serverMAC; //Endereço MAC do servidor  

//Credenciais da rede
const char* ssid  = "SEVERINO_01"; //Nome da rede WiFi
const char* password  = "a67a70l00"; //Senha da rede WiFi
WiFiServer server(80); //Servidor WiFi, inicializado na porta 80

void setup(){
  Serial.begin(115200); //Inicialização da serial
  //Inicializa variavéis
  pacient = 0;
  data = timestamp = clientMAC = "";
  //Leitura do MAC address do esp32
  WiFi.mode(WIFI_STA); //Coloca o WiFi no modo Station
  WiFi.STA.begin();
  Serial.print("[DEFAULT]ESP32 Board MAC Address: ");
  ReadMACaddress(); //Lê o endereço MAC
  Serial.println(clientMAC); //Exibe o endereço MAC no monitor serial
   //Conexão na rede WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //Inicia a conexão WiFi na rede 'ssid' com a senha 'password'
  while(WiFi.status() != WL_CONNECTED){ //Enquanto não conecta na rede
    Serial.print(".");
    delay(500); //Delay de 0.5s
  }
  Serial.println();
  Serial.println("WiFi connected.");
  //Imprime informacoes da rede
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin(); //Inicia o servidor
  Serial.println("Server Initiated!");
}

void loop(){
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
      GetData(data);
      Serial.println(pacient);
      Serial.println(timestamp);
      Serial.println(serverMAC);
      Serial.println(clientMAC);     
    }
    //Fecha a conexão com o cliente
    client.stop(); 
    Serial.println("Client disconnected");
  }
}

void ReadMACaddress(){ //Função 'ReadMACaddress', utilizada para ler o endereço MAC do esp32
  uint8_t mac[6];
  esp_err_t ret; //Indicador da leitura do endereço MAC

  ret = esp_wifi_get_mac(WIFI_IF_STA, mac); //Realiza a leitura do endereço MAC
  if(ret == ESP_OK){ //Se a leitura ocorreu com sucesso
    serverMAC = MACtoString((byte*) &mac); //Converte o endereço MAC para o formato String e registra em 'clientMAC'
  }
  else{ //Senão
    Serial.println("Failed to read MAC address");
  }
}

String MACtoString(byte ar[]){ //Função 'MACtoString', utilizada para converter o endereço MAC para o formato de String, para ser enviado ao cliente
  String s; //Endereço MAC em formato de String
  char buf[3]; //buffer de dados

  for(byte i = 0; i < 6; ++i){ //Laço de repetição
    sprintf(buf, "%02X", ar[i]); //Converte byte para char
    s += buf; //Acrescenta o char à String
    if(i < 5){ //Se Terminou de receber o byte
      s += ':'; //Acrescenta ':'
    }
  }

  return s; //Retorna endereço MAC do esp32 em formato de String
}

bool ReadWifiData(){ //Função 'ReadWifiData', utilizada para receber dados via WiFi
  WiFiClient client; //Cliente WiFi
  
  client = server.available(); //Verifica se há clientes
  if(client){ //Se um cliente se conectou
    Serial.println("New client connected");
    //Lê dados do cliente
    while(client.connected()){ //Enquanto o cliente está conectado
      if(client.connected()){ //Se o cliente ainda estiver conectado
        data = client.readStringUntil('\r'); //Realiza a leitura dos dados recebidos do cliente através de 'data'  
        break; //Saí do loop      
      }
      else{ //Senão
        Serial.println("Lost Connection to Client");
        break; //Saí do loop de repetição    
      }
    }
    client.stop(); //Fecha a conexão com o cliente
    Serial.println("Client disconnected");
  }
  else{ //Senão
    return false; //Retorna FALSE  
  }

  return true; //Retorna TRUE
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
