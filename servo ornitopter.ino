#include "debug.h"

// Parámetros para PWM
#define PWM_PERIOD 1999  // Valor del período para 167 Hz
#define PWM_PRESCALER 71  // Prescaler para obtener 1 kHz de frecuencia base
#define PULSE_MIN 80   // Valor de pulso para 0 grados
#define PULSE_MAX 950  // Valor de pulso para 190 grados

// Función para inicializar PWM
void TIM1_PWMOut_Init(u16 arr, u16 psc, u16 ccp, u8 channel)
{
    GPIO_PinRemapConfig(AFIO_PCFR1_PA12_REMAP, DISABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    if (channel == 2) {
        // Inicializar PA2 para TIM1 CH2N
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        // Inicializar TIM1 CH2N
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; // Polaridad complementaria
        TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; // Habilitar salida complementaria
        TIM_OCInitStructure.TIM_Pulse = ccp;
        TIM_OC2Init(TIM1, &TIM_OCInitStructure);
        TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    } else if (channel == 4) {
        // Inicializar PC4 para TIM1 CH4
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        // Inicializar TIM1 CH4
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        TIM_OCInitStructure.TIM_Pulse = ccp;
        TIM_OC4Init(TIM1, &TIM_OCInitStructure);
        TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    }

    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

// Función para convertir ángulo a pulso basado en los valores medidos
u16 angleToPulse(int angle) {
    return PULSE_MIN + ((angle * (PULSE_MAX - PULSE_MIN)) / 190);
}

int main(void)
{
    SystemCoreClockUpdate();
    Delay_Init();

    // Parámetros configurables
    int min_angle = 60;    // Ángulo mínimo deseado
    int max_angle = 140;  // Ángulo máximo deseado
    int speed = 1.5;       // Velocidad de barrido (ms de retardo)

    // Inicializar PWM para ambos canales
    TIM1_PWMOut_Init(PWM_PERIOD, PWM_PRESCALER, angleToPulse(min_angle), 4);  // Para TIM1 CH4 en PC4
    TIM1_PWMOut_Init(PWM_PERIOD, PWM_PRESCALER, angleToPulse(min_angle), 2);  // Para TIM1 CH2N en PA2

    while (1)
    {
        // Barrido de ángulo mínimo a máximo
        for (int angle = min_angle; angle <= max_angle; angle++)
        {
            u16 pulse_width = angleToPulse(angle);
            TIM_SetCompare4(TIM1, pulse_width);  // Actualizar TIM1 CH4
            TIM_SetCompare2(TIM1, pulse_width);  // Actualizar TIM1 CH2N
            Delay_Ms(speed);  // Ajustar la velocidad de barrido
        }

        // Barrido de ángulo máximo a mínimo
        for (int angle = max_angle; angle >= min_angle; angle--)
        {
            u16 pulse_width = angleToPulse(angle);
            TIM_SetCompare4(TIM1, pulse_width);  // Actualizar TIM1 CH4
            TIM_SetCompare2(TIM1, pulse_width);  // Actualizar TIM1 CH2N
            Delay_Ms(speed);  // Ajustar la velocidad de barrido
        }
    }
}
