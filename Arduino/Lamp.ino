/*
NOME DO PROJETO: NurseCallSystem - Lamp
AUTOR: Leonardo Severino - leoseverino0901@gmail.com   
DESCRIÇÃO:
  Programa em Arduino que recebe e processa as chamadas de pacientes, ativando os Leds e transmitindo as informações ao servidor que enviará as informações ao aplicativo integrado. Quando um paciente realiza um chamado, ele registra os dados recebidos de timestamp, 
endereço MAC do cliente, ID do paciente que realizou a chamada e prioridade do chamado. Então, ele envia os dados para a aplicação em React native e aguarda que o usuário responda através desta, desativando e/ou ativando os Leds de acordo.
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
int priorityA, priorityB; //Prioridade dos chamados dos pacientes A e B
bool pacient1Active, pacient2Active; //Estado dos chamados dos pacientes 1 e 2

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
  ssid  = ""; //Inicializa nome da rede
  password  = ""; //Inicializa senha da rede
  serverURL = ""; //Inicializa URL do servidor
  priorityA = priorityB = 0; //Inicializa prioridade dos pacientes
  pacient1Active = pacient2Active = false; //inicializa o estado dos chamados do pacientes
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


void HandleSalute(){ //Função 'HandleSalute' que responde à rota GET /Salute
  Serial.println("Returning: Hello!");
  server.send(200, "text/plain", "Hello!");  
}

void HandleCall(){ //Função 'HandleCall', que lida com a requisição POST
  String message, payload;
  StaticJsonDocument<200> doc; //Parse do JSON recebido
  DeserializationError error;
  int priority; //Prioridade do chamado
  String pacient; //ID do paciente
  String timestamp; //Timestamp do chamado
  String clientMAC; //Endereço MAC do cliente

  //Inicializa variavéis internas
  priority = 0;
  pacient = ""; //Quarto em que o paciente está localizado
  message = payload = timestamp = clientMAC = "";
  if(server.hasArg("plain")){ //Verifica se há dados na requisição
    message = server.arg("plain"); //Registra os dados da requisição
    Serial.println("Data received: " + message);
    error = deserializeJson(doc, message);
    if(!error){ //Se não houve erro
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
      timestamp = doc["timestamp"].as<String>(); //Registra a hora e data do chamado em 'timestamp'
      clientMAC = doc["clientMAC"].as<String>(); //Registra o endereço MAC do cliente em 'clientMAC'
      server.send(200, "application/json", "{\"Data received sucessfuly!\"}"); // Enviar resposta ao cliente     
      Serial.println("!NEW CALL!");
      updateLed(); //Atualiza os Leds
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

void ReceiveOff(){ //Função 'ReceiveOff', que recebe uma requisição para desativar um chamado
  String pacient; //Paciente cujo chamado será desativado 
  
  if(server.hasArg("pacient")){
    pacient = server.arg("pacient"); //Registra o paciente cujo chamado será desativado em 'pacient'
    if(pacient == "A"){ //Se for o paciente A
      pacient1Active = false; //Desativa o chamado do paciente A  
    }
    else if(pacient == "B"){ //Se for o paciente B
      pacient2Active = false; //Desativa o chamado do paciente B   
    }
    updateLED(); //Atualiza os Leds
    server.send(200, "text/plain", "Data received");
  }
  else{
    server.send(400, "text/plain", "No pacient provided");  
  }
}

void updateLED(){ //Função 'updateLED', que atualiza qual Led ficará ativo
  if(pacient1Active == false && pacient2Active == false){ //Se nenhum paciente tiver um chamado ativo
    TurnLedsOff(); //Desliga todos os Leds  
  }
  else{ //Senão
    if(pacient1Active == true && pacient2Active == true){ //Se ambos os pacientes tiverem um chamado ativo
      if(priorityA > priorityB){ //Se o chamado do paciente A tiver maior prioridade
        TurnLedOn(priorityA); //Ativa o Led correspondente a prioridade do chamado do paciente A      
      }
      else if(priorityB > priorityA){ //Se o chamado do paciente B tiver maior prioridade
        TurnLedOn(priorityB); //Ativa o Led correspondente a prioridade do chamado do paciente B  
      }
      else{ //Se ambos tiverem a mesma prioridade
        TurnLedOn(priorityA); //Ativa o Led correspondente a prioridade do chamado do paciente A
        TurnLedOn(priorityB); //Ativa o Led correspondente a prioridade do chamado do paciente B    
      }
    }
    else if(pacient1Active == true && pacient2Active == false){ //Se apenas o paciente 1 tiver um chamado ativo
      TurnLedOn(priorityA); //Ativa o Led correspondente a prioridade do chamado do paciente A  
    }
    else{ //Se apenas o paciente 2 tiver um chamado ativo
      TurnLedOn(priorityB); //Ativa o Led correspondente a prioridade do chamado do paciente B  
    }
  }
}

void TurnLedOn(int led){ //Função 'TurnLedOn', utilizada para ligar os LEDs
    TurnLedsOff(); //Desliga todos os Leds
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

void TurnLedsOff(){ //Função 'TurnLedsOff', utilizada para desligar os LEDs
  digitalWrite(LED1, LOW); //Apaga LED 1
  digitalWrite(LED2, LOW); //Apaga LED 2  
  digitalWrite(LED3, LOW); //Apaga LED 3     
}

void SaveData(String pacient, String timestamp, String clientMAC, String serverMAC, String path){ //Função 'SaveData', utilizada para registrar os chamados no arquivo
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

bool ReadFile(String path){ //Função 'ReadFile', utilizada para ler e exibir conteúdo do arquivo
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

bool WriteFile(String path, String message){ //Função 'WriteFile', utilizada para criar e escrever conteúdo no arquivo
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

bool AppendFile(String path, String message){ //Função 'AppendFile', utilizada para anexar conteúdo ao arquivo
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

void SendData(String pacient, int priority, String timestamp, String serverMAC, String clientMAC){ //Função 'SendData', utilizada para enviar os dados do chamado ao aplicativo
  HTTPClient http; //Cliente HTTP
  int httpResponseCode; //Código de resposta HTTP
  String message, response, route, localIP;
 
  route = "/sendCall"; //Rota para enviar chamado
  if(WiFi.status() == WL_CONNECTED){ //Se o WiFi estiver conectado
    http.begin(serverURL + route); //Inicia a conexão
    http.addHeader("Content-Type", "application/json"); //Adiciona header
    localIP = WiFi.localIP().toString(); //Registra o IP local em 'localIP'
    message = "{\"priority\":" + String(priority) + 
              ",\"timestamp\":\"" + timestamp + 
              "\",\"pacient\":\"" + pacient +
              "\",\"serverMAC\":\"" + serverMAC + 
              "\",\"clientMAC\":\"" + clientMAC + 
              "\",\"lampIP\":\"" + localIP + "\"}"; //Registra os dados do chamado em 'message'    
    httpResponseCode = http.POST(message); //Realiza o POST e registra o código de resposta em 'httpResponseCode'
    if(httpResponseCode > 0){ //Se o código de resposta for diferente de 0
      Serial.println("Data send sucessfuly!"); //Informa que os dados foram enviados com sucesso
    }
    else{ //Senão
      Serial.println("Error: " + httpResponseCode); //Informa que houve um erro
    }
    http.end();  // Fecha a conexão HTTP
  }
}
