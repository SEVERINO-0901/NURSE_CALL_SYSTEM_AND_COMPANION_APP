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
#define BUTTON1 5 //Pino do botão 1
#define BUTTON2 18 //Pino do botão 2
#define BUTTON3 19 //Pino do botão 3
#define LED 2 //Pino do Led

//Protótipo das funções
String HttpGet(String route);
void HttpPost(String route, String message);
String GetTime();
int ButtonPressed();
void SaveData(String path);
bool WriteFile(String path, String message);
bool ReadFile(String path);
bool AppendFile(String path, String message);
bool FormatSPIFFS();

//Dados que serão enviados ao servidor
int pacient; //Número correspondente ao paciente que realizou o chamado
String timestamp; //Data e Horário do chamado
String esp32MAC; //Endereço MAC do esp32

String serverMAC; //Endereço MAC do servidor

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
const char* serverIP  = "192.168.0.207"; //Endereço IP do servidor

void setup(){
  Serial.begin(115200); //Inicialização da serial
  //Habilita o pull-up interno no pino dos botões
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  //Inicializa o LED como saída
  pinMode(LED, OUTPUT);
  //Inicializa estado atual dos botões
  button1State = digitalRead(BUTTON1);
  button2State = digitalRead(BUTTON2);
  button3State = digitalRead(BUTTON3);
  //Inicializa variavéis
  pacient = 0;
  timestamp = "";
  //Incializa DATA e HORA
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
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
  serverMAC = HttpGet("/MacAddress");
  Serial.println("Server response: " + serverMAC);
  //Se tudo tiver dado certo, liga o LED da placa
  digitalWrite(LED, HIGH);
}
void loop(){
  int button; //Botão pressionado pelo usuário
  String message, path;
  
  button = ButtonPressed(); //Verifica se houve um chamado
  if(button == 1 || button == 2 || button == 3){ //Se houve uma chamada
    timestamp = GetTime(); //Registra em 'timestamp' a data e horário do chamado
    pacient = button; //Registra o paciente que realizou a chamada
    message = timestamp + ',' + esp32MAC + ',' + String(pacient);
    HttpPost("/NewCall", message);
    path = "/log.txt"; //Define caminho do arquivo
    SaveData(path);  
  } 
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

String HttpGet(String route){
  int httpResponseCode;
  String serverURL, response;
  HTTPClient http;
  
  if(WiFi.status() == WL_CONNECTED){ //Se o WiFi estiver conectado
    serverURL = "http://" + String(serverIP) + route;
    http.begin(serverURL);
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
  String serverURL, response;
  HTTPClient http;
  
  if(WiFi.status() == WL_CONNECTED){
    serverURL = "http://" + String(serverIP) + route;
    http.begin(serverURL);
    http.addHeader("Content-Type", "text/plain");
    httpResponseCode = http.POST(message);
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
  
void SaveData(String path){
  String message;
  
  if(!SPIFFS.exists(path)){ //Se o arquivo ainda não existir
    message = "PACIENT: " + String(pacient) + '\n' + 
              "TIMESTAMP: " + timestamp + '\n' +
              "SERVER MAC: " + serverMAC + '\n' +
              "CLIENT MAC: " + esp32MAC + '\n' +
              "\r---------------------------------------\n"; //Registra os dados do chamado em 'message'          
    if(WriteFile(path, message)){ //Se a escrita ocorrer com sucesso
      Serial.println("------------------LOG------------------");
      if(ReadFile(path)){ //Mostra na Serial o conteúdo do arquivo
        Serial.println();
      }
    }
  }
  else{ //Senão
    message = "PACIENT: " + String(pacient) + '\n' + 
              "TIMESTAMP: " + timestamp + '\n' +
              "SERVER MAC: " + serverMAC + '\n' +
              "CLIENT MAC: " + esp32MAC + '\n' +
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

bool WriteFile(String path, String message){ //Função 'WriteFile', utilizada para criar e escrever conteúdo em um arquivo
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

bool ReadFile(String path){ //Função 'ReadFile', utilizada para ler conteúdo de um arquivo
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

bool AppendFile(String path, String message){ //Função 'AppendFile', utilizada para anexar conteúdo a um arquivo
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

bool FormatSPIFFS(){ //Função 'FormatSPIFFS', utilizada para formatar o sistema de arquivos SPIFFS
  if(!SPIFFS.format()){ //Se a formatação falhar
    return false; //Retorna FALSE
  }

  return true; //Retorna TRUE
}
