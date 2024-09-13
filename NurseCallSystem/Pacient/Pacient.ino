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
#include <WiFiClient.h>
#include <time.h>
#include <esp_wifi.h>
#include <SPIFFS.h>
#include <FS.h>

//Pinos dos botões
#define BUTTON1 5 //Pino do botão 1
#define BUTTON2 18 //Pino do botão 2
#define BUTTON3 19 //Pino do botão 3

//Protótipo das funções
void ReadMACaddress();
String MACtoString(byte ar[]);
String GetTime();
int ButtonPressed();
bool SendWifiData();


//Dados que serão enviados ao servidor
int pacient; //Número correspondente ao paciente que realizou o chamado
String timestamp; //Data e Horário do chamado no formato DD-MM-YY HH:MM:SS
String clientMAC; //Endereço MAC do esp32 no formato AA:BB:CC:DD:EE:FF

//Estados dos botões
int button1LastState, button1State; //Estado inicial e atual do botão 1
int button2LastState, button2State; //Estado inicial e atual do botão 2
int button3LastState, button3State; //Estado inicial e atual do botão 3

//Informacoes de DATA e HORA
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;

//Credenciais da rede
const char* ssid  = "SEVERINO_01"; //Nome da rede WiFi
const char* password  = "a67a70l00"; //Senha da rede WiFi
//Credenciais do servidor
const char* server_ip  = "192.168.0.207"; //Endereço IP do servidor
const int server_port = 80; //Porta do servidor

WiFiClient server; //Cliente WiFi

void setup(){
  Serial.begin(115200); //Inicialização da serial
  //Habilita o pull-up interno no pino dos botões
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  //Inicializa estado atual dos botões
  button1State = digitalRead(BUTTON1);
  button2State = digitalRead(BUTTON2);
  button3State = digitalRead(BUTTON3);
  //Inicializa variavéis
  pacient = 0;
  timestamp = clientMAC = "";
  //Incializa DATA e HORA
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
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
}

void loop(){
  int button; //Botão pressionado pelo usuário

  button = ButtonPressed(); //Verifica se houve um chamado, registrando em 'pacient' o número correspondente ao paciente que realizou o chamado
  if(button == 1 || button == 2 || button == 3){ //Se houve uma chamada
    timestamp = GetTime(); //Registra em 'timestamp' a data e horário do chamado
    pacient = button; //Registra o paciente que realizou a chamada  
    if(SendWifiData()){ //Envia os dados ao servidor
      Serial.println("Data Sucessfuly Send!");
    }
    else{ //Senão
      Serial.println("!Error Sending Data!");
    } 
  }
}

void ReadMACaddress(){ //Função 'ReadMACaddress', utilizada para ler o endereço MAC do esp32
  uint8_t mac[6];
  esp_err_t ret; //Indicador da leitura do endereço MAC

  ret = esp_wifi_get_mac(WIFI_IF_STA, mac); //Realiza a leitura do endereço MAC
  if(ret == ESP_OK){ //Se a leitura ocorreu com sucesso
    clientMAC = MACtoString((byte*) &mac); //Converte o endereço MAC para o formato String e registra em 'clientMAC'
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

String GetTime(){ //Função 'GetTime', utilizada para ler data e hora atual
  struct tm timeinfo; //Informaçãoes de data e hora
  char local_time[20]; //buffer de dados

  //Registra DATA e HORA do chamado
  getLocalTime(&timeinfo);
  sprintf(local_time, "%02d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year - 100, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec); //Registra DATA e HORA no formato DD-MM-YY HH:MM:SS na variavél 'local_time'

  return String(local_time); //Retorna data e hora do chamado no formato String
}

int ButtonPressed(){ //Função para verificar se um botão foi pressionado
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
      return 2; //Retorna 2
    }
    else{ //Se o botão 3 foi pressionado
      return 3; //Retorna 3
    }
  }
  else { //Se nenhum botão foi pressionado
    return 0; //Retorna 0
  }
}

bool SendWifiData(){ //Função 'SendWifiData', utilizada para enviar dados via WiFi
  String data; //Buffer de dados

  //Conexão ao servidor
  if(!server.connected()){ //Se não estava conectado ao servidor
    Serial.print("Connecting to ");
    Serial.println(server_ip);
    //Conecta-se ao servidor
    if(server.connect(server_ip, server_port)){ //Se conseguiu se conectar ao servidor
      Serial.print("Connected to ");
      Serial.println(server_ip);
      data = timestamp + ',' + clientMAC + ',' + String(pacient); //Dados que serão enviados ao servidor
      Serial.println("Sending Data");
      server.println(data); //Envia dados para o sevidor
      //Aguarda resposta do servidor
      while(server.connected()){ //Enquanto estiver conectado ao servidor
        if(server.available()){ //Se o servidor estiver disponível
          Serial.println(server.readStringUntil('\r')); //Exibe dados recebidos do servidor no monitor serial
          break; //Saí do loop de repetição
        }
      }
      //Fecha a conexão com o servidor
      server.stop();
      Serial.println("Connection to server closed");
    }
    else{ //Se a conexão falhou
      Serial.println("Failed to Connect to Server");
      return false; //Retorna FALSE
    }
  }
  delay(1000); //Delay de 1s

  return true; //Retorna TRUE
}
