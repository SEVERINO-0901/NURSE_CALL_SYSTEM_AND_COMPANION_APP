import React, { useState, useEffect } from "react";
import { 
  StyleSheet, 
  Text, 
  View,
  Platform, 
  TextInput,
  Button,
  Alert,
  BackHandler
} from "react-native";
import { useNavigation } from "@react-navigation/native";
import * as Application from 'expo-application';
import * as Location from "expo-location";
import * as Network from "expo-network";

export default function LoginScreen() {
  const navigation = useNavigation();
  const [permissionGranted, setPermissionGranted] = useState(false);
  const [user, setUser] = useState("");
  const [password, setPassword] = useState("");

  useEffect(() => {
    const requestPermissions = async () => {
      try{
        const { status } = await Location.requestForegroundPermissionsAsync();
        if(status !== 'granted'){
          Alert.alert('Permissão necessária', 'Permissão de localização negada.');
          return;
        }
        setPermissionGranted(true);
      }
      catch(error){
        Alert.alert('Erro', 'Erro ao solicitar permissões: ' + error.message);
      }
    };
    requestPermissions();
  }, []);

  const handleOk = async () => {
    if(user === "" || password === ""){
      Alert.alert("Dados de Login INCOMPLETOS");
      return;
    }
    else if(!permissionGranted){
      Alert.alert('Permissão necessária', 'Não foi possível obter as permissões necessárias.');
      return;
    }
    else{
      try{
        const deviceID = Application.getAndroidId() || Application.getIosIdForVendorAsync(); //Device ID
        //Timestamp
        const currentTimestamp = new Date();
        const formattedTimestamp = currentTimestamp.toLocaleString('pt-BR', {
          day: "2-digit",
          month: "2-digit",
          year: "2-digit",
          hour: "2-digit",
          minute: "2-digit",
          second: "2-digit"
        });
        const location = await Location.getCurrentPositionAsync({}); //Localizacao
        const ipAddress = await Network.getIpAddressAsync(); //Endereço IP
        const data = {
          deviceID,
          timestamp: formattedTimestamp,
          ipAddress,
          location: {
            latitude: location.coords.latitude,
            longitude: location.coords.longitude
          },
          user,
          password
        };
        navigation.navigate("Info", { data });
      }
      catch(error){
        Alert.alert('Erro', 'Erro ao coletar dados: ' + error.message);
      }
    }
  };

  const handleCancel = () => {
    setUser("");
    setPassword("");
    BackHandler.exitApp();
  };

  return (
    <View style={styles.container}> 
      <View style={[styles.box, styles.boxShadow]}> 
        <Text style={styles.textPalmaSistemas}>Palma Sistemas</Text> 

        <View style={styles.inputContainer}> 
          <Text style={styles.inputLabel}>Usuário: </Text> 
          <TextInput
            style={styles.inputBox}
            placeholder=""
            value={user}
            onChangeText={setUser}
          />
        </View>
        
        <View style={styles.inputContainer}> 
          <Text style={styles.inputLabel}>Senha: </Text> 
          <TextInput 
            style={styles.inputBox}
            placeholder=""
            value={password}
            onChangeText={setPassword}
            secureTextEntry={true}
          />
        </View>

        <View style={styles.buttonContainer}> 
          <View style={styles.button}> 
            <Button
              title="OK"
              color="black"
              onPress={handleOk}
            />
          </View>
          <View style={styles.button}>
            <Button
              title="Cancelar"
              color="black"
              onPress={handleCancel}
            />
          </View>
        </View>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    paddingHorizontal: 20,
    justifyContent: "center",
    alignItems: "center",
    backgroundColor: "#f2f2f2"
  },
  box: {
    width: 300,
    height: 250,
    padding: 10,
    backgroundColor: "white",
    borderWidth: 2,
    borderColor: "black",
    alignItems: "center",
    justifyContent: "flex-start"
  },
  boxShadow: {
    ...Platform.select({
      ios: {
        shadowColor: "#333333",
        shadowOffset: {
          width: 6,
          height: 6
        },
        shadowOpacity: 0.6,
        shadowRadius: 4
      },
      android: {
        elevation: 10
      }
    })
  },
  textPalmaSistemas: {
    fontSize: 24,
    marginBottom: 30,
    fontWeight: "bold",
    textAlign: "center"
  },
  inputContainer: {
    marginBottom: 20,
    flexDirection: "row",
    alignItems: "center",
    width: "100%"  
  },
  inputLabel: {
    fontSize: 16,
    width: 80
  },
  inputBox: {
    flex: 1,
    borderWidth: 1,
    padding: 5,
    borderRadius: 5,
    borderColor: "#000",
    backgroundColor: "#fff"
  },
  buttonContainer: {
    flexDirection: "row",
    justifyContent: "space-between"
  },
  button: {
    flex: 1,
    marginHorizontal: 5
  }
});
