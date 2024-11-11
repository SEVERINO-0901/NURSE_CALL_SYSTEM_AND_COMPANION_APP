import { NavigationContainer } from "@react-navigation/native";
import { createNativeStackNavigator } from "@react-navigation/native-stack";
import CallScreen from "./screens/CallScreen";

const Stack = createNativeStackNavigator();



export default function App() {
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
