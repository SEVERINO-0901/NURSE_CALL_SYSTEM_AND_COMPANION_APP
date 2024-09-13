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

//Protótipo das funções
void ReadMACaddress();
String MACtoString(byte ar[]);
bool ReadWifiData();
void GetData(String data);
bool WriteFile(String path, String message);
bool ReadFile(String path);
bool AppendFile(String path, String message);
bool FormatSPIFFS();

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
  //Inicialização da partição SPIFFS sem formatar
  if(SPIFFS.begin(false)){ //Se a inicialização ocorreu com sucesso
    Serial.println("SPIFFS initialized!");
  }
  else{ //Senão
    Serial.println("Fail do initialize SPIFFS");
    while (1); //Loop infinito
  }
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
  String path; //Caminho do arquivo a ser gravado
  String message; //Texto a ser gravado no arquivo

  path = "/log.txt"; //Define caminho do arquivo
  //Verifica se recebeu dados
  if(ReadWifiData()){ //Se recebeu dados do cliente 
    GetData(data); //Separa dados 
    //Salva o chamado no arquivo 'log.txt'
    if(!SPIFFS.exists(path)){ //Se o arquivo ainda não existir
      message = "PACIENT: " + String(pacient) + '\n' + 
                "TIMESTAMP: " + timestamp + '\n' +
                "SERVER MAC: " + serverMAC + '\n' +
                "CLIENT MAC: " + clientMAC + '\n' +
                  "\r---------------------------------------\n"; //Registra os dados do chamado em 'message'          
      if(WriteFile(path, message)){ //Se a escrita ocorrer com sucesso
        Serial.println("------------------LOG------------------");
        if(ReadFile(path)){ //Mostra na Serial o conteúdo do arquivo
          Serial.println();
        }
      }
    }
    else{ //
      message = "PACIENT: " + String(pacient) + '\n' + 
                "TIMESTAMP: " + timestamp + '\n' +
                "SERVER MAC: " + serverMAC + '\n' +
                "CLIENT MAC: " + clientMAC + '\n' +
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
        Serial.print("DATA RECEIVED: ");
        Serial.println(data); //Imprime os dados recebidos no monitor serial
        client.println("Data received successfully");
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
