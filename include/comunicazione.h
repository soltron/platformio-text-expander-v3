
byte MAX_IN_DATA = 30; // utilizzo per la seriale (massima grandezza caratteri in ingresso)
unsigned long time_now_rx=0;
unsigned long tick=0;
String inputString = "";
String tempString = "";         // a String to hold incoming data
byte on_com =0;
////////////////////////////////////////

////////////////////////////////////////
byte checkcomm() {
  // individuo inizio stringa comando FF 0A
  
  char test = 0xff;
  if (tempString[0] == test) {  // Verifico ci sia la sequenza FF 
    test = 0x0a;
    if (tempString[1] == test) { // Verifico ci sia indirizzo >0A
      return(3); // al byte 3 c'è il byte comando
    }
  }
  return(0);
}
//////////////////////////////////////// CECKSUM //////////////////////////////
byte cecksum(byte pos_cmd) {
  byte chk = tempString[pos_cmd-1]; //inizio la somma partendo dall'Address
  byte lunghezza = tempString[pos_cmd+1];
  byte ckval_com = tempString[pos_cmd+2+lunghezza];
  for (byte x =0; x<=lunghezza+1;x++) {
    byte dato = tempString[x+pos_cmd];
    chk = chk + dato;
  }
    byte res_chk = 0 - chk;
  if (res_chk == ckval_com){
    return res_chk; 
  }
    else {
      return (0);    
    }
}

 /*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    on_com++;
    if (on_com>MAX_IN_DATA) { //Controllo lunghezza inputString...
      on_com=1;
      inputString="";
    }
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
  }
  time_now_rx = tick; 
}