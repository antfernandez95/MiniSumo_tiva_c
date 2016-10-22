// Las salidas PWM conectadas a los LED son las siguientes:
//(TIMER0B)T0CCP1  --> PF1 (LED rojo)
//(TIMER1A)T1CCP0  --> PF2 (LED azul)
//(TIMER1B)T1CCP1  --> PF3 (LED verde)
// Esto se puede ver en el Data-Sheet del micro (Tiva TM4C123HG6PM Microcontroller Data Sheet, pag.706-707)

// Hemos hecho una variante que cambio el ciclo de trabajo de un solo LED a lo largo del tiempo, lo cual supone un aumento
// del brillo hasta su valor maximo y una disminucion posterior.

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "drivers/buttons.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h" // Include para poder configurar el pin como salida PWM
#include "inc/hw_gpio.h"

#define PWMCYCLE    320         // Periodo de la onda PWM que hemos definido (el valor maximo posible es 65535)
#define DUTYCYCLE99 PWMCYCLE*0.99 // Diferentes ciclos de trabajo de ejemplo; cuanto mas
#define DUTYCYCLE75 PWMCYCLE*0.75 // cercano a PWMCYCLE, menos tiempo se ilumina el LED
#define DUTYCYCLE25 PWMCYCLE*0.25

double pwm=0.5;

int main(void) {

	// Reloj del sistema a 40MHz (PLL-200MHz/5=40MHz)
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	//Configure PWM Clock to match system
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);  //The Tiva Launchpad has two modules (0 and 1). Module 1 covers the LED pins

	//Configuracion de los pulsadores
	ButtonsInit();
	GPIOIntClear(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0);
	GPIOIntEnable(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0);
	IntEnable(INT_GPIOF);
	GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0,GPIO_FALLING_EDGE);

	// Configuracion de puertos (LEDs)
	// Habilita puerto GPIOF (LEDs)
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Configura pines conectados a LEDs como salidas GPIO
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	// Apaga los LEDs
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);


	//PWM //MIRAR PORQ!!!!!
	GPIOPinConfigure(0x00050805);
	GPIOPinConfigure(0x00050C05);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);

	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

	//Set the Period (expressed in clock ticks)
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 320);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 320);

	//Set PWM duty-50% (Period /2)
	//PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5,100);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,160);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,160);

	// Enable the PWM generator
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

	// Turn on the Output pins
	PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);

	// Codigo principal, en bucle infinito
	while(1) {
	}
}

void RutinaISR(void)
{
	uint8_t ui8Changed,ui8Buttons;
	uint8_t status=ButtonsPoll(&ui8Changed,&ui8Buttons);

	if (ui8Buttons&GPIO_PIN_4)
	{
		if(pwm<=0.95){
			pwm+=0.05;
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,pwm*PWMCYCLE);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,pwm*PWMCYCLE);
		}
	}
	if (ui8Buttons&GPIO_PIN_0){
		if(pwm>=0.05){
			pwm-=0.05;
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6,pwm*PWMCYCLE);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7,pwm*PWMCYCLE);
		}
	}

	GPIOIntClear(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0);

}


