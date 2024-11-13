//Servidor para receber os chamados
const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const { default: axios } = require('axios');

const app = express();
const port = 3000; //Porta do servidor

app.use(cors()); // Permitir CORS
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

let calls = []; //Lista de chamadas recebidas

//Rotas POST
//rota para receber dados do esp32
app.post("/sendCall", (req, res) => {
  const { priority,  timestamp, pacient, serverMAC, clientMAC, lampIP } = req.body; //Registra os dados
  if(!pacient || !priority || !timestamp || !serverMAC || !clientMAC || !lampIP){ //Se tiver erro
    return res.status(400).send("Missing data"); //Retorna erro e informa
  }
  else{ //Senão
    console.log("Data Received: ");
    console.log(priority,  timestamp, pacient, serverMAC, clientMAC, lampIP);
    calls.push({priority, timestamp, pacient, serverMAC, clientMAC, lampIP}); //Adiciona a chamada ao final da fila
    res.json({message: "Data received sucessfuly!"}); //Responder para o ESP32 que os dados foram recebidos
  }
});
//rota para receber OFF
app.post("/sendOff", (req, res) => {
  const pacient = req.body.pacient; //Paciente
  const lampIP = req.body.lampIP; //Ip do esp32 da lampada

  if(pacient && lampIP){ //Se os dados estiverem corretos
    console.log("Removing call for pacient:", pacient);
    // Envia uma requisição POST para o ESP32 com o parametro 'pacient'
    axios.post(`http://${lampIP}/receiveOff`, `pacient=${encodeURIComponent(pacient)}`, {
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    })
    .then(response => res.status(200).send("Call removed"))
    .catch(error => res.status(500).send("Error removing call"));
  }
  else{ //Senão
    res.status(400).send("No pacient or lampIP provided"); //Informa que houve um erro
  }
});

//Rotas GET
//Rota para enviar dados para o app
app.get("/getCalls", (req, res) => {
  if(calls.length > 0){ //Se a lista Nao estiver vazia
    const call = calls.shift(); //Remove a chamada mais antiga
    res.json(call); //Retorna a chamada mais antiga
  }
  else{ //Senão
    res.status(404).send("N/A"); //Informa que houve um erro
  }
});

//Mantém o servidor rodando
app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});