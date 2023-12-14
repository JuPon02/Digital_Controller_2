// Ajout suivi GIT
// INCLUDE
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>	// Manipulation de cha�nes de caract�res
#include <stdlib.h> // pour utiliser la fonction itoa()

#include "Main.h"
#include "OS.h"
#include "Hardware.h"
#include "USART.h"
#include "lcd.h"
#include "ADC.h"
#include "ftoa.h"
#include "PWM.h"
#include "I2C_Master.h"
#include "DS3232.h"

// Mes variables globales
unsigned char IDCB_Led = 0;			// Identificateur callback timer pour le clignotement de la LED

//****************** fonction principale *****************
int main (void)
{
 	//Initialisation hardware 
	Init_Hardware();
	//Initilisation LCD
	lcd_init(LCD_DISP_ON); lcd_clrscr(); 
	
	
	//Initialisation ADC
	ADC_init();
	
	// Initialisation PWM	
	// PWM_1_A_B_init(0x5,255);	// prescaler = 0b001 ; TOP = 4095; 16 bits
	// PWM_2_B_init(0x7,255); // TOP = 255; 8 bits
	
	// Initialisation des Callbacks
	OS_Init();
 	IDCB_Led = Callbacks_Record_Timer(Switch_LED, 500);
	
	Callbacks_Record_Usart0_RX(USART0_RX);

 	// Lancement OS (Boucle infinie)
	OS_Start();
	//N'arrive jamais ici
	return 0; 
}

//**************** Switch LED ************************************
//  Signalisation par clignotement de la LED  pendant 500 ms 
//****************************************************************
void Switch_LED(void)
{
	TOGGLE_IO(PORTD,PORTD7);
}


//****************USART0_RX*************************
// Appel� quand on a re�u un message du terminal PC
//**************************************************
void USART0_RX(volatile char *Trame_USART0)
{
        
}
		
	
	//*****************************************
	//           STATE MACHINE
	//*****************************************
	
	
	char Date(char input)
	{
		// Arnaud et Julien
		static unsigned char First_in_Function = TRUE;
		static int Date_Jour;static int Date_Moi;static int Date_Annee;
		static char temp;
		int IndJ;  int IndMoi; int IndAn;
		char String_Jour[4];char String_Moi[4];char String_Annee[4];
		
		if (First_in_Function){
			// Initialization of date data.
			Date_Jour = RTC_Clock_Read_Byte(DATE);if (Date_Jour<10){IndJ = 1;}else{IndJ = 0;}
			Date_Moi = RTC_Clock_Read_Byte(MOIS);if (Date_Moi<10){IndMoi = 4;}else{IndMoi = 3;}
			Date_Annee = RTC_Clock_Read_Byte(ANNEE);if (Date_Annee<10){IndAn = 7;}else{IndAn = 6;}
			temp = 0;
			
			// Process to display on the LCD.
			itoa(Date_Jour, String_Jour, 10);itoa(Date_Moi, String_Moi, 10);itoa(Date_Annee, String_Annee, 10);
			
			// Display on the LCD.
			cli();lcd_gotoxy(0,1);lcd_puts("  -  -          ");sei();
			cli();lcd_gotoxy(IndJ,1);lcd_puts(String_Jour);sei();
			cli();lcd_gotoxy(IndMoi,1);lcd_puts(String_Moi);sei();
			cli();lcd_gotoxy(IndAn,1);lcd_puts(String_Annee);sei();
			//cli();lcd_gotoxy(1,1);lcd_command(LCD_ON_CURSOR);lcd_command(LCD_ON_BLINK);sei();
			First_in_Function = FALSE;
		}
		else
		{
			if (input != ENTER)
			{
				// If left or right: choose the character to set.
				switch(input){
					case LEFT :
					temp--;
					if (temp<0){temp=0;}
					break;
					case RIGHT:
					temp++;
					if (temp>2){temp=2;}
					break;
				}
				// Increment and decrement values based on the character to set.
				switch(temp){
					case 0 :
					Date_Jour = EDIT_VALUE(Date_Jour, input,1,31);
					if (Date_Jour<10){IndJ = 1;cli();lcd_gotoxy(0,1);lcd_puts(0);sei();}else{IndJ = 0;}
					itoa(Date_Jour, String_Jour, 10);
					cli();lcd_gotoxy(IndJ,1);lcd_puts(String_Jour);sei();
					break;
					case 1 :
					Date_Moi = EDIT_VALUE(Date_Moi, input,1,12);
					if (Date_Moi<10){IndMoi = 4;cli();lcd_gotoxy(3,1);lcd_puts(0);sei();}else{IndMoi = 3;}
					itoa(Date_Moi, String_Moi, 10);
					cli();lcd_gotoxy(IndMoi,1);lcd_puts(String_Moi);sei();
					break;
					case 2 :
					Date_Annee = EDIT_VALUE(Date_Annee, input,0,99);
					if (Date_Annee<10){IndAn = 7;cli();lcd_gotoxy(6,1);lcd_puts(0);sei();}else{IndAn = 6;}
					itoa(Date_Annee, String_Annee, 10);
					cli();lcd_gotoxy(IndAn,1);lcd_puts(String_Annee);sei();
					break;
				}
				
			}
			else
			{
				RTC_Clock_Set_Date(Date_Jour,Date_Moi,Date_Annee); // Set the date
				First_in_Function = TRUE;
				return ST_TXT_TIME;
			}
		}
		return ST_FCT_DATE;
	}
	
	char Time(char input)
	{
		static unsigned char First_in_Function = TRUE;
		static int Sec;static int Min;static int Heure;
		static char temp;
		int Inds;  int Indmin; int Indh;
		char String_s[4];char String_min[4];char String_Heure[4];
		
		if (First_in_Function){
			// Initialization of time data.
			Sec = RTC_Clock_Read_Byte(SECONDE);if (Sec<10){Inds = 1;}else{Inds = 0;}
			Min = RTC_Clock_Read_Byte(MINUTE);if (Min<10){Indmin = 4;}else{Indmin = 3;}
			Heure = RTC_Clock_Read_Byte(HEURE);if (Heure<10){Indh = 7;}else{Indh = 6;}
			temp = 0;
			// Process to display on the LCD.
			itoa(Sec, String_s, 10);itoa(Min, String_min, 10);itoa(Heure, String_Heure, 10);
			// Display on the LCD.
			cli();lcd_gotoxy(0,1);lcd_puts("  h  m  s        ");sei();
			cli();lcd_gotoxy(Inds,1);lcd_puts(String_Heure);sei();
			cli();lcd_gotoxy(Indmin,1);lcd_puts(String_min);sei();
			cli();lcd_gotoxy(Indh,1);lcd_puts(String_s);sei();
			First_in_Function = FALSE;
		}
		else
		{
			if (input != ENTER)
			{
				// If left or right: choose the character to set.
				switch(input){
					case LEFT :
					temp--;
					if (temp<0){temp=0;}
					break;
					case RIGHT:
					temp++;
					if (temp>2){temp=2;}
					break;	
				}
				// Increment and decrement values based on the character to set.
				switch(temp){
					case 2 :
					Sec= EDIT_VALUE(Sec, input,0,59);
					if (Sec<10){Inds = 7;cli();lcd_gotoxy(6,1);lcd_puts(0);sei();}else{Inds = 6;}
					itoa(Sec, String_s, 10);
					cli();lcd_gotoxy(Inds,1);lcd_puts(String_s);sei();
					break;
					case 1 :
					Min = EDIT_VALUE(Min, input,0,59);
					if (Min<10){Indmin = 4;cli();lcd_gotoxy(3,1);lcd_puts(0);sei();}else{Indmin = 3;}
					itoa(Min, String_min, 10);
					cli();lcd_gotoxy(Indmin,1);lcd_puts(String_min);sei();
					break;
					case 0 :
					Heure = EDIT_VALUE(Heure, input,0,99);
					if (Heure<10){Indh = 1;cli();lcd_gotoxy(0,1);lcd_puts(0);sei();}else{Indh = 0;}
					itoa(Heure, String_Heure, 10);
					cli();lcd_gotoxy(Indh,1);lcd_puts(String_Heure);sei();
					break;
				}
				
			}
			else
			{
				RTC_Clock_Set_Heure(Sec,Min,Heure);// Set the hour
				First_in_Function = TRUE;
				return ST_TXT_SENSOR;
			}
		}
		return ST_FCT_TIME;
	}
	
	char Adc(char input)
	{
		// Florimond + Daniel + Pierre + R�mi  
		return ST_TXT_ACTUATOR;
	}
	
	char Current(char input)
	{
		
		// Antoine + Math�o - Travail ADC
		return ST_TXT_ACTUATOR;
	}
	
	char Thermocouple(char input)
	{
		
		// Nobody
		return ST_TXT_ACTUATOR;
	}
	
	char Dac(char input)
	{
		// Pierre + R�mi 
		return ST_TXT_STORAGE;
	}
	
	char Relay(char input)
	{
		// Florimond + Daniel + Pierre + R�mi 
		return ST_TXT_STORAGE;
	}
	
	char Pwm_1(char input)
	{
		static unsigned char First_in_Function = TRUE;
		
		if (First_in_Function){
			cli();lcd_gotoxy(0,1);lcd_puts("1 PWM selectionne");sei();
			First_in_Function = FALSE;
			PWM_init(1);
			setDutyCycle_1A(127);
			setDutyCycle_1B(0);
			setDutyCycle_2B(0);
		}
		else{
			if (input == ENTER){
				First_in_Function = TRUE;
				return ST_TXT_PWM_SETUP;
			}
		}
		return ST_FCT_PWM_1;
	}
	
	char Pwm_1_2(char input)
	{
		static unsigned char First_in_Function = TRUE;
		
		if (First_in_Function){
			cli();lcd_gotoxy(0,1);lcd_puts("2 PWM selectionnes");sei();
			First_in_Function = FALSE;
			PWM_init(2);
			setDutyCycle_1A(127);
			setDutyCycle_1B(127);
			setDutyCycle_2B(0);
		}
		else{
			if (input == ENTER){
				First_in_Function = TRUE;
				return ST_TXT_PWM_SETUP;	
			}
		}
		return ST_FCT_PWM_1_2;
	}
	
	char Pwm_1_2_3(char input)
	{
		static unsigned char First_in_Function = TRUE;
		
		if (First_in_Function){
			cli();lcd_gotoxy(0,1);lcd_puts("3 PWM selectionnes");sei();
			First_in_Function = FALSE;
			PWM_init(3);
			setDutyCycle_1A(127);
			setDutyCycle_1B(127);
			setDutyCycle_2B(127);
		}
		else{
			if (input == ENTER){
				First_in_Function = TRUE;
				return ST_TXT_PWM_SETUP;
			}
		}
	return ST_FCT_PWM_1_2_3;
	}
	
	char Periode(char input)
	{
		static int PeriodePWM = 50;
		char String[4];
		static unsigned char First_in_Function = TRUE;
		
		if (First_in_Function){
				cli();lcd_gotoxy(0,1);lcd_puts("Frequence :50");sei();
				First_in_Function = FALSE;
		}
		else{
			if (input != ENTER){
				switch(input){
					case DOWN :
					PeriodePWM -= 10;
					if (PeriodePWM<0){PeriodePWM=0;}
					itoa(PeriodePWM, String, 10);
					cli();lcd_gotoxy(11,1);lcd_puts("  ");lcd_gotoxy(11,1);lcd_puts(String);sei();
					break;
					case UP:
					PeriodePWM +=10;
					if (PeriodePWM>256){PeriodePWM=255;}
					itoa(PeriodePWM, String, 10);
					cli();lcd_gotoxy(11,1);lcd_puts("  ");lcd_gotoxy(11,1);lcd_puts(String);sei();
					break;
				}
				
			}
			else
			{
				PWM_1_A_B_init(0B001,PeriodePWM);	// prescaler = 0b001 ; TOP = 4095; 16 bits
				PWM_2_B_init(0b001,PeriodePWM);
				setDutyCycle_1A(PeriodePWM/2);
				setDutyCycle_1B(PeriodePWM/2);
				setDutyCycle_2B(PeriodePWM/2);
				First_in_Function = TRUE;
				return ST_TXT_STORAGE;
			}	
		}
		return ST_FCT_PERIODE;
	}
	
	char Resolution(char input)
	{
		// Arnaud + Julien + Emre + Charles
		//return ST_FCT_RESOLUTION;
		static unsigned char First_in_Function = TRUE;
		if (First_in_Function){
			First_in_Function = FALSE;
		}
		else{
			if (input != ENTER){
			}
			else{
				First_in_Function = TRUE;
			return ST_TXT_PWM_SETUP;
			}
		}
		return ST_FCT_RESOLUTION;
	}
	
	char Dent_Scie(char input)
	{
		// Arnaud + Julien + Emre + Charles
		static unsigned char First_in_Function = TRUE;
		if (First_in_Function){
			First_in_Function = FALSE;
		}
		else{
			if (input != ENTER){
			}
			else{
				First_in_Function = TRUE;
				return ST_TXT_PWM_SETUP;
			}
		}
		return ST_FCT_DENT_SCIE;
	}
	
	
	char Isocele(char input)
	{
		// Arnaud + Julien + Emre + Charles
		static unsigned char First_in_Function = TRUE;
		if (First_in_Function){
			First_in_Function = FALSE;
		}
		else{
			if (input != ENTER){
			}
			else{
				First_in_Function = TRUE;
				return ST_TXT_PWM_SETUP;
			}
		}
		return ST_FCT_ISOCELE;
	}
	
	
	char Eeprom(char input)
	{
		// Florian + Hughes
		return ST_TXT_MONITORING;
	}
	
	char Sram(char input)
	{
		// Emre + Charles
		return ST_TXT_MONITORING;
	}
	
	char Monitoring(char input)
	{
		// Antoine + Math�o
		return ST_TXT_CONTRAST;
	}
	
	char Contrast(char input)
	{
		// Nobody
		return ST_TXT_DEBUG;
	}
	
	char Debug(char input)
	{
		// Antoine + Math�o
		return ST_TXT_SETUP;
	}
	
	char Sampling_Time(char input)
	{
		// Antoine + Math�o + Florian + Hughes
		//return ST_FCT_SAMPLING_TIME;
		return ST_TXT_ON_OFF;
	}
	
	char Duration(char input)
	{
		// Antoine + Math�o + Florian + Hughes
		//return ST_FCT_DURATION;
		return ST_TXT_ON_OFF;
	}
	
	char Datalogger_On_Off(char input)
	{
		// Antoine + Math�o + Florian + Hughes
		return ST_TXT_DATA_TRANSFER;
	}
	
	char Transfer_Usart(char input)
	{
		// Antoine + Math�o + Florian + Hughes
		//return ST_FCT_DATA_TRANSFER;
		return ST_TXT_DATALOGGER;
	}
	
	char Sampling_CTRL(char input)
	{
		// Florimond + Daniel + Pierre + R�mi 
		//return ST_FCT_SAMPLING_CTRL;
		return ST_TXT_MANUAL_MODE;
	}

	char Manual_Mode(char input)
	{
		// Florimond + Daniel + Pierre + R�mi 
		//return ST_FCT_MANUAL_MODE;
		return ST_TXT_AUTO_MODE;
	}
	
	char Auto_Mode(char input)
	{
		// Florimond + Daniel + Pierre + R�mi 
		//return ST_FCT_AUTO_MODE;
		return ST_TXT_MANUAL_MODE;
	}
	
	char Pid_P(char input)
	{
		// Nobody
		//return ST_FCT_P;
		return ST_TXT_I;
	}
	
	char Pid_I(char input)
	{
		// Nobody
		//return ST_FCT_I;
		return ST_TXT_D;
	}
	
	
	char Pid_D(char input)
	{
		// Nobody
		//return ST_FCT_D;
		return ST_TXT_WINDUP;
	}
	
	
	char Pid_WindUp(char input)
	{
		// Nobody
		//return ST_FCT_WINDUP;
		return ST_TXT_SAMPLING_CTRL;
	}
	
	char Recursion(char input)
	{
		// Florimond + Daniel + Pierre + R�mi
		// Download fichier CSV commun !
		return ST_TXT_SAMPLING_CTRL;
	}
	
	char Smith(char input)
	{
		// Nobody
		// Download fichier CSV commun !
		return ST_TXT_SAMPLING_CTRL;
	}
	
	char Internal_Model(char input)
	{
		// Nobody
		// Download fichier CSV commun !
		return ST_TXT_MODEL_SETTING;
	}
	
	char Internal_Model_Setting(char input)
	{
		// Nobody
		// Download fichier CSV commun !
		//return ST_FCT_MODEL_SETTING;
		return ST_TXT_SAMPLING_CTRL;
	}
	
	char Rst(char input)
	{
		// Nobody
		// Download fichier CSV commun !
		return ST_TXT_SAMPLING_CTRL;
	}
	
	char Pwm_Manual(char input)
	{
		// Arnaud + Julien + Emre + Charles
		//return ST_FCT_MANUAL_MODE_PWM;
		return ST_TXT_PWM;
	}


	char Pwm_Sinus(char input)
	{
		// Arnaud + Julien + Emre + Charles
		//return ST_FCT_MODE_SINUS;
		static unsigned char First_in_Function = TRUE;
		if (First_in_Function){
			First_in_Function = FALSE;
		}
		else{
			if (input != ENTER){
			}
			else{
				First_in_Function = TRUE;
				return ST_TXT_PWM;
			}
		}
		return ST_FCT_MODE_SINUS;
	}

	char Sinus_Amplitude(char input)
	{
		static unsigned char First_in_Function = TRUE;
		if (First_in_Function){
			First_in_Function = FALSE;
		}
		else{
			if (input != ENTER){
			}
			else{
				First_in_Function = TRUE;
				return ST_TXT_ON_OFF_SINUS;
			}
		}
		return ST_FCT_SINUS_AMPLITUDE;	
	}
	char Sinus_Periode(char input)
	{
		// Arnaud + Julien + Emre + Charles
		//return ST_FCT_SINUS_PERIODE;static unsigned char First_in_Function = TRUE;
		static unsigned char First_in_Function = TRUE;
		if (First_in_Function){
			First_in_Function = FALSE;
		}
		else{
			if (input != ENTER){
			}
			else{
				First_in_Function = TRUE;
				return ST_TXT_ON_OFF_SINUS;
			}
		}
		return ST_FCT_SINUS_PERIODE;
	}
	
	char Sinus_On_Off(char input)
	{
		// Arnaud + Julien + Emre + Charles
		//return ST_FCT_ON_OFF_SINUS;
		static unsigned char First_in_Function = TRUE;
		if (First_in_Function){
			First_in_Function = FALSE;
		}
		else{
			if (input != ENTER){
			}
			else{
				First_in_Function = TRUE;
				return ST_TXT_SINUS;
			}
		}
		return ST_FCT_ON_OFF_SINUS;
	}
	
	
	
	int EDIT_VALUE_0_99(int Initial_Value, char input)
	{
		char String[4];
		switch (input)
		{
			case DOWN :
			{
				Initial_Value--;
				if (Initial_Value == -1) Initial_Value = 99;
				itoa(Initial_Value, String, 10);
				//cli();lcd_gotoxy(0,1);lcd_puts("                ");lcd_gotoxy(0,1);lcd_puts(String);sei();
				break;
			}
			case UP :
			{
				Initial_Value++;
				if (Initial_Value == 100) Initial_Value = 0;
				itoa(Initial_Value, String, 10);
				//cli();lcd_gotoxy(0,1);lcd_puts("                ");lcd_gotoxy(0,1);lcd_puts(String);sei();
				break;
			}
			case LEFT :
			{
				Initial_Value = Initial_Value - 10;
				if (Initial_Value <0) Initial_Value = 0;
				itoa(Initial_Value, String, 10);
				//cli();lcd_gotoxy(0,1);lcd_puts("                ");lcd_gotoxy(0,1);lcd_puts(String);sei();
				break;
			}
			case RIGHT :
			{
				Initial_Value = Initial_Value + 10;
				if (Initial_Value > 99) Initial_Value = 99;
				itoa(Initial_Value, String, 10);
				//cli();lcd_gotoxy(0,1);lcd_puts("                ");lcd_gotoxy(0,1);lcd_puts(String);sei();
				break;
			}
		}
		return Initial_Value;
	}
	
// This function is used to set the time and date of the DS3232.
//  It allows staying between two defined values and processing 
//  them based on the button pressed.
	int EDIT_VALUE(int Initial_Value, char input, int Value_Min, int Value_Max)
	{
		//char String[4];
		switch (input)
		{
			case DOWN :
			{
				Initial_Value--;
				if (Initial_Value == Value_Min-1) Initial_Value = Value_Max;
				break;
			}
			case UP :
			{
				Initial_Value++;
				if (Initial_Value == Value_Max+1) Initial_Value = Value_Min;
				break;
			}
		}
		return Initial_Value;
	}