/*
Cria um "New File" no wokwi para colocar essa parte de codigo.

#ifndef SECRETS_H
#define SECRETS_H

const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

const String FIREBASE_HOST = "portaeletronica-a6cfe-default-rtdb.firebaseio.com";

const String FIREBASE_EMAIL    = "thauan.denver@gmail.com";
const String FIREBASE_PASSWORD = "16Thauan20D";
const String FIREBASE_API_KEY  = "AIzaSyChMG3yyK6WXnLhc-QgoL9CYcA62APqyIg";

#endif
*/

#include "secrets.h"
#include <WiFi.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>


#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS    17
#define SERVO_PIN   15
#define LED_VERDE   37
#define LED_VERMELHO 36


#define FIREBASE_USERS_PATH "/users"


Servo servo;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
WiFiClientSecure secureClient;

// Teclado
char teclas[4][4] = {
  {'1','2','3','C'},
  {'4','5','6','R'},
  {'7','8','9','U'},
  {'*','0','#','D'}
};
byte linhas[4] = {35, 34, 33, 26};
byte colunas[4] = {21, 20, 19, 18};
Keypad teclado = Keypad(makeKeymap(teclas), linhas, colunas, 4, 4);

String usuario = "";
String senha = "";
bool digitandoUsuario = true;
String idToken = "";
unsigned long tokenAcquiredMillis = 0;
const unsigned long TOKEN_VALIDITY_MS = 3500UL * 1000;


void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  
  lcd.clear();
  lcd.print("Conectando WiFi");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(250);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    lcd.clear();
    lcd.print("WiFi OK!");
    delay(1000);
  } else {
    Serial.println("\nFalha ao conectar WiFi");
    lcd.clear();
    lcd.print("WiFi FALHOU!");
    delay(2000);
  }
}

bool firebaseLogin() {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + FIREBASE_API_KEY;
  https.begin(client, url);
  https.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> doc;
  doc["email"] = FIREBASE_EMAIL;
  doc["password"] = FIREBASE_PASSWORD;
  doc["returnSecureToken"] = true;

  String payload;
  serializeJson(doc, payload);

  int httpCode = https.POST(payload);
  if (httpCode == HTTP_CODE_OK) {
    String resp = https.getString();
    StaticJsonDocument<512> resDoc;
    DeserializationError err = deserializeJson(resDoc, resp);
    if (!err) {
      idToken = resDoc["idToken"].as<String>();
      tokenAcquiredMillis = millis();
      Serial.println("Login Firebase OK!");
      https.end();
      return true;
    }
  } else {
    Serial.printf("Falha login: %d\n", httpCode);
  }
  https.end();
  return false;
}



bool verificarCredenciais(String user, String pass) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado!");
    return false;
  }
  
  if (idToken.isEmpty()) {
    Serial.println("Sem token válido!");
    return false;
  }

  // Buscar usuário
  String url = "https://" + FIREBASE_HOST + FIREBASE_USERS_PATH + "/" + user + ".json?auth=" + idToken;
  
  secureClient.setInsecure();
  HTTPClient http;
  http.begin(secureClient, url);
  
  int httpCode = http.GET();
  bool credenciaisValidas = false;
  
  if (httpCode == HTTP_CODE_OK) {
    String resp = http.getString();
    Serial.printf("Resposta Firebase: %s\n", resp.c_str());
    
    if (resp == "null") {
      Serial.println("Usuário não encontrado!");
    } else {
      StaticJsonDocument<256> doc;
      DeserializationError err = deserializeJson(doc, resp);
      
      if (!err) {
        String senhaArmazenada = doc["senha"].as<String>();
        
        if (senhaArmazenada == pass) {
          Serial.println("Credenciais válidas!");
          credenciaisValidas = true;
        } else {
          Serial.println("Senha incorreta!");
        }
      }
    }
  } else {
    Serial.printf("Erro HTTP: %d -> %s\n", httpCode, http.getString().c_str());
  }
  
  http.end();
  return credenciaisValidas;
}

void abrirPorta() {
  lcd.clear();
  lcd.print("ACESSO LIBERADO");
  digitalWrite(LED_VERDE, HIGH);
  
  servo.write(90);
  delay(3000);
  servo.write(0);
  
  digitalWrite(LED_VERDE, LOW);
  delay(1000);
}

void negarAcesso() {
  lcd.clear();
  lcd.print("ACESSO NEGADO!");
  digitalWrite(LED_VERMELHO, HIGH);
  delay(2000);
  digitalWrite(LED_VERMELHO, LOW);
}

void resetarTela() {
  usuario = "";
  senha = "";
  digitandoUsuario = true;
  lcd.clear();
  lcd.print("User:");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("Controle de Acesso Firebase");
  
  // Inicializar componentes
  Wire.begin(17, 16); // SDA=21, SCL=26
  servo.attach(SERVO_PIN);
  servo.write(0);
  
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.print("Iniciando...");
  
  connectWiFi();
  secureClient.setInsecure();
  firebaseLogin();
  
  digitalWrite(LED_VERMELHO, HIGH);
  delay(1000);
  digitalWrite(LED_VERMELHO, LOW);
  
  Serial.println("Sistema iniciado!");
  
  delay(2000);
  resetarTela();
}

void loop() {
  // Verificar WiFi periodicamente
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck >= 30000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) connectWiFi();
  }
  
  char tecla = teclado.getKey();
  
  if (tecla) {
    Serial.println(tecla);
    
    if (tecla != '#' && tecla != '*') {
      if ((usuario.length() < 4) || (senha.length() < 4)){
        if (digitandoUsuario) {
          usuario += tecla;
          lcd.setCursor(0, 1);
          lcd.print(usuario);
        } else {
          senha += tecla;
          lcd.setCursor(0, 1);
          String asteriscos = "";
          for (int i = 0; i < senha.length(); i++) {
            asteriscos += "*";
          }
          lcd.print(asteriscos);
        }
      } else {
        lcd.clear();
        lcd.print("Tamanho máximo atingido");
        delay(3000);
        resetarTela();
        }
    } 

    if (tecla == '#') {
      if (digitandoUsuario && usuario.length() > 0) {
        digitandoUsuario = false;
        lcd.clear();
        lcd.print("Senha:");
      } else if (!digitandoUsuario && senha.length() > 0) {
        lcd.clear();
        lcd.print("Verificando...");
        
        bool acessoPermitido = verificarCredenciais(usuario, senha);
        
        if (acessoPermitido) {
          abrirPorta();
        } else {
          negarAcesso();
        }
        
        resetarTela();
      }
    }
    
    // Tecla '*' limpa e recomeça
    if (tecla == '*') {
      resetarTela();
    }
  }
}