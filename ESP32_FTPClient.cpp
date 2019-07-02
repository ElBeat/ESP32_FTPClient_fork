#include <WiFiClient.h>
#include "ESP32_FTPClient.h"

ESP32_FTPClient::ESP32_FTPClient(char* _serverAdress, char* _userName, char* _passWord){
  userName = _userName;
  passWord = _passWord;
  serverAdress = _serverAdress;
}

WiFiClient* ESP32_FTPClient::GetDataClient() {
  return &dclient;
}

void ESP32_FTPClient::GetLastModifiedTime(char* fileName, char* result) {
  Serial.println("Send MDTM");
  client.print(F("MDTM "));
  client.println(F(fileName));
  GetFTPAnswer (result, 4);
}

void ESP32_FTPClient::WriteClientBuffered(WiFiClient* cli, unsigned char * data, int dataLength) {
  
  size_t clientCount = 0;
  for(int i = 0; i < dataLength;i++){
    clientBuf[clientCount] = data[i];
    //client.write(data[i])
    clientCount++;
    if (clientCount > bufferSize-1) {
        cli->write(clientBuf, bufferSize); 
      clientCount = 0;
    }
  }
  if (clientCount > 0){
      cli->write(clientBuf, clientCount); 
  }
}


void ESP32_FTPClient::GetFTPAnswer (char* result, int offsetStart) {
  byte respCode;
  byte thisByte;
  outCount = 0;
  while (!client.available()) delay(1);

  while (client.available()) {
    thisByte = client.read();
    if (outCount < 127) {
      outBuf[outCount] = thisByte;
      outCount++;
      outBuf[outCount] = 0;
    }
  }
  if(result != NULL){
    Serial.println("Result start");
    for(int i = offsetStart;i<21;i++){
      result[i] = outBuf[i - offsetStart];
    }
    Serial.print("Result: ");
    Serial.write(result);
    Serial.println("Result end");
  }
}

void ESP32_FTPClient::WriteData (unsigned char * data, int dataLength) {
  Serial.println(F("Writing"));
  WriteClientBuffered(&dclient, &data[0], dataLength);
}

void ESP32_FTPClient::CloseFile () {
  Serial.println(F("Close File"));
  dclient.stop();
  GetFTPAnswer();
}

void ESP32_FTPClient::Write(char * str) {
  GetDataClient()->print(str);
}

void ESP32_FTPClient::CloseConnection() {
  client.println(F("QUIT"));
  client.stop();
  Serial.println(F("Connection closed"));
}

void ESP32_FTPClient::OpenConnection() {
  Serial.print(F("Connecting to: "));
  Serial.println(serverAdress);
  if (client.connect(serverAdress, 21)) {  // 21 = FTP server
    Serial.println(F("Command connected"));
  } 
  GetFTPAnswer();
  

  Serial.println("Send USER");
  client.print(F("USER "));
  client.println(F(userName));
  GetFTPAnswer();

  Serial.println("Send PASSWORD");
  client.print(F("PASS "));
  client.println(F(passWord));
  GetFTPAnswer();
  
  Serial.println("Send SYST");
  client.println(F("SYST"));
  GetFTPAnswer();
}

void ESP32_FTPClient::RenameFile(char* from, char* to) {
  Serial.println("Send RNFR");
  client.print(F("RNFR "));
  client.println(F(from));
  GetFTPAnswer();

  Serial.println("Send RNTO");
  client.print(F("RNTO "));
  client.println(F(to));
  GetFTPAnswer();
}

void ESP32_FTPClient::NewFile (char* fileName) {
  Serial.println("Send STOR");
  client.print(F("STOR "));
  client.println(F(fileName));
  GetFTPAnswer();
}

void ESP32_FTPClient::InitFile(char* type){
  Serial.print("Send ");
  Serial.println(type);
  client.println(F(type));
  GetFTPAnswer();

  Serial.println("Send PASV");
  client.println(F("PASV"));
  GetFTPAnswer();

  char *tStr = strtok(outBuf, "(,");
  int array_pasv[6];
  for ( int i = 0; i < 6; i++) {
    tStr = strtok(NULL, "(,");
    array_pasv[i] = atoi(tStr);
    if (tStr == NULL) {
      Serial.println(F("Bad PASV Answer"));
    }
  }
  unsigned int hiPort, loPort;
  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;

  Serial.print(F("Data port: "));
  hiPort = hiPort | loPort;
  Serial.println(hiPort);
  if (dclient.connect(serverAdress, hiPort)) {
    Serial.println(F("Data connection etablished"));
  }
}

void ESP32_FTPClient::AppendFile (char* fileName) {
  Serial.println("Send APPE");
  client.print(F("APPE "));
  client.println(F(fileName));
  GetFTPAnswer();
}
