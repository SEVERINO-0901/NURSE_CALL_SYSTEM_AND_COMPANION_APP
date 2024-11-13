//Importação dos componentes
import React, { 
  useState, 
  useEffect 
} from "react";
import { 
    StyleSheet, 
    Text, 
    View,
    Button, 
    ScrollView,
    Alert
} from "react-native";
import axios from "axios";

export default function CallScreen(){ //Função 'CallScreen', que implementa a tela de recepção de chamados
  const [pacientsData, setPacientsData] = useState([null]); //Dados das chamadas(configurar para quantos pacientes forem necessários)
  const sortedPacients = sortPacients(pacientsData); //Lista de pacientes ordenada
  let serverIP = ""; //Ip do servidor
  let serverPort = 3000; //Porta do servidor


  const FetchDataFromEsp32 = async () => { //Função 'FetchDataFromEsp32', que recebe dados do esp32
    try{
      const response = await axios.get(`http://${serverIP}:${serverPort}/getCalls`); //Rota para receber dados
  
      console.log(response.data);
      if(response.status === 200){ //Se recebeu dados
        const espData = response.data; //Registra os dados da chamada em 'espData'
        
        //Atualiza o estado garantindo uma chamada por paciente(clientMAC como chave)
        setPacientsData(prevData => {
          // Encontra se o paciente já está presente na lista
          const updatedData = [...prevData];
          const pacientIndex = updatedData.findIndex(pacient => pacient && pacient.clientMAC === espData.clientMAC);
          
          if(pacientIndex !== -1){ //Se o paciente já está na lista de chamados
            updatedData[pacientIndex] = espData; //Atualiza a chamada existente
          }
          else{//Se não houver chamada para o paciente 
            const firstNullIndex = updatedData.findIndex(pacient => pacient === null); //encontra o primeiro índice null para adicionar a nova chamada

            if(firstNullIndex !== -1){ //Se o índice for válido
              updatedData[firstNullIndex] = espData; //Atualiza a lista de chamados
            }
            else{ //Se não houver espaço
              updatedData.push(espData); //Adiciona ao final da lista
            }
          }
          Alert.alert("!!!NEW CALL!!!", `Pacient: ${espData.pacient}\nPriority: ${espData.priority}\n`); //Exibe o alerta de nova chamada

          return updatedData; //Retorna a lista de chamadas ativas
        });
      }
    }
    catch(error){
      console.log("Error: ", error); //Exibe erros(se houver)
    }
  };

  const ClearPacient = (index, pacient, ip) => { //Função 'ClearPacient', utilizada para remover um chamado da lista de chamadas ativas
    sendDataToEsp32(pacient, ip); //Envia os dados para o esp32
    setPacientsData(prevData => { //Atualiza a lista de chamados ativos
      const updatedData = [...prevData]; //Dados atualizados
  
      if(updatedData[index] !== null){ //Se o indíce não for nulo
        updatedData[index] = null; //Limpa os dados do paciente
      }
  
      // Filtra quaisquer valores 'null' ou inválidos que possam ter sobrado
      const cleanedData = updatedData.filter(pacient => pacient !== null);
  
      return cleanedData;
    });
  };

  const sortPacients = (data) => { //Função 'sortPacients', que ordena os chamados em ordem de prioridade e de timestamp
    return data //Retorna dados ordenados
      .filter(item => item !== null) //Remove dados nulos
      .sort((a, b) => {
        if(a.priority !== b.priority){
          return (b.priority - a.priority); //Ordem de prioridade
        }
        else{
          return (new Date(a.timestamp) - new Date(b.timestamp)); //Ordem de timestamp
        }  
      });
  };

  const sendDataToEsp32 = async(pacient, lampIP) => { //Função 'sendDataToEsp32', que envia dados do aplicativo para o esp32
    try {
      await axios.post(`http://${serverIP}:${serverPort}/sendOff`, { pacient, lampIP }, { //Envia requisição para desativar o chamado de 'pacient' no esp32 de 'lampIP' ao servidor
        headers: { 'Content-Type': 'application/json' } //Adiciona header
      });
    }
    catch(error) {
      console.log("Error: ", error); //Informa que houve erro
    }  
  };

  useEffect(() => {
    const interval = setInterval(FetchDataFromEsp32, 1000); //Busca informações no servidor a cada 1s
    return () => clearInterval(interval);
  }, [pacientsData]);

  return(
    <ScrollView>
      <View style = {styles.container}>
        {sortedPacients.map((data, index) => ( //Ordena chamados
          <View 
            key = {index} 
            style = {styles.callContainer}>
            <View style = {styles.pacientContainer}>
              <Text style = {styles.pacientTextStyle}>Paciente {data.pacient}</Text>
              <View style = {styles.infoContainer}>
                {data && (
                  <>
                    <Text style = {styles.callInfoText}>Timestamp: {data.timestamp}</Text>
                    <Text style = {styles.callInfoText}>Server MAC: {data.serverMAC}</Text>
                    <Text style = {styles.callInfoText}>Client MAC: {data.clientMAC}</Text>
                    <Button
                      title = "OK"
                      color = "midnightblue"
                      onPress = {() => ClearPacient(index, data.pacient,data.lampIP)} //Remove o chamado ativo ao pressionar o botão
                    />
                  </>
                )}
              </View>
              <View style = {styles.ledContainer}>
                {data ? (
                  <View 
                    style = {[
                      styles.activeLed,
                      data.priority === 1 && styles.greenLed,
                      data.priority === 2 && styles.yellowLed,
                      data.priority === 3 && styles.redLed,
                    ]} 
                  />
                ) : (
                  <View style = {styles.inactiveLed} />
                )}
              </View>
            </View>
          </View>
        ))}
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: { //Container da tela
    flex: 1,
    paddingHorizontal: 20,
    justifyContent: "flex-start",
    alignItems: "center",
    backgroundColor: "#eaeaea"
  },
  callContainer: { //Container do chamado
    width: "100%",
    marginVertical: 10,
    padding: 10,
    backgroundColor: "#fff",
    borderRadius: 10,
    shadowColor: "#000",
    shadowOffset: { 
      width: 0, 
      height: 2 
    },
    shadowOpacity: 0.3,
    shadowRadius: 3,
    position: "relative"
  },
  pacientContainer: { //Container do paciente
    width: "100%", 
    justifyContent: "flex-start",
    alignItems: "flex-start" 
  },
  infoContainer: { //container das informações do chamado
    marginTop: 10
  },
  pacientTextStyle: {
    fontSize: 18,
    fontWeight: "bold",
    textAlign: "left"
  },
  ledContainer: { //Container do Led
    position: "absolute",
    top: 10, 
    right: 10
  },
  activeLed: { //Led ATIVO
    width: 30,
    height: 30,
    borderRadius: 15
  },
  greenLed: {
    backgroundColor: "#008000" //Verde para prioridade 1
  },
  yellowLed: {
    backgroundColor: "#ffd700" //Amarelo dourado para prioridade 2
  },
  redLed: {
    backgroundColor: "#ff0000" //Vermelhor para prioridade 3
  },
  inactiveLed: { //Led INATIVO
    width: 30,
    height: 30,
    borderRadius: 15,
    backgroundColor: '#ccc'
  },
  callInfoText: {
    fontSize: 15,
    textAlign: "left"
  }
});