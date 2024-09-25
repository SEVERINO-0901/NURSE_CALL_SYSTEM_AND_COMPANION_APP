import { useState, useEffect } from 'react';
import { 
    StyleSheet, 
    Text,
    View, 
    Platform,
    SafeAreaView
} from 'react-native';
import NetInfo from '@react-native-community/netinfo';

const SignalBar = ({active}) => (
  <View style = {[styles.bar, active && styles.activeBar]}/>
);

export default function InfoScreen({route}){
  const { data } = route.params;
  const [isConnected, setIsConnected] = useState("");
  const [connectionType, setConnectionType] = useState("");
  const [wifiStrength, setWifiStrength] = useState(0);  

  useEffect(() => {
    const checkConnection = async () => {
      const state = await NetInfo.fetch();

      setIsConnected(state.isConnected);
      setConnectionType(state.type);
      //Atualiza a força do sinal
      if(state.details && state.details.strength !== undefined){
        setWifiStrength(state.details.strength);
      }
      else{
        setWifiStrength(0);
      }
    };
    // Verificar a conexão imediatamente
    checkConnection();
    // Configura um intervalo para verificar a conexão a cada 1 segundo
    const intervalId = setInterval(checkConnection, 1000);

    // Cleanup na desmontagem
    return () => clearInterval(intervalId);
  }, []);

  const GetFilledBars = () => { // Função para mapear a força do sinal para o número de barras preenchidas
    if(wifiStrength >= 67){
      return 3;
    }
    else if(wifiStrength >= 34){
      return 2;
    }
    else if(wifiStrength > 0){
      return 1;
    }
    else{
      return 0;
    }
  };

  const RenderBars = () => {
    const bars = 3;
    const FilledBars = GetFilledBars();

    return Array.from({length: bars}, (_, index) => (
      <SignalBar 
        key = {index}
        active = {index < FilledBars}
      />
    ));
  };

  return (
    <SafeAreaView style = {styles.container}>
        <View style = {[styles.box, styles.boxShadow]}>
            <Text style = {styles.textINFORMACOES}>INFORMAÇÕES</Text>
            
            <Text style = {styles.textLabel}>Device ID:</Text>
            <Text style = {styles.textInfo}>{data.deviceID}</Text>

            <Text style = {styles.textLabel}>Timestamp:</Text>
            <Text style = {styles.textInfo}>{data.timestamp}</Text>
            
            <Text style = {styles.textLabel}>Endereço IP:</Text>
            <Text style = {styles.textInfo}>{data.ipAddress}</Text>
            
            <Text style = {styles.textLabel}>Coordenadas Geográficas:</Text>
            <Text style = {styles.textInfo}>{data.location.latitude}</Text>
            <Text style = {styles.textInfo}>{data.location.longitude}</Text>
            
            <Text style = {styles.textLabel}>Usuário:</Text>
            <Text style = {styles.textInfo}>{data.user}</Text>
            
            <Text style = {styles.textLabel}>Senha:</Text>
            <Text style = {styles.textInfo}>{data.password}</Text>

            <Text style = {styles.textLabel}>Sinal WI-FI:</Text>
            <View style={styles.signalContainer}>
              {RenderBars()}
            </View>
        </View>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    paddingHorizontal: 20,
    justifyContent: "center",
    alignItems: "center",
    backgroundColor: "#f2f2f2"
  },
  box: {
    width: "90%",
    height: "77%",
    padding: 10,
    borderWidth: 2,
    backgroundColor: "white",
    borderColor: "black",
    alignItems: "center",
    justifyContent: "flex-start"
  },
  boxShadow: {
    ...Platform.select({
      ios: {
        shadowOffset: {
        widht: 6,
        height: 6
        },
        shadowOpacity: 0.6,
        shadowRadius: 4,
        shadowColor: "#333333",
      },
      android: {
        elevation: 10
      }
    })
  },
  textINFORMACOES: {
    fontSize: 24,
    fontWeight: "bold",
    textAlign: "center"
  },
  textLabel: {
    fontSize: 18,
    fontWeight: "condensedBold",
    textAlign: "center"
  },
  textInfo: {
    fontSize: 16,
    marginBottom: 1,
    fontWeight: "regular",
    textAlign: "center"
  },
  signalContainer: {
    flexDirection: 'row',
    alignItems: 'flex-end',
    marginTop: 10,
  },
  bar: {
    width: 10,
    height: 30,
    marginHorizontal: 5,
    backgroundColor: '#ccc'
  },
  activeBar: {
    backgroundColor: '#4caf50'
  },
});