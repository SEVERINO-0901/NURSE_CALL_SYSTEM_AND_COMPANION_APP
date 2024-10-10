import React, { useState, useEffect } from "react";
import { 
    StyleSheet, 
    Text, 
    View,
    Button, 
    ScrollView,
    Alert
} from "react-native";
import axios from "axios";

export default function CallScreen() {
  const [pacientsData, setPacientsData] = useState([null, null, null, null, null, null, null, null]);

  const FetchDataFromEsp32 = async () => {
    try {
      const response = await axios.get("http://192.168.0.225:3000/getCalls");
  
      console.log(response.data);
      if (response.status === 200) {
        const espData = response.data;
        
        // Atualiza o estado garantindo uma chamada por paciente (clientMAC como chave)
        setPacientsData(prevData => {
          // Encontra se o paciente já está presente na lista
          const updatedData = [...prevData];
          const pacientIndex = updatedData.findIndex(pacient => pacient && pacient.clientMAC === espData.clientMAC);
          
          if (pacientIndex !== -1) {
            // Substitui a chamada existente para o mesmo paciente
            updatedData[pacientIndex] = espData;
          } else {
            // Se não houver chamada para o paciente, encontra o primeiro índice null para adicionar a nova chamada
            const firstNullIndex = updatedData.findIndex(pacient => pacient === null);
            if (firstNullIndex !== -1) {
              updatedData[firstNullIndex] = espData;
            } else {
              // Se não houver espaço, adiciona ao final da lista
              updatedData.push(espData);
            }
          }
  
          Alert.alert("!!!NEW CALL!!!", `Pacient: ${espData.pacient}\nPriority: ${espData.priority}\n`);
          return updatedData;
        });
      } else {
        console.log("No new calls");
      }
    } catch (error) {
      console.log("Error: ", error);
    }
  };

  const ClearPacient = (index, ip) => {
    sendDataToEsp32(ip);
    setPacientsData(prevData => {
      const updatedData = [...prevData];
  
      if (updatedData[index] !== null) {
        updatedData[index] = null; // Limpa os dados do paciente
      }
  
      // Filtra quaisquer valores 'null' ou inválidos que possam ter sobrado
      const cleanedData = updatedData.filter(pacient => pacient !== null);
  
      console.log('Pacients after clear:', cleanedData);
      return cleanedData;
    });
  };

  const sortPacients = (data) => {
    return data
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

  const sendDataToEsp32 = async(ip) => {
    try{
      const response = await fetch("http://192.168.0.225:3000/sendOff", {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: `data=${encodeURIComponent(ip)}` //Envio do IP do esp32 que deve ser desligado  
      });
      const result = await response.text();
      console.log(result);
    }
    catch(error){
      console.log("Error: ", error);
    }  
  };

  useEffect(() => {
    FetchDataFromEsp32();
    const interval = setInterval(FetchDataFromEsp32, 1000);
    return () => clearInterval(interval);
  }, [pacientsData]);
  
  const sortedPacients = sortPacients(pacientsData);

  return(
    <ScrollView>
      <View style = {styles.container}>
        {sortedPacients.map((data, index) => (
          <View 
            key = {index} 
            style = {styles.callContainer}>
            {console.log('Rendering Pacient:', data)}
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
                      onPress = {() => ClearPacient(index, data.lampIP)}
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
  container: {
    flex: 1,
    paddingHorizontal: 20,
    justifyContent: "flex-start",
    alignItems: "center",
    backgroundColor: "#eaeaea",
  },
  callContainer: {
    width: "100%",
    marginVertical: 10,
    padding: 10,
    backgroundColor: "#fff",
    borderRadius: 10,
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.3,
    shadowRadius: 3,
    position: "relative", // Necessário para posicionar o LED
  },
  pacientContainer: {
    width: "100%", 
    justifyContent: "flex-start",
    alignItems: "flex-start", 
  },
  infoContainer: {
    marginTop: 10, 
  },
  pacientTextStyle: {
    fontSize: 18,
    fontWeight: "bold",
    textAlign: "left",
  },
  ledContainer: {
    position: "absolute",
    top: 10, 
    right: 10, // Posiciona o LED no canto superior direito
  },
  activeLed: {
    width: 30, // Aumentando o tamanho do LED
    height: 30,
    borderRadius: 15,
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
  inactiveLed: {
    width: 30, // Aumentando o tamanho do LED
    height: 30,
    borderRadius: 15,
    backgroundColor: '#ccc',
  },
  callInfoText: {
    fontSize: 15,
    textAlign: "left",
  }
});