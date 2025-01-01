/**
 * @file    7segment.c
 * @author  HS6502
 * @version 1.1
 * @date    30-Dec-2024
 * @brief   7segment Display Direct Driving Library for STM8
 */

/* Space (Empty) Character */
#define SPC 255

uint8_t Number_to_Digit[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

void Refresh_Delay(void){
	uint16_t t=7500;
	while(t){
		t--;
	}
}

uint8_t Char_to_Digit (uint8_t Number){
    uint8_t Hex_Value = 0x00;
    switch(Number){
		case '-' : Hex_Value = 0x40; break;
		case 'E' : Hex_Value = 0x79; break;
		case 'H' : Hex_Value = 0x76; break;
		case 'L' : Hex_Value = 0x38; break;
		case 'm' : Hex_Value = 0x79; break;
		case 'u' : Hex_Value = 0x1C; break;
		default : Hex_Value = 0x00; break;
    }
    return Hex_Value;
}

void Display_Write (uint8_t Numbers[], uint8_t Dot_Num){
	uint8_t i,Digit=0,Segment,Data[4];

				/*	Anodes :  	Anode1, Anode2, Anode3, Anode4*/
	GPIO_TypeDef* Dig_Port[] = {GPIOD, GPIOD,  GPIOD,  GPIOA};
	uint8_t Dig_Pin[] 	=	   {  3,	 4,	     5,      3};

				/* Segments : 	 Dot	  G		  F		  E		 D	    C		B	  A */
	GPIO_TypeDef* Seg_Port[] = {GPIOC,	GPIOC,	GPIOD,	GPIOB, GPIOB, GPIOC, GPIOD, GPIOD	};
	uint8_t Seg_Pin[] 	= 	   { 3	,	  7,	  2,	  5,	 4,	    4,  	6,    1  };

	for(i=0; i<4; i++){
		if(Numbers[i] > 10){			/* if Character */
			Data[i] = Char_to_Digit(Numbers[i]);
		}
		else{							/* if Number */
			Data[i] = Number_to_Digit[Numbers[i]];
		}
	}

	for(Digit=0; Digit < 4; Digit++){
		for(i=0; i<4; i++){
			Dig_Port[i]->ODR &= ~(1 << Dig_Pin[i]);		/* Turn OFF Last Digit */
		}

		Data[Digit] <<= 1;									/* Ignore DotPoint bit */
		
		for(Segment=1; Segment<8; Segment++){

			if(Data[Digit] & 0x80){	
				Seg_Port[Segment]->ODR &= ~(1 << Seg_Pin[Segment]);	/* Segment ON 	*/		 
			}
			else{
				Seg_Port[Segment]->ODR |= (1 << Seg_Pin[Segment]);	/* Segment OFF */
			}
			Data[Digit] <<= 1;
		}

		if((Dot_Num - 1) == Digit){
			Seg_Port[0]->ODR &= ~(1 << Seg_Pin[0]);			/* Turn ON DotPoint on Selected Digit */
		}
		else{
			Seg_Port[0]->ODR |= (1 << Seg_Pin[0]);			/* DotPoint OFF */
		}

		Dig_Port[Digit]->ODR |= (1 << Dig_Pin[Digit]);		/* Turn ON Current Digit */
		Refresh_Delay();
	}
}

void Boot_Logo (void){
	uint8_t i,d, n=0, dot=5;
	uint8_t Temp[4] = {SPC,SPC,SPC,SPC};	/* SPC Means Empty Character */
	uint8_t Arr[] = {SPC,SPC,SPC,'E','-',6,5,0,2,SPC,SPC,SPC,SPC,SPC};
	for(d=0; d<11; d++){
		for(i=0; i<20; i++){
			Display_Write(Temp,dot);        
		}
    Temp[0] = Arr[n];
    Temp[1] = Arr[n+1];
    Temp[2] = Arr[n+2];
    Temp[3] = Arr[n+3];
    n++;
    dot--;			
	}
}