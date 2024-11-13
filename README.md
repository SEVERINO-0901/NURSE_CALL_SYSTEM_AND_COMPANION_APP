## DADOS DO PROJETO
- **Nome:** NurseCallSystem+CompanionApp; 
- **Autor:** Leonardo Severino - leoseverino0901@gmail.com;
- **Data de Criação:** DD/MM/AAAA - HH:MM:SS;
## SOBRE
- **Descrição:**
  - Implementação em Arduino de um sistema de chamada de enfermagem utilizando dois dispositivos ESP32, no qual um ESP32(Paciente) se comunica com outro ESP32(Servidor) para acionar um alerta quando um paciente solicita atendimento, além de armazenar as informações de endereço MAC dos dispositivos, ID do paciente que realizou a chamada e o timestamp da desta;
  - Implementação em React Native de uma aplicação para interagir com o sistema de chamadas de enfermagem, permitindo ao usuário visualizar os dados da chamada para que este possa responder ao chamado do paciente;
- **Requisitos:**
   - x3 dispositivos ESP32;
   - x6 push-buttons;
   - x3 Leds;
   - x3 Resistores de 220 ohms;
   - Possuir a biblioteca 'SPIFFS(https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/tag/1.1)' instalada na IDE do Arduino e inicializar uma imagem SPIFFS no ESP32 do Paciente;
- **Funcionalidades:**
  - ESP32-Pacient: Programa em Arduino que realiza chamadas de pacientes, utilizando os botões para enviar um chamado com uma determinada prioridade para o atendimento.Inicialmente, o ESP32 conecta-se a uma rede WiFi e, caso a conexão tenha sucesso, lê-se o endereço MAC do ESP32 e realiza-se o primeiro contato com a lampada correspondente ao quarto do paciente para o qual será enviado os dados do paciente que acionou o chamado, para que esta acenda o Led correspondente e envie ao servidor as informações do chamado. As informações incluem o endereço MAC do ESP32, o leito correspondente ao paciente, o timestamp e a prioridade do chamado;
  - ESP32-Lamp: Programa em Arduino que recebe e processa as chamadas de pacientes, ativando os Leds e transmitindo as informações ao servidor que enviará as informações ao aplicativo integrado. Quando um paciente realiza um chamado, ele registra em um arquivo os dados recebidos de timestamp, endereço MAC do cliente, ID do paciente que realizou a chamada e prioridade do chamado. Então, ele envia os dados para a aplicação em React native e aguarda que o usuário responda através desta, desativando e/ou ativando os Leds de acordo;
  - Companion App: Programa em React Native que implementa um aplicativo para que o usuário possa interagir com o sistema, iniciando o servidor para o qual serão enviados os dados dos chamados, exibindo os dados dos mesmos e permitindo respondê-los com o pressionar de um botão;

## DEMONSTRAÇÃO
![f2](https://github.com/user-attachments/assets/ede7f88a-6f5e-4850-a8bd-72c0960a18a9)
![f3](https://github.com/user-attachments/assets/24b5ba4d-62e5-49ac-87af-3e3871e4fc6e)
![f4](https://github.com/user-attachments/assets/be2df8e7-983b-4750-92e8-b00a948902d5)
![f5](https://github.com/user-attachments/assets/eada6232-692e-4ffe-a067-341db416500a)
![f6](https://github.com/user-attachments/assets/d1d79b5c-01d4-47b8-a5d4-38717f0daf4a)
![f7](https://github.com/user-attachments/assets/53719ca1-63bd-4bfa-8fae-d374a7476a21)
