/**
 * @file	 main.c
 * @author	 HS6502
 * @date 	 21-Jan-2025		// File Create Date : 15-July-2024 
 * @version  V1.2
 * @brief    Cheap and High Quality Digital Inductance Meter with STM8S003F3 Microcontroller
 */	

#include "stm8s.h"
#include "7segment.c"

#define STM8S003F3				/* Comment this Line for Using STM8S003K3 */

#undef  EEPROM									/* unDefine the EEPROM word in File stm8s.h */
#define EEPROM(ByteAddress) 	 (*(volatile uint8_t*)(ByteAddress))
#define CAP0    0.0000000047f    				/* Default C3 Capacitor Value : 4.7nF */ 
/*======= OPTION BYTES =======*/
volatile uint8_t *AFR  = (uint8_t *) 0x4803;	/* Alternate Function0 Config bit */
volatile uint8_t *NAFR = (uint8_t *) 0x4804;
/*============================*/

volatile uint8_t Dot_Number, Symbol;
volatile uint8_t Buffer[4] = {0,0,0,0};
volatile uint8_t Overflow_Count, SampleNum = 3, Repeat = 0;
uint16_t d;
volatile uint16_t Pulse_Count;
volatile uint32_t EEP_Temp;
volatile float Default_Freq = 123456;			/* Default Frequency when Lx Probe is Shorted */
volatile float Frequency;
volatile float L0 ;						/* on Board L1 Value */
volatile float Lx ;

void GPIO_Config (void){
	GPIOA->DDR = 0x08;				/* PortA Pin3 as Output */
	GPIOB->DDR = 0x30;				/* PortB Pin4 and 5 as Output */
	GPIOC->DDR = 0x9D;				/* PortC Pin1,5,6 as Input, other Pins Output */
	GPIOD->DDR = 0xFF;  			/* PortD Pin1 to 6 as Output*/
	GPIOA->CR1 = 0x08;				/* PortA Pin3 as PushPull */
	GPIOB->CR1 = 0x30;				/* PortB Pin4 and 5 as PushPull*/
	GPIOC->CR1 = 0xFF;				/* PortC Pin1,5,6 Pullup Enable, Output Pins PushPull */
	GPIOD->CR1 = 0xFF;				/* PortD All Pins as PushPull */
	EXTI->CR1  = 0x20;				/* PortC Interrupts only Falling Edge */
	GPIOC->CR2 = 0x20;				/* PortC Pin 5 (Button Pin) Interrupt Enable */	
}

void Clock_Begin (void){
	CLK->CKDIVR = 0x00;				/* Run CPU and Peripherals at 16MHz */
	CLK->ECKR = 0x01;				/* Enable External Crystal Clock Source */
	CLK->SWR = 0xB4;				/* Select External Crystal at Master Clock */
	for(d=0; d<15000; d++);			/* Delay needed for Stablization External Crystal */
	if(CLK->SWCR & 0x08){			/* if External Crystal Available */
		CLK->SWCR &= 0xF7;			/* Clear SWIF Flag */			
		CLK->SWCR |= 0x02;			/* Switch Master Clock to Crystal */
	}
	else{							/* if External Clock UnAvailable, Switch Back to HSI */
		CLK->SWR = 0xE1;
		CLK->ECKR = 0x00;
	}
	
}

void EEPROM_Load_Saved (void){
	EEP_Temp  =   EEPROM (0x4003);
	EEP_Temp |= (uint32_t)(EEPROM (0x4002)) << 8;
	EEP_Temp |= (uint32_t)(EEPROM (0x4001)) << 16;

	Default_Freq = EEP_Temp;
	L0 = (float) 1 / ((39.4784f * CAP0) * (Default_Freq * Default_Freq) );
}

#if		defined(STM8S003F3)
/* ReMap the Timer1 Channel 1 to Port C6 */
void Alternate_Function_1 (void){
	if((*AFR & 0x01) == 0){
		FLASH->CR2 = 0x80;			// Enable Write Access to Option Bits 
		FLASH->NCR2 = 0x7F;			// Enable Write Access to Option Bits 
		*AFR = 0x01;				// Write 1 to AFR0 Bit 
		*NAFR = 0xFE;				// Write 0 to NAFR0 Bit 
	}
}
#endif

void Timer1_PulseCounter_Begin (void){
	TIM1->IER = 0x01;				/* Enable Update (Overflow) Interrupt */	
	TIM1->SR1 = 0x00;				/* Clear All TIM1 Interrput Flags */
	TIM1->CNTRH = 0x00;				/* Reset Counter MSB to Zero */
	TIM1->CNTRL = 0x00;				/* Reset Counter LSB to Zero */
	TIM1->CCMR1 = 0x01;				/* Select TIM1 Channel 1 as Input */
	TIM1->CCER1 = 0x03;				/* Select Falling Edge Detection Mode and Enable Capture */	
	TIM1->SMCR = 0x57;				/* TIM1 Channel 1 as Trigger and Select External Clock Mode */
}

void Timer2_Delay_Begin (void){
	TIM2->IER  = 0x01;				/* Enable Update (Overflow) Interrupt */
	TIM2->ARRH = 0x51;				/* Set AutoReload Value MSB */
	TIM2->ARRL = 0x61;				/* Set AutoReload Value LSB */
	TIM2->PSCR = 0x08;				/* Set Prescaler Value to 256 */
	TIM2->EGR  = 0x01;				/* Generate Update Event for Take Prescaler Value */
}

void Get_Frequency (void){
	TIM1->CR1 = 0x00;						/* Disable Timer1 Counter */
	Pulse_Count = TIM1->CNTRL;				/* Read LSB of Counter */
	Pulse_Count |= TIM1->CNTRH << 8;		/* Read MSB of Counter */
	Frequency = Pulse_Count + (Overflow_Count * 65536);		/* Measure Frequency */
	Frequency *= SampleNum;					/* Multiply in Sample Number */
	Overflow_Count = 0;
	TIM1->CNTRH = 0x00;
	TIM1->CNTRL = 0x00;	
	TIM1->CR1 = 0x01;						/* Enable Timer1 Counter */
}

void Measure_Inductance (void){	
	Lx = ( ((Default_Freq * Default_Freq) / (Frequency * Frequency)) -1.0f) * L0;
	if(Lx >= 0.5f && Lx < 6000.0f){
		if(Repeat == 3){	// f*=3
			SampleNum = 1;
			TIM2->ARRH = 0xF4;
			TIM2->ARRL = 0x24;
		}
		Repeat++;
	}
	else{
		if(TIM2->ARRL != 0x61){
			SampleNum = 3;
			TIM2->ARRH = 0x51;
			TIM2->ARRL = 0x61;
			Repeat = 0;
		}
	}						//lx here
}

/* Convert Float Value to Integer for Display to 7segment */
void Update_Values (float Value){
	
    if(Value <= 0.0f){
		Value = 0.0f;
        Dot_Number = 1;
		Symbol = 'u';
	}
	else if((Value > 0.0f ) && (Value < 0.00001f)){
		Value *= 100000000;
        Dot_Number = 1;
		Symbol = 'u';
	}
    else if(Value >= 0.00001f && Value < 0.0001f){
        Value *= 10000000;
        Dot_Number = 2;
		Symbol = 'u';
    }
	else if(Value >= 0.0001f && Value < 0.001f){
		Value *= 1000000;
        Dot_Number = 0;
		Symbol = 'u';
	}
	else if(Value >= 0.001f && Value < 0.01f){
		Value *= 100000;
        Dot_Number = 1;
		Symbol = 'm';
	}
	else if(Value >= 0.01f && Value < 0.1f){
		Value *= 10000;
        Dot_Number = 2;
		Symbol = 'm';
	}
	else if(Value >= 0.1f && Value < 1.0f){
		Value *= 1000;
        Dot_Number = 0;
		Symbol = 'm';
	}
    else if(Value >= 1.0f && Value < 10.0f){
        Value *= 100;
        Dot_Number = 1;
		Symbol = 'H';
    }
    else if(Value >= 10.0f && Value < 100.0f){
        Value *= 10;
        Dot_Number = 2;
		Symbol = 'H';
    }
    else if(Value >= 100.0f && Value < 1000.0f){
        Dot_Number = 0;
		Symbol = 'H';
    }
	if(Value >= 1000.0f){				/* Display "OL" on 7segment */ 
		Dot_Number = 0;
		Buffer[0] = SPC;
		Buffer[1] = 0;
		Buffer[2] = 'L';
		Buffer[3] = SPC;
	}
	else{
		Buffer[0] = (uint16_t) Value / 100;
		Buffer[1] = (uint16_t)(Value / 10) % 10; 
		Buffer[2] = (uint16_t) Value % 10;
		Buffer[3] = Symbol;
	}
}

void TIM1_Overflow (void)__interrupt(11){
	Overflow_Count ++ ;					/* Increase Number of Overflow Count */ 
	TIM1->SR1 = 0x00;					/* Clear Update Flag */ 
}

void TIM2_Overflow (void)__interrupt(13){
	Get_Frequency();
	Measure_Inductance();
	TIM2->SR1 = 0x00;				/* Clear Update Flag */
}

void EEPROM_Save (uint32_t S_Value){
	FLASH->DUKR = 0xAE;				/* EEPROM Unlock Key 2 */ 
	FLASH->DUKR = 0x56;				/* EEPROM Unlock Key 1 */ 

	EEPROM (0x4003) =  S_Value & 0xFF;
	EEPROM (0x4002) = (S_Value >> 8) & 0xFF;
	EEPROM (0x4001) = (S_Value >> 16) & 0xFF;

	FLASH->IAPSR = 0x00;			/* Clear Bit3 for Write Protect EEPROM */ 
} 

/* PORTC PIN 5 Interrupt Vector , for Button Press Detection */
void BUTTON_Press(void)__interrupt(5){
	TIM1->CR1 = 0x00;
	Default_Freq = Frequency;
	L0 = (float) 1 / ((39.4784f * CAP0) * (Frequency * Frequency) );
	EEPROM_Save(Default_Freq);
	TIM1->CR1 = 0x01;
}

void main (void){
	uint8_t i;
	disableInterrupts();
	GPIO_Config();
	Clock_Begin();
	Boot_Logo();
	EEPROM_Load_Saved();
#if	defined(STM8S003F3)
	Alternate_Function_1();
#endif
	Timer1_PulseCounter_Begin();
	Timer2_Delay_Begin();
	enableInterrupts();	
	TIM2->CR1 = 0x01;				/* Enable Timer2 Counter */ 
	while(1){
		Update_Values(Lx);
		for(i=0; i<7; i++){
			Display_Write(Buffer,Dot_Number);
		}
	}
}				/* End of File */