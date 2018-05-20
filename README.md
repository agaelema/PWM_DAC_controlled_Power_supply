
# PWM DAC controlled Power Supply

PWM DAC controlled Power supply using LM317.

-------------------------------------------------

Projeto desenvolvido para o workshop ["Do mundo real ao digital: condicionamento de sinais"](https://pt.scribd.com/document/378584270/Sieel-2018-Do-Mundo-Real-Ao-Digital-Condicionamento-de-Sinais), realizado durante a SIEEL 2018 - 08/05/2018.

Arduino Uno/Nano controlando a saída do regulador LM317/117. PWM DAC gerando referência de tensão e controle em malha fechada simples (proporcional) para controle da tensão.

Seleção da tensão a partir do terminal serial.

Arduino Pins:
 - A0 -> Divisor de tensão saída Regulador
 - D6 -> Oscilador para tensão negativa
 - D9 -> PWM 10 bit para referência

Esquemático
![Esquemático](https://github.com/agaelema/PWM_DAC_controlled_Power_supply/blob/master/Schematic.png?raw=true)

Fonte em funcionamento
![Fonte em funcionamento](https://github.com/agaelema/PWM_DAC_controlled_Power_supply/blob/master/PWM_DAC_scope_01.png?raw=true)

Resposta ao degrau
![Resposta ao degrau](https://github.com/agaelema/PWM_DAC_controlled_Power_supply/blob/master/PWM_DAC_scope_02.png?raw=true)