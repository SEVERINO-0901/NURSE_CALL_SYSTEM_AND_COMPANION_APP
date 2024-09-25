import { NavigationContainer } from "@react-navigation/native";
import { createNativeStackNavigator } from "@react-navigation/native-stack";
import LoginScreen from "./screens/LoginScreen";
import InfoScreen from "./screens/InfoScreen";

const Stack = createNativeStackNavigator();

export default function App() {
  return(
    <NavigationContainer>
      <Stack.Navigator
        initialRouteName = "Login"
      >
        <Stack.Screen 
          name = "Login"
          component = {LoginScreen}
        />
        <Stack.Screen 
          name = "Info"
          component = {InfoScreen}
          initialParams = {{
            user: "Guest"
          }}
        />
      </Stack.Navigator>
    </NavigationContainer>
  );
}
