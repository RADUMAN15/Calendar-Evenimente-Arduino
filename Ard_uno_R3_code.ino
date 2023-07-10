#include "RTClib.h"
#include <LiquidCrystal.h>
#include <Wire.h>
#include <EEPROM.h>
 
const byte EEPROM_ID=0x99;
const int ID_ADDR = 0;
const int NR_EVENTS_ADDR = 1;
const int buttonPin1 = A3;     // the number of the pushbutton pin
const int buttonPin2 = A2;     // the number of the pushbutton pin
const int buttonPin3 = A1;
int current_state1, current_state2, current_state3;
byte nr_events=0;
char incomingByte;
RTC_DS3231 rtc;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
void setup()
{
  Serial.begin(9600);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  byte id=EEPROM.read(ID_ADDR);
  if (id == EEPROM_ID) // ma asigur ca aplicatia mea e cea care a folosit ultima oara EEPROM-ul ca sa scrie events
  {
    nr_events = EEPROM.read(NR_EVENTS_ADDR);
  }
  else
  {
    EEPROM.write(ID_ADDR,EEPROM_ID);
    EEPROM.write(NR_EVENTS_ADDR,0);
  }

  #ifndef ESP8266
  while (!Serial);
  #endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  analogWrite(6,15); //setez contrast-ul de pe pin-ul 6
  lcd.begin(16, 2); //Initialize the 16x2 LCD
}
void loop() {

  DateTime now = rtc.now();
  incomingByte='0';
  if (Serial.available())
  {       
    current_state1=digitalRead(buttonPin1);
    current_state2=digitalRead(buttonPin2);
    current_state3=digitalRead(buttonPin3);
    if(current_state1==0)
        incomingByte='1';
      else if(current_state2==0)
          incomingByte='2';
        else if(current_state3==0)
          incomingByte='3';
        else incomingByte='0';
    if (incomingByte=='1')
    {
      if (nr_events<10)
      { 
        Serial.println("Introduceti datele despre eveniment ($ pentru final):");   
        Serial.println("Formatul folosit: ZZ-LL-YYYY (HH:MM) -> Text eveniment (maxim 50 chars)");
        lcd.clear();
        lcd.print("Adauga eveniment"); 
        char event[100]="";
        int poz=0;
        while(incomingByte != '$' && poz<95)
        {
          if (Serial.available()){          
            incomingByte = Serial.read();          
            event[poz++]=incomingByte;
          }
        }      
        event[poz-1]=0;
        if(Format_corect(event) && Data_corect(event) && Timp_corect(event))
        {
          Serial.println();
          Serial.println("Eventimentul nou introdus:");
          lcd.setCursor(0,0);
          lcd.print(" Ati adaugat un ");
          lcd.setCursor(0,1);
          lcd.print(" eveniment nou! ");
          delay(4000);
          Serial.println(event);
          
          afisare_data(event);
             
          for(int i=0; i<100; i++)
            EEPROM.write(10 + i + nr_events*100, event[i]);
          nr_events++;
          EEPROM.write(NR_EVENTS_ADDR,nr_events);  
        }
        else
        {
           if(Format_corect(event)==0)
           {
               Serial.println("Formatul introdus este incorect! Introduceti din nou evenimentul!");
               lcd.clear();
               lcd.setCursor(0,0); 
               lcd.print(" Format gresit!");  
               lcd.setCursor(0,1);
               lcd.print(" Scrie din nou!");
           }
           else
           {
                 if(Data_corect(event)==0)
                 {
                     Serial.println("Data introdusa este incorecta! Introduceti din nou evenimentul!");
                     lcd.clear();
                     lcd.setCursor(0,0);
                     lcd.print("Data gresita!");  
                     lcd.setCursor(0,1);
                     lcd.print(" Scrie din nou!");
                 }
                 if(Timp_corect(event)==0)
                 {
                     Serial.println("Timpul introdus este incorect! Introduceti din nou evenimentul!");
                     lcd.clear();
                     lcd.setCursor(0,0); 
                     lcd.print("Timp gresit!");  
                     lcd.setCursor(0,1);
                     lcd.print(" Scrie din nou!");
                 }
           }
        }      
      }
      else
      {
         Serial.println("ATI ATINS NUMARUL MAXIM DE EVENIMENTE (10).");
         lcd.clear();
         lcd.setCursor(0,0); 
         lcd.print(" NUMARUL  MAXIM ");  
         lcd.setCursor(0,1);
         lcd.print("DE EVENIMENTE:10");
      }
    }
    if (incomingByte=='2')
    {
      if (nr_events==0){
        Serial.println();
        Serial.println("Nu aveti nici un eveniment setat.");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Nu puteti sterge");
        lcd.setCursor(0,1);
        lcd.print("NR Evenimente: 0"); 
      }
      else
      {
        Serial.println("Pentru a sterge un eveniment scrie numarul acestuia urmat de $");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Pentru a sterge:");
        lcd.setCursor(0,1);
        lcd.print("Scrie nr. event.");
        char event[4]="";
        int poz=0;
        while(incomingByte!='$' && poz<4)
        {
          if (Serial.available()){          
            incomingByte = Serial.read();          
            event[poz++]=incomingByte;
          }
        }
        event[poz-1]=0;      
        int id_event=0;
        for(byte i=1;i<poz-1;i++)
          id_event = id_event*10 +  event[i]-'0';        

        if(id_event<=nr_events && id_event>=1)
        {
            Serial.print("Vom sterge evenimentul cu numarul: ");
            Serial.println(id_event);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Evenimentul: ");
            lcd.print(id_event);
            lcd.setCursor(0,1);
            lcd.print("  Va fi sters  "); 
            
            char eventd[100]="";
            for(byte i=id_event; i<nr_events; i++)             
            {    for(byte j=0; j<100; j++)
                    eventd[j]=EEPROM.read(10+j+i*100); ///iau event-ul urmator 
                    
                for(byte k=0; k<100; k++)
                     EEPROM.write(10 + k + (id_event-1)*100, eventd[k]); ///il copiez in locul celui anterior
                
            }
            nr_events--;
            EEPROM.write(NR_EVENTS_ADDR,nr_events);
        }
        else
        {
          Serial.print("Nu sunt inregistrate atat de multe evenimente!");
          Serial.println("Introduceti alta valoare!");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Nr evenimente: ");
          lcd.print(nr_events);
          lcd.setCursor(0,1);
          lcd.print("NU EXISTA EVENT!"); //----------> aici am un bug (uneori nu-mi arata id_event si imi pune un char '-')
          //lcd.print(id_event);
        }
      }
    }
    if (incomingByte=='3')
    {
      if (nr_events==0){
        Serial.println();
        Serial.println("Nu aveti nici un eveniment setat.");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Nu aveti nici un");
        lcd.setCursor(0,1);
        lcd.print("eveniment setat!");
      }
      else
      {
        Serial.println();
        Serial.println("Evenimentele disponibile sunt urmatoarele:");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Aveti introduse:");
        lcd.setCursor(0,1);
        lcd.print(" "); 
        lcd.print(nr_events);
        lcd.print(" evenimente! ");  
        delay(5000);    
        for(byte i=0; i<nr_events; i++) 
        {
          char event[100]="";
          for(byte j=0; j<100; j++)
          {
            event[j]=EEPROM.read(10+j+i*100);
          }
          Serial.print(i+1);
          Serial.print(": ");
          Serial.println(event);
          afisare_data(event);
        }
      }
    }
  }
  else ///nu am serialul deschis-adica pot sa le vad cu calculatorul inchis
  {
      current_state3=digitalRead(buttonPin3);
      if(current_state3 == 0)
      {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(" EVENIMENTE AZI");
          lcd.setCursor(0,1);
          lcd.print("DATA: ");
          if(now.day()<=9)
          {
            lcd.print("0");
            lcd.print(now.day(), DEC);
          }
          else
            lcd.print(now.day(), DEC);
            
          lcd.print('/');
          if(now.month()<=9)
          {
            lcd.print("0");
            lcd.print(now.month(), DEC);
          }
          else
            lcd.print(now.month(), DEC);
          lcd.print('/');
          lcd.print(now.year(), DEC);
          delay(4000);
          lcd.clear();
          bool exista_evenimente=0;
          for(byte i=0; i<nr_events; i++) 
          {
              char event[100]="";
              for(byte j=0; j<100; j++)
              {
                event[j]=EEPROM.read(10+j+i*100);
              }
              int zi=(event[1]-'0')*10+event[2]-('0');
              int luna=(event[4]-'0')*10 + (event[5]-'0');
              int an=(event[7]-'0')*1000 + (event[8]-'0')*100 + (event[9]-'0')*10 + (event[10]-'0');
              if(zi == now.day() && luna == now.month() && an == now.year())
              {
                exista_evenimente=1;
                afisare_data(event);
              }               
          }
          if(!exista_evenimente)
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Nu aveti nici un");
            lcd.setCursor(0,1);
            lcd.print("eveniment astazi");
            delay(3000);
            
          }
          lcd.clear();
      }  
  }         
}
///12-13-2009 (11:30) -> TESTESTESTESTESTEST$
void afisare_data(char event[])
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(event[1]);
  lcd.print(event[2]);
  lcd.print("/");
  lcd.print(event[4]);
  lcd.print(event[5]);
  lcd.print("/");
  lcd.print(event[7]);
  lcd.print(event[8]);
  lcd.print(event[9]);
  lcd.print(event[10]);
  lcd.print(" ");
  lcd.print(event[13]); 
  lcd.print(event[14]);
  lcd.print(":");
  lcd.print(event[16]);
  lcd.print(event[17]);
  lcd.setCursor(0,1);
  
  if(strlen(event)-23>16)
  {
      int k=0;
      while(k<=2)
      {     
            for (int letter = 22; letter < strlen(event) - 15; letter++) //From 0 to upto n-16 characters supply to below function
            {
              autoscroll_mesage(0, letter, event, 1);
            }
            k++;     
      }
  }
  else
  {
    char text[17]="";
    int poz=0;
    for(int i=23;i<=38;i++)
        text[poz++]=event[i];
    lcd.print(text);
    delay(4000);
  }
  lcd.clear();
}
void autoscroll_mesage(int printStart, int startLetter, String messagePadded, int level)
{
  lcd.setCursor(printStart, level);
  for (int letter = startLetter; letter <= startLetter + 15; letter++) // Print only 16 chars in Line #2 starting 'startLetter'
  {
    lcd.print(messagePadded[letter]);
  }
  lcd.print(" ");
  delay(450);
  
}
int Format_corect(char event[])
{
  ///mai intai verific spatii, paranteze, sageata si linuta
  ///ZZ-LL-YYYY (HH:MM) -> Text eveniment
  ///1234567890123456789012
  ///         1         2
  if(event[3]!='-')
    return 0;
  if(event[6]!='-')
    return 0;
  if(event[11]!=' ')
    return 0;
  if(event[19]!=' ')
    return 0;
  if(event[22]!=' ')
    return 0;
  if(event[12]!='(')
    return 0;
  if(event[18]!=')')
    return 0;
  if(event[20]!='-')
    return 0;
  if(event[21]!='>')
    return 0;
  return 1;
}
int Data_corect(char event[])
{
   ///verific acum datele
   /// zi intre 0 si 31 luna intre 0 si 12 an sa nu depaseasca 4cifre sau sa aiba 3   
   int zi=(event[1]-'0')*10 + (event[2]-'0');
   int luna=(event[4]-'0')*10 + (event[5]-'0');
   int an=(event[7]-'0')*1000 + (event[8]-'0')*100 + (event[9]-'0')*10 + (event[10]-'0');
   if(zi>31)
     return 0;
   if(luna>12)
     return 0;
   if(an<=1000)
     return 0;
   return 1;
}
int Timp_corect(char event[])
{
    /// ora intre 00 si 24 minut intre 00 si 60
    int ora=(event[13]-'0')*10 + (event[14]-'0');
    int minut=(event[16]-'0')*10 + (event[17]-'0');
    if(ora>24)
       return 0;
    if(minut>60)
       return 0;
    return 1;
}
