// Projeto 2 - MicroControladores
// Autor: Gustavo Oliveira da Silva
/*
Este projeto com o Arduino informa o valor da temperatura pelo monitor Serial
e por um display LCD. Este valor pode ser apresentado nas escalas, Celsius e Fahrenheit
trocadas pelo botão para mostrar a temperatura nas duas escalas.

O led verde indicará uma temperatura aceitável. O led vermelho indicará atenção pois 
a temperatura do processo está maior ou menor do que a recomendada e caso isto aconteça um
sinal sonoro é ativado.
*/

// Inclusão da biblioteca para uso do LCD e do sensor DHT
#include <DHT.h>
#include <LiquidCrystal.h>

// Define o pino e a tipagem do DHT
#define DHTPIN 10
#define DHTTYPE 22

// Define os valores limiter minimo e maximos aceitaveis como seguros
#define LIMHIGH 60
#define LIMLOW 0

// Pinagem dos leds
#define led_red 8
#define led_green 9

// Pinagem do botão
#define Bottom 3

// Pinagem do Buzzer
#define Buzzer 2

// Setando variaveis para o LCD
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

// Setando variaveis para o DHT
DHT dht(DHTPIN, DHTTYPE);
float oldtemp = 0;
float newtemp = 0;
bool trade = 0;

// Setando variaveis para o buzzer
int hz = 440;

// Ponteiro de função para alternancia da temperatura
void (*fpTemp)(float);

// Ponteiro de função para alternancia dos leds
void (*fpLed)();

// Simbolo para graus
uint8_t degree[8] =
{
  0b00010,
  0b00101,
  0b00010,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// Simbolo para warning parte 1
uint8_t danger1[8] =
{
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00100,
  0b01010,
  0b01010,
  0b10001
};

// Simbolo para warning parte 2
uint8_t danger2[8] =
{
  0b00001,
  0b00001,
  0b00010,
  0b00100,
  0b00100,
  0b01000,
  0b10000,
  0b11111
};

// Simbolo para warning parte 3
uint8_t danger3[8] =
{
  0b00100,
  0b00100,
  0b00100,
  0b00000,
  0b00100,
  0b00000,
  0b00000,
  0b11111
};

// Simbolo para warning parte 4
uint8_t danger4[8] =
{
  0b10000,
  0b10000,
  0b01000,
  0b00100,
  0b00100,
  0b00010,
  0b00001,
  0b11111
};

// Função que printa no monitor serial e no LCD a temperatura em Celsius
void Celsius(float temp)
{
  lcd.setCursor(0,1);
  lcd.print(temp);
  lcd.print("\x03\C");
  Serial.print(temp);
  Serial.println("°C");
}

// Função que printa no monitor serial e no LCD a temperatura em Fahrenheit
void Fahrenheit(float temp)
{
  temp = (((temp*9)/5)+32);
  lcd.setCursor(0,1);
  lcd.print(temp);
  lcd.print("\x03\F");
  Serial.print(temp);
  Serial.println("°F");
}

// Função que ascende o led vermelho e aciona o sinal de warning no LCD
void Red()
{
  digitalWrite(led_red, HIGH);
  digitalWrite(led_green, LOW);
  
  lcd.setCursor(11,0);
  lcd.print("\x04");
  lcd.setCursor(10,1);
  lcd.print("\x05");
  lcd.setCursor(11,1);
  lcd.print("\x06");
  lcd.setCursor(12,1);
  lcd.print("\x07");
}

// Função que ascende o led verde
void Green()
{
  noTone(Buzzer);

  digitalWrite(led_red, LOW);
  digitalWrite(led_green, HIGH);
}

// Interrupção - Troca da grandeza de temperatura
void interrupt()
{
  if(fpTemp == Celsius)
  {
    fpTemp = Fahrenheit;
  }
  else
  {
    fpTemp = Celsius;
  }
  trade = 1;
}

void setup()
{ 
  // Inicia porta serial
  Serial.begin(115200);

  // Inicia o sensor DHT
  dht.begin();

  // Inicia o LCD
  lcd.begin(16,2);

  // Seta os simbolos costumizados
  lcd.createChar(3, degree);
  lcd.createChar(4, danger1);
  lcd.createChar(5, danger2);
  lcd.createChar(6, danger3);
  lcd.createChar(7, danger4);

  // Pinmode dos leds como OUTPUT
  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);

  // Pinmode dos botão como INPUT
  pinMode(Bottom, INPUT);

  // Declaração da Interrupção
  attachInterrupt(digitalPinToInterrupt(Bottom), interrupt, RISING);

  // Preparação das variaveis para começar o loop
  fpTemp = Celsius;
  fpLed = Green;
}

void loop()
{
  // Atualiza a temperatura com o sensor DHT
  newtemp = dht.readTemperature();

  // Confere se não deu erro na leitura da temperatura
  if(!(isnan(newtemp)))
  {
    // Confere se ouve alteração na temperatura
    if((trade) || (newtemp != oldtemp))
    {
      // Limpa o LCD
      lcd.clear();

      // Printa a temperatura
      fpTemp(newtemp);

      // Confere se esta em uma temperatura segura
      if((newtemp >= LIMLOW) && (newtemp <= LIMHIGH))
      {
        // Define como led verde aceso
        fpLed = Green;
      }
      else
      {
        // Define como led vermelho aceso, simbolo de warning no LCD e sinal sonoro de alerta
        fpLed = Red;
      }

      // Ascende o led selecionado
      fpLed();

      // Atualização das variaveis
      oldtemp = newtemp;
      trade = 0;
    }

    // Confere se esta na situação de risco
    if(fpLed == Red)
    {
      // Tocar sirene com o buzzer
      if(hz < 1400)
      {
        tone(Buzzer, hz);
        delay(1);
        hz++;
      }
      else
      {
        hz = 440;
      }
    }
  }
  else
  {
    // Printa no LCD a falha na leitura
    lcd.setCursor(0,0);
    lcd.print("Falha ao ler DHT22");

    // Printa no Serial a falha na leitura
    Serial.println("Falha ao ler DHT22");

    delay(1000);
  }
}