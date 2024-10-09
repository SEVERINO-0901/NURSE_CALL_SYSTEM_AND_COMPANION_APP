// server.js
const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const { default: axios } = require('axios');

const app = express();
const port = 3000;

app.use(cors()); // Permitir CORS
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

let calls = []; //Lista de chamadas recebidas

//Rotas POST
//rota para receber dados do esp32
app.post("/sendCall", (req, res) => {
  const { priority,  timestamp, pacient, serverMAC, clientMAC, lampIP } = req.body; //Registra os dados
  if(!pacient || !priority || !timestamp || !serverMAC || !clientMAC || !lampIP){
    return res.status(400).send("Missing data");
  }
  else{
    console.log("Data Received: ");
    console.log(priority,  timestamp, pacient, serverMAC, clientMAC, lampIP);
    
    calls.push({priority, timestamp, pacient, serverMAC, clientMAC, lampIP}); //Adiciona a chamada ao final da fila
    res.json({message: "Data received sucessfuly!"}); // Responder para o ESP32 que os dados foram recebidos
  }
});
//rota para receber dados da aplicação
app.post("/sendOff", (req, res) => {
  if(req.body.data){
    console.log("received data from RN:", req.body.data);
    res.status(200).send("Data received");
    sendDataToESP32(req.body.data);
  }
  else{
    res.status(400).send("No data received");  
  }
});
async function sendDataToESP32(ip) {
  try{
    const response = await axios.post(`http://${ip}/receiveOff`)
    console.log("Sending OFF to esp32");
  }
  catch(error){
    console.log("Error: ", error);
  }  
}

//Rotas GET
//Rota para enviar dados para o app
app.get("/getCalls", (req, res) => {
  if(calls.length > 0){ //Se a lista Nao estiver vazia
    const call = calls.shift(); //Remove a chamada mais antiga
    res.json(call); //Retorna a chamada mais antiga
  }
  else{
    res.status(404).send("N/A");
  }
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});