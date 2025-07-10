#include <Bluepad32.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// ================= Display OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ícones de Bluetooth (16x16) invertidos
const uint8_t bt_connected_16x16[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0xc0, 0x09, 0x60, 0x05, 0x20, 0x03, 0x40, 0x11, 0x88, 
	0x11, 0x88, 0x03, 0x40, 0x05, 0x20, 0x09, 0x60, 0x01, 0xc0, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

const uint8_t bt_disconnected_16x16[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x31, 0xc0, 0x19, 0x60, 0x0d, 0x20, 0x04, 0x40, 0x02, 0x00, 
	0x01, 0x00, 0x03, 0x80, 0x05, 0x40, 0x09, 0x60, 0x01, 0xd0, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00
};

// Motores
#define IN1 14
#define IN2 27
#define ENA 32
#define IN3 26
#define IN4 25
#define ENB 33

const int freq = 5000;
const int pwmChannelA = 0;
const int pwmChannelB = 1;
const int resolution = 8;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// Botões
#define BOTAO_RE_RAPIDA     0x0080
#define BOTAO_RE_MEDIA      0x0010
#define BOTAO_FRENTE_MEDIA  0x0020
#define BOTAO_FRENTE_RAPIDA 0x0040
#define BOTAO_DRIBLE_DIR    0x0002
#define BOTAO_DRIBLE_ESQ    0x0004

// MPU6050
Adafruit_MPU6050 mpu;
float anguloZ = 0;
unsigned long ultimoTempoGiro = 0;

// Drible
bool executandoDrible = false;
int direcaoDrible = 0;

void desenhaBluetooth(bool conectado) {
  display.clearDisplay();
  display.drawBitmap(2, 0, conectado ? bt_connected_16x16 : bt_disconnected_16x16, 16, 16, WHITE);
  display.display();
}

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      Serial.printf("🎮 Controle conectado! Index=%d\n", i);
      desenhaBluetooth(true);
      return;
    }
  }
  Serial.println("⚠️ Controle conectado, mas não há slots livres.");
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      Serial.printf("🚫 Controle desconectado! Index=%d\n", i);
      desenhaBluetooth(false);
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  ledcSetup(pwmChannelA, freq, resolution);
  ledcAttachPin(ENA, pwmChannelA);
  ledcSetup(pwmChannelB, freq, resolution);
  ledcAttachPin(ENB, pwmChannelB);

  Wire.begin();
  if (!mpu.begin()) {
    Serial.println("❌ MPU6050 não encontrado!");
    while (1);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("❌ Falha ao iniciar a tela OLED"));
    while (1);
  }
  display.clearDisplay();
  display.display();

  desenhaBluetooth(false);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  BP32.setup(&onConnectedController, &onDisconnectedController);
  Serial.println("🚀 Bluetooth pronto, conecte seu controle!");
}

void loop() {
  BP32.update();

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    ControllerPtr ctl = myControllers[i];
    if (ctl && ctl->isConnected()) {
      uint16_t botoes = ctl->buttons();

      if (executandoDrible) {
        if (botoes & (BOTAO_RE_RAPIDA | BOTAO_RE_MEDIA | BOTAO_FRENTE_MEDIA | BOTAO_FRENTE_RAPIDA)) {
          executandoDrible = false;
          parar();
          Serial.println("⛔️ Drible cancelado por ação manual.");
        } else {
          sensors_event_t a, g, temp;
          mpu.getEvent(&a, &g, &temp);

          unsigned long tempoAtual = millis();
          float deltaT = (tempoAtual - ultimoTempoGiro) / 1000.0;
          ultimoTempoGiro = tempoAtual;

          float velAng = g.gyro.z * 57.2958;
          anguloZ += velAng * deltaT;

          float anguloAlvo = 90.0 * direcaoDrible;
          float erro = anguloAlvo - anguloZ;

          display.setCursor(0, 20);
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.fillRect(0, 20, SCREEN_WIDTH, 10, BLACK);
          display.printf("Angulo: %.1f\xB0", anguloZ);
          display.display();

          if (abs(erro) < 1.0) {
            parar();
            executandoDrible = false;
            Serial.println("✅ Giro finalizado!");
          } else {
            int pwm = constrain(abs(erro) * 2, 60, 255);
            pwm = erro > 0 ?  pwm : -pwm;

            if (direcaoDrible > 0)
              moverDiferencial(-pwm, 0);
            else
              moverDiferencial(0, -pwm);
          }
        }
        return;
      }

      if (botoes & BOTAO_DRIBLE_DIR) {
        iniciarDrible(1);
        return;
      }
      if (botoes & BOTAO_DRIBLE_ESQ) {
        iniciarDrible(-1);
        return;
      }

      int eixoX = ctl->axisX();
      int curva = map(eixoX, -512, 512, 100, -100);

      bool reRapida     = botoes & BOTAO_RE_RAPIDA;
      bool reMedia      = botoes & BOTAO_RE_MEDIA;
      bool frenteMedia  = botoes & BOTAO_FRENTE_MEDIA;
      bool frenteRapida = botoes & BOTAO_FRENTE_RAPIDA;

      if (frenteRapida) mover(255, curva);
      else if (frenteMedia) mover(-180, curva);
      else if (reRapida) mover(-255, curva);
      else if (reMedia) mover(180, curva);
      else parar();
    }
  }

  delay(10);
}

void iniciarDrible(int direcao) {
  executandoDrible = true;
  direcaoDrible = direcao;
  anguloZ = 0;
  ultimoTempoGiro = millis();

  Serial.printf("↪️ Iniciando curva pivot de 90° para %s\n", direcao > 0 ? "DIREITA" : "ESQUERDA");
}

void mover(int velocidade, int curva) {
  int velA = velocidade, velB = velocidade;

  if (curva > 0) velA = velocidade * (100 - curva) / 100;
  else if (curva < 0) velB = velocidade * (100 + curva) / 100;

  controlaMotorA(velA);
  controlaMotorB(velB);
  Serial.printf("PWM A: %d | PWM B: %d\n", abs(velA), abs(velB));
}

void moverDiferencial(int velocidadeA, int velocidadeB) {
  controlaMotorA(velocidadeA);
  controlaMotorB(velocidadeB);
  Serial.printf("🪯 Pivot - A: %d | B: %d\n", velocidadeA, velocidadeB);
}

void controlaMotorA(int velocidade) {
  if (velocidade > 0) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    ledcWrite(pwmChannelA, velocidade);
  } else if (velocidade < 0) {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    ledcWrite(pwmChannelA, abs(velocidade));
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    ledcWrite(pwmChannelA, 0);
  }
}

void controlaMotorB(int velocidade) {
  if (velocidade > 0) {
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    ledcWrite(pwmChannelB, velocidade);
  } else if (velocidade < 0) {
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    ledcWrite(pwmChannelB, abs(velocidade));
  } else {
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    ledcWrite(pwmChannelB, 0);
  }
}

void parar() {
  controlaMotorA(0);
  controlaMotorB(0);
}
