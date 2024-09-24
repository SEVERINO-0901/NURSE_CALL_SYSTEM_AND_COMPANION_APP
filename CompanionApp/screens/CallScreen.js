import React, { useState, useEffect } from "react";
import { 
    StyleSheet, 
    Text, 
    View 
} from "react-native";
import axios from "axios";

export default function CallScreen() {
  const [data, setData] = useState(null);

  const FetchData = async () => {
    try{
      const espData = await axios.get("http://192.168.0.224:3000/data");
      setData(espData.data);
    }
    catch(error){
      console.log("Error: ", error);
    }  
  };

  useEffect(() => {
    FetchData();
    const interval = setInterval(FetchData, 1000);
    return () => clearInterval(interval);
  }, []);
  
  return(
    <View style={styles.container}>
      {data ? (
        <>
          <Text style={styles.text}>Paciente: {data.pacient}</Text>
          <Text style={styles.text}>Horário: {data.timestamp}</Text>
          <Text style={styles.text}>ESP32 MAC: {data.esp32MAC}</Text>
          <Text style={styles.text}>Client MAC: {data.clientMAC}</Text>
        </>
      ) : (
        <Text style={styles.text}>Carregando dados...</Text>
      )}
    </View>
  );
    {/*
    <View style = {styles.container}>
      <View style = {styles.callContainer}>
        <View style = {styles.pacientBox}>
            <Text style = {styles.textStyle}>Paciente 1</Text>
        </View>
        <View style = {styles.ledBox}>
            <View style = {styles.bar}/>
        </View>
      </View>
      <View style = {styles.callContainer}>
        <View style = {styles.pacientBox}>
            <Text style = {styles.textStyle}>Paciente 2</Text>
        </View>
        <View style = {styles.ledBox}>
            <View style = {styles.bar}/>
        </View>
      </View>
      <View style = {styles.callContainer}>
        <View style = {styles.pacientBox}>
            <Text style = {styles.textStyle}>Paciente 3</Text>
        </View>
        <View style = {styles.ledBox}>
            <View style = {styles.bar}/>
        </View>
      </View>
    </View>
    */}
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    paddingHorizontal: 20,
    justifyContent: "space-around",
    alignItems: "flex-start",
    backgroundColor: "#f2f2f2"
  },
  callContainer: {
    width: "100%",
    height: "30%",
    padding: 10,
    flexDirection: "row",
    justifyContent: "center",
    alignItems: "center", 
  },
  pacientBox: {
    width: "70%",
    height: "35%",
    padding: 10,
    borderWidth: 2,
    justifyContent: "center",
    alignItems: "center",
    borderColor: "#000",
    backgroundColor: "#fff",
  },
  ledBox: {
    width: "30%",
    height: "35%",
    padding: 10,
    borderWidth: 2,
    justifyContent: "center",
    alignItems: "center",
    borderColor: "#000",
    backgroundColor: "#fff",
  },
  textStyle: {
    fontSize: 18,
    fontWeight: "condensedBold",
    textAlign: "center"  
  },
  bar: {
    width: "50%",
    height: "80%",
    marginHorizontal: 5,
    backgroundColor: '#ccc'
  },
});