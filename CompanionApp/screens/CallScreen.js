import React, { useState, useEffect } from "react";
import { 
    StyleSheet, 
    Text, 
    View,
    Button, 
    ScrollView
} from "react-native";
import axios from "axios";

export default function CallScreen() {
  const [pacientsData, setPacientsData] = useState([null, null, null, null, null, null, null, null]);

  const FetchData = async () => {
    try{
      const espData = await axios.get("http://192.168.0.224:3000/data");
      const pacientIndex = espData.data.pacient - 1; // Índice do paciente

      if((pacientIndex >= 0) && (pacientIndex <= 2)){
        setPacientsData(prevData => {
          const updatedData = [...prevData];

          updatedData[pacientIndex] = espData.data;
          return updatedData; //Atualiza o paciente
        });
      }
    }
    catch(error){
      console.log("Error: ", error);
    }  
  };

  const ClearPacient = (index) => {
    setPacientsData(prevData => {
      const updatedData = [...prevData];

      updatedData[index] = null; //Limpa os dados do paciente
      return updatedData;
    });
  };

  useEffect(() => {
    FetchData();
    const interval = setInterval(FetchData, 1000);
    return () => clearInterval(interval);
  }, []);
  
  return(
    <ScrollView>
      <View style = {styles.container}>
        {pacientsData.map((data, index) => (
          <View 
            key={index} 
            style={styles.callContainer}>
            <View style = {styles.pacientContainer}>
              <Text style = {styles.pacientTextStyle}>Paciente {index + 1}</Text>
              <View style = {styles.infoContainer}>
                {data && (
                  <>
                    <Text style = {styles.callInfoText}>Timestamp: {data.timestamp}</Text>
                    <Text style = {styles.callInfoText}>Server MAC: {data.serverMAC}</Text>
                    <Text style = {styles.callInfoText}>Client MAC: {data.clientMAC}</Text>
                    <Button
                      title = "OK"
                      color = "midnightblue"
                      onPress = {() => ClearPacient(index)}
                    />
                  </>
                )}
              </View>
              <View style = {styles.ledContainer}>
                {data ? (
                  <View style = {styles.activeLed} />
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
    backgroundColor: "#f2f2f2",
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
  inactiveLed: {
    width: 30, // Aumentando o tamanho do LED
    height: 30,
    borderRadius: 15,
    backgroundColor: '#ccc',
  },
  activeLed: {
    width: 30, // Aumentando o tamanho do LED
    height: 30,
    borderRadius: 15,
    backgroundColor: '#4caf50',
  },
  callInfoText: {
    fontSize: 15,
    textAlign: "left",
  }
});