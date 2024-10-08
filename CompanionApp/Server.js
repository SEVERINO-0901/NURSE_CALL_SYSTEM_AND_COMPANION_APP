const express = require("express");
const app = express();
const port = 3000;

app.use(express.json());

let calls = []; //Lista de chamadas recebidas

//rota para receber dados do esp32
app.post("/api/data", (req, res) => {
  const { priority,  timestamp, pacient, serverMAC, clientMAC } = req.body; //Registra os dados
  if(!pacient || !priority || !timestamp || !serverMAC || !clientMAC){
    return res.status(400).send("Missing data");
  }
  else{
    console.log("Data Received: ");
    console.log(pacient, priority, timestamp, serverMAC, clientMAC);
    
    calls.push({priority, timestamp, pacient, serverMAC, clientMAC}); //Adiciona a chamada ao final da fila
    res.json({message: "Data received sucessfuly!"}); // Responder para o ESP32 que os dados foram recebidos
  }
});

//Rota para enviar dados para o app
app.get("/api/data", (req, res) => {
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