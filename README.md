# 🤖 Robô de Futebol com Controle Bluetooth, MPU6050 e Display OLED

Projeto de robô controlado por gamepad Bluetooth com giroscópio (MPU6050) para dribles automáticos de 90° e feedback visual em display OLED. Utiliza motores DC com ponte H (L298N) e microcontrolador ESP32.

## 📦 Funcionalidades

- Controle remoto via controle Bluetooth compatível
- Execução de dribles de 90° (esquerda/direita) com correção automática
- Cancelamento de drible por comando do usuário
- Visualização do status Bluetooth e do ângulo no display OLED
- Controle de motores com ajuste de velocidade e curvas

## 🔧 Componentes Utilizados

| Componente             | Descrição                                                                 |
|------------------------|---------------------------------------------------------------------------|
| ESP32                  | Microcontrolador principal, Wi-Fi/Bluetooth integrado                    |
| MPU6050                | Sensor de movimento (acelerômetro + giroscópio) via I2C                 |
| L298N                  | Ponte H dual para controle de dois motores DC                            |
| Motores DC x2          | Motores de 12V para movimentação                                          |
| Display OLED SSD1306   | Tela I2C de 0.96" 128x64 px para exibir ícones e dados                  |
| Controle Bluetooth     | Gamepad compatível com Bluepad32 (ex: 8BitDo, PS4, etc.)                |
| Fonte 12V / Bateria LiPo | Alimentação geral do robô                                               |

### 🛠 Ligações dos Componentes

#### Display OLED (SSD1306 I2C)
| OLED     | ESP32        |
|----------|--------------|
| GND      | GND          |
| VCC      | 3.3V         |
| SCL      | GPIO 22      |
| SDA      | GPIO 21      |

#### MPU6050 (I2C)
Mesmo barramento do display OLED:
| MPU6050  | ESP32        |
|----------|--------------|
| GND      | GND          |
| VCC      | 3.3V         |
| SCL      | GPIO 22      |
| SDA      | GPIO 21      |

#### Ponte H L298N
| L298N       | ESP32     | Função                      |
|-------------|-----------|-----------------------------|
| IN1         | GPIO 14   | Motor A direção             |
| IN2         | GPIO 27   | Motor A direção             |
| ENA         | GPIO 32   | PWM Motor A                 |
| IN3         | GPIO 26   | Motor B direção             |
| IN4         | GPIO 25   | Motor B direção             |
| ENB         | GPIO 33   | PWM Motor B                 |
| VCC         | 12V       | Alimentação dos motores     |
| GND         | GND       | Referência                  |
| 5V (opcional)| Não usar com ESP32 | Pode ser removido         |

#### Alimentação
- Fonte 12V conectada ao L298N
- ESP32 pode ser alimentado via conversor buck 5V/3.3V conectado à mesma fonte (ou USB durante testes)

## 📚 Bibliotecas Utilizadas

- [`Bluepad32`](https://github.com/ricardoquesada/bluepad32) - Comunicação com gamepad via Bluetooth
- [`Adafruit_MPU6050`](https://github.com/adafruit/Adafruit_MPU6050) - Comunicação com giroscópio
- [`Adafruit_GFX`](https://github.com/adafruit/Adafruit-GFX-Library) - Biblioteca gráfica para display
- [`Adafruit_SSD1306`](https://github.com/adafruit/Adafruit_SSD1306) - Driver para display OLED

## 🎮 Mapeamento dos Botões

| Botão no Controle      | Ação Executada                        |
|------------------------|----------------------------------------|
| Frente Rápida          | Avança com potência máxima             |
| Frente Média           | Avança com potência média              |
| Ré Rápida              | Recuo com potência máxima              |
| Ré Média               | Recuo com potência média               |
| Drible Direita         | Gira 90° no sentido horário            |
| Drible Esquerda        | Gira 90° no sentido anti-horário       |

## 🧠 Lógica do Drible

1. O botão de drible ativa o modo de giro automático.
2. O MPU6050 lê a velocidade angular e integra o ângulo.
3. A velocidade de rotação é ajustada proporcionalmente ao erro (ângulo restante).
4. Quando se aproxima de 90°, a velocidade diminui para uma parada suave.
5. Se ultrapassar, o motor gira no sentido inverso para correção.
6. O usuário pode cancelar a qualquer momento pressionando outro botão.

## 🚀 Como Usar

1. Conecte todos os componentes conforme descrito.
2. Faça o upload do firmware no ESP32 via Arduino IDE ou PlatformIO.
3. Ligue a alimentação (12V) para o robô.
4. Emparelhe o controle Bluetooth com o ESP32.
5. Use os botões para movimentar e acionar os dribles.

## 📷 Imagens e Vídeos

> _Adicione aqui fotos do circuito e vídeos de demonstração._

## 📜 Licença

Este projeto está sob licença MIT. Você pode usá-lo, modificá-lo e distribuí-lo livremente.
