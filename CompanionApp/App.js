/*
NOME DO PROJETO: CompanionApp                                                           
AUTOR: Leonardo Severino - leoseverino0901@gmail.com
DESCRIÇÃO:
  Implementação em React Native de uma aplicação para interagir com o sistema de chamadas de enfermagem, permitindo ao usuário visualizar os dados da chamada para que este possa responder ao chamado do paciente.
*/

//Importação dos componentes
import { NavigationContainer } from "@react-navigation/native";
import { createNativeStackNavigator } from "@react-navigation/native-stack";
import CallScreen from "./screens/CallScreen";

const Stack = createNativeStackNavigator(); //Stack de navegação



export default function App(){ //Função 'App'
  return (
    <NavigationContainer>
      <Stack.Navigator
        initialRouteName = "CallsScreen"
      >
        <Stack.Screen 
          name = "CallsScreen"
          component = {CallScreen}
        />
      </Stack.Navigator>
    </NavigationContainer>
  );
}
