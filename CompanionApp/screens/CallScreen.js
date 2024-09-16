import { 
    StyleSheet, 
    Text, 
    View 
} from "react-native";

export default function CallScreen() {
  return (
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
  );
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