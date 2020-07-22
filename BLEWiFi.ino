#include <WiFi.h>
#include <HTTPClient.h> 
#include <string.h>
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>

#define SCAN_TIME  10 // seconds
#define SLEEP_TIME  10 // seconds
#define SERIAL_PRINT

//Nome da rede
const char* ssid = "****";
//Senha da rede
const char* password =  "****";

IPAddress ip(192,168,0,250);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);

//Cria um objeto BLEScan
BLEScan *pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice){
      Serial.printf("Advertised Device: %s | rssi %d \n", advertisedDevice.toString().c_str());
    }
};

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a rede WiFi..");
  }

  Serial.println("Conectado a rede!");

  #ifdef SERIAL_PRINT
  Serial.println("ESP32 BLE Scanner");
  #endif

  BLEDevice::init("");

  // put your main code here, to run repeatedly:
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(0x50);
  pBLEScan->setWindow(0x30);

}

void loop() {
  
  //String que armazena os dados que vão ser enviados
  String mensagem;
  //String que armazena o IP do iBecon
  String endereco;
  //String com o endereço IP fixo na rede
  String IP = "192.168.0.250";
  //Armazena a intensidade do sinal (RSSI)
  int valor;
 

  Serial.printf("Iniciando o scan por %d segundos...\n", SCAN_TIME);
  //Inicia o scan
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  //Quantidade de dispositivos BLE encontrados
  int count = foundDevices.getCount();
 
  //Verifica o status da conexão
  if(WiFi.status()== WL_CONNECTED){
    
    //Cria um objeto HTTPCliente
    HTTPClient http;
   
    //Link do servidor composto pelo IP do Raspberry e a porta
    http.begin("http://192.168.0.103:8090/post");

    //Especifica o tipo de conteudo que vai ser enviado
    //Content-Type informa o tipo de conteudo que vai ser enviado
    //text/plain envia o conteudo descartando os simbolos
    http.addHeader("Content-Type", "text/plain");             

    //*aqui a string deve receber os valores de rssi e id*
    for(int i = 0; i < count; i++){
      
      BLEAdvertisedDevice d = foundDevices.getDevice(i);
      endereco = d.getAddress().toString().c_str();
      valor = d.getRSSI();
      mensagem = endereco + '|' + valor + '|' + IP;
      #ifdef SERIAL_PRINT
      Serial.printf("Endereco: %s | Rssi: %d \n", d.getAddress().toString().c_str(), valor);
      #endif 
      
      //Envia a mensagem e retorna um valor. Caso ocorra algum erro, o valor retornado é menor que 0
      int httpResponseCode = http.POST(mensagem);

      if(httpResponseCode>0){
      
      Serial.println(httpResponseCode); 
    
      }else{

      Serial.println("Error on sending POST");

      }
      
    }
   
   //Libera os recursos
   http.end();
  
  }else{

    Serial.println("Error na conexão WiFi");   

 }
  
  delay(10000);
}
