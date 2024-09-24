const express = require("express");
const app = express();
const port = 3000;

app.use(express.json()); // Para interpretar o JSON recebido

let calls = [];

// Endpoint para receber o chamado
app.post("/data", (req, res) => {
  const { pacient, timestamp, esp32MAC, clientMAC } = req.body;
  calls.push({pacient, timestamp, esp32MAC, clientMAC});
  // Responder para o ESP32 que os dados foram recebidos
  res.status(200).send("Data received sucessfuly!");
});

app.get("/data", (req, res) => {
  if(calls.length > 0){
    res.json(calls[calls.length - 1]);
  }
  else{
    res.status(404).send("N/A");
  }
});

app.listen(port, () => {
  console.log("Server Running");
});