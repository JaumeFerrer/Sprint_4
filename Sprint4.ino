#include <Wire.h>      // Librería para usar el BUS I2C  
#include <Adafruit_ADS1X15.h> // Librería del ADS1115  
#include <ESP8266WiFi.h>
#define WiFi_CONNECTION_UPV
#define REST_SERVER_THINGSPEAK


  Adafruit_ADS1115 ads1115;//---> Incluir el constructor del ads1115  

//-------------------------------------------------------------------------------------------------------------------------------------------------------
//Declaración de variables.
//-------------------------------------------------------------------------------------------------------------------------------------------------------


 //Declaramos los distintas variables de calibración que necesita el sensor de humedad.

  const int AirValueH = 20300;  // Medimos valor en seco  

  const int WaterValueH = 9750;  // Medimos valor en agua  

  

   

  //Declaramos los distintas variables de calibración que necesita el sensor de salinidad.


  const int SinSal = 9715;  //9715-Sin sal 

  const int ConSal =19990; //19990-Con sal  





 //Declaramos que el power pin es el pin 5.

  int power_pin = 5; 

  //WiFi Definitions
  
  #ifdef WiFi_CONNECTION_UPV //Conexion UPV
    const char WiFiSSID[] = "GTI1";
    const char WiFiPSK[] = "1PV.arduino.Toledo";
  #else //Conexion fuera de la UPV
    const char WiFiSSID[] = "WifiMovil";
    const char WiFiPSK[] = "00000001";
  #endif

  //SERVER Definitions

  #if defined(WiFi_CONNECTION_UPV) //Conexion UPV
    const char Server_Host[] = "proxy.upv.es";
    const int Server_HttpPort = 8080;
  #elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
    const char Server_Host[] = "api.thingspeak.com";
   const int Server_HttpPort = 80;
  #else
   const char Server_Host[] = "dweet.io";
   const int Server_HttpPort = 80;
  #endif  

  WiFiClient client;

  //HTTP REST Connection

  #ifdef REST_SERVER_THINGSPEAK 
   const char Rest_Host[] = "api.thingspeak.com";
   String MyWriteAPIKey="44OYVWP35X6R9SBP"; // Escribe la clave de tu canal ThingSpeak
  #else 
   const char Rest_Host[] = "dweet.io";
   String MyWriteAPIKey="PruebaGTI"; // Escribe la clave de tu canal Dweet
  #endif

  #define NUM_FIELDS_TO_SEND 2 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

  // Pin Definitions

  const int LED_PIN = 5; // Thing's onboard, green LED
  
   // WiFi Connection
  
  void connectWiFi()
{
  byte ledStatus = LOW;

  #ifdef PRINT_DEBUG_MESSAGES
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  #endif
  
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    #ifdef PRINT_DEBUG_MESSAGES
       Serial.println(".");
    #endif
    delay(500);
  }
  #ifdef PRINT_DEBUG_MESSAGES
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
  #endif
}

  // HTTP POST  ThingSpeak

  void HTTPPost(String fieldData[], int numFields){

// Esta funcion construye el string de datos a enviar a ThingSpeak mediante el metodo HTTP POST
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar numFields al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
       
        // Construimos el string de datos. Si tienes multiples campos asegurate de no pasarte de 1440 caracteres
   
        String PostData= "api_key=" + MyWriteAPIKey ;
        for ( int field = 1; field < (numFields + 1); field++ ){
            PostData += "&field" + String( field ) + "=" + fieldData[ field ];
        }     
        
        // POST data via HTTP
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( "Connecting to ThingSpeak for update..." );
        #endif
        client.println( "POST http://" + String(Rest_Host) + "/update HTTP/1.1" );
        client.println( "Host: " + String(Rest_Host) );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( PostData.length() ) );
        client.println();
        client.println( PostData );
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( PostData );
            Serial.println();
            //Para ver la respuesta del servidor
            #ifdef PRINT_HTTP_RESPONSE
              delay(500);
              Serial.println();
              while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
              Serial.println();
              Serial.println();
            #endif
        #endif
    }
}
  // HTTP GET

  void HTTPGet(String fieldData[], int numFields){
  
// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
           #ifdef REST_SERVER_THINGSPEAK 
              String PostData= "GET https://api.thingspeak.com/update?api_key=";
              PostData= PostData + MyWriteAPIKey ;
           #else 
              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           #endif
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }
          
           
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( "Connecting to Server for update..." );
           #endif
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( PostData );
              Serial.println();
              //Para ver la respuesta del servidor
              #ifdef PRINT_HTTP_RESPONSE
                delay(500);
                Serial.println();
                while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
                Serial.println();
                Serial.println();
              #endif
           #endif  
    }
}
void setup() {
  Serial.begin(115200);  

  

  Serial.println("Inicializando...");  


  
  pinMode(power_pin,OUTPUT);//Configurar power_pin como pin de salida  



  ads1115.begin();//Inicializar el ads1115  

  

  ads1115.setGain(GAIN_ONE);//Ajustar la ganancia del ads1115    

    
#ifdef PRINT_DEBUG_MESSAGES
    Serial.begin(9600);
  #endif
  
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);

  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif
 
} 

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Loop. Donde se efectuan las operaciones y las configuraciones de los distintos operadores que se utilizan.ç
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------





void loop() {

    double S,H,T,L;

    S=salinidad();

    H=humedad();

    T=temperatura();

    L=luminosidad();
  
    String data[ NUM_FIELDS_TO_SEND + 1];  // Podemos enviar hasta 8 datos

    
    data[ 1 ] = String( S ); //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print(Salinidad );
    #endif

    data[ 2 ] = String( H ); //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print(Humedad);
    #endif

    data[ 3 ] = String( T ); //Escribimos el dato 3. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print(Temperatura);
    #endif

    data[ 4 ] = String( L ); //Escribimos el dato 4. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print(Luminosidad);
    #endif
    
    //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPPost( data, NUM_FIELDS_TO_SEND );

    //Selecciona si quieres un retardo de 15seg para hacer pruebas o dormir el SparkFun
    delay( 15000 );   
    //Serial.print( "Goodnight" );
    //ESP.deepSleep( sleepTimeSeconds * 1000000 );
    

  Serial.println("*******************************************");//Mostramos una linea por pantalla para que cuando se efectue el loop los datos se separen, cambiamos la forma de la separación para saber cuando empieza la siguiente vuelta del loop..
} 

int porcentage(int x, int y,int z){

  int a;
   
  a=100*x/(x-y)-z*100/(x-y);

  return a;
  
}

int tomarMedidas(int x){

  int a;

  a=ads1115.readADC_SingleEnded(x);

  return a;
}

double salinidad(){
  
  int x,salinidad;//Variables de la función.
   


  digitalWrite( power_pin, HIGH );//Poner power_pin a nivel alto  

  

  delay(100);//Esperar 100ms  

  

  x = tomarMedidas(0); //Muestrear la tensión del sensor de salinidad  

 

  digitalWrite( power_pin, LOW );//Poner power_pin a nivel bajo  


  salinidad = porcentage(SinSal,ConSal,x);

  
  if(salinidad<0){
    
    salinidad=0;
    
    }

   
  if(salinidad>100){
    
    salinidad=100;
    
    }

  Serial.print("La cantidad de sal que hay respecto al recipiente es del ");

  Serial.print(salinidad);  
  

  Serial.println("%");//---> Mostrar por el monitor serie el valor de salinidad en %  

  
  
  if(salinidad>=98){ 

      Serial.println("Alerta: El nivel de salinidad esta por encima de los parametros permitidos"); 
      
      } 

  return salinidad;
  
  Serial.println("-----------------------------------------------------------------------------------------------------");//Separamos el sensor de salinidad y humedad con una linea para que a la hora de mostrar los datos sea mas sencillo leer los datos
 
  }
double humedad(){
  
   int16_t adc0;  

  

  int16_t humedad;  

  

  adc0 =tomarMedidas(1);  


  
  humedad =porcentage(AirValueH,WaterValueH,adc0);
  
  

  Serial.print("AIN0: ");  

  

  Serial.println(adc0);  

  

  Serial.print("El sensor de humedad actualmente tiene un grado de humedad del ");  

  

  Serial.print(humedad);  

  

  Serial.println("%");  

  
//Creamos unos if para que en los distintos valores distintos que puede detectar el sensor nos diga si se encuentra seco, humedo o mojado.

  Serial.print("Por tanto podemos decir que el sensor esta ");

  if(humedad>50){  

     Serial.println("Mojado");  
 
      }

  if(humedad<=50 && humedad>=10){

    Serial.println("Humedo");  

    }

  if(humedad<5){

    Serial.println("Seco");  

    }

  return humedad; 

  Serial.println("-----------------------------------------------------------------------------------------------------");//Separamos el sensor de temperatura y humedad con una linea para que a la hora de mostrar los datos sea mas sencillo leer los datos

  }

double temperatura(){
  
  double y,T;//Configuramos las variables.
 
 

  y = tomarMedidas(2);//Muestreamos la tensión del sensor de temperatura.
  


  T=((((y*4.096)/pow(2,15)-1)-0.786)/0.034);//Aplicamos la formula que creamos en la practica de electronica.

  
  
  delay(500);


  Serial.println("Hay una temperatura ambiente de ");

  Serial.print(T);

  Serial.print("ºC");
  
  return T;
  
  Serial.println("-----------------------------------------------------------------------------------------------------");

  }

 double luminosidad(){
  
  double luz;

  luz = tomarMedidas(3);//Muestreamos la tensión del sensor de iluminación

  if(luz>20000){  

     Serial.println("Hace un dia soleado.");  
 
      }

  if(luz<=20000 && luz>=10000){

    Serial.println("Hay nubes pero sigue haciendo sol.");  

    }

  if(luz<10000){

    Serial.println("Es un dia con muchas nubes, hay muy poca luz.");  

  }

  delay(500);

  return luz;
 }
