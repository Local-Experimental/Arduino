//#include <stdlib.h>
//#include <stddef.h>
//#include <RotaryEncoder.h>
#include <Arduino.h>
#include "ThermoSencor.h"
#include "ManageReadWrite8bitEEPROM.h"
#include "menuLiquidCrystal.h"
#include "functions.h"


unsigned long clock_01sec = 0;

ThermoSencor Temperature(A3, 8740);

//int8_t FANPIN = RELAY_TRIAC_MotorSpeedControl; // ~  DC FAN 10, 5 for TRIAC AC FAN
int8_t FANPIN = 10; // ~  DC FAN 10, 5 for TRIAC AC FAN
int8_t PELLETPUSHERPIN = 11; // ~
int8_t TEMPSENSORPIN = A3;

int8_t static OneSec = 10; // times in loop until reach one secunde

controls LCDLIGHT(0, 1);//address
						// Celcius
controls TEMPMIN(1, 17); // default 
controls TEMPMAX(2, 29); //address
						 // Fan
controls FANMINSPEED(3, 35); //address
controls FANMAXSPEED(4, 150); //address

controls FANSECONDSHOLD(5, 60); //Hold time after fast shift , that make to burn last cycle dropet pellets
int16_t FANHOLDTIMEOUT_ON = 0;
int16_t FANSPEEDRUN = 0;
bool FANHOLDENABLE_ON = false;
bool FANFIRESTARTTIMERENABLE = true;


int16_t FANFIRESTARTTIME;// = 100 * OneSec; //seconds 600
int16_t FANFIRESTARTTIMEOUT = FANSECONDSHOLD.getValue() * OneSec; //startup first time in rise fan 
float FANSPEEDRUNFLOAT;
float FANSPEEDRUNFLOATSUM;



// Pellets  




controls PELLETsOFTsTart(13, 5); // When pellet motor is started, draw burst amount of amperes. ( (PelletPusherPower*80*  / pelletSoftStart*5* == 16); (for x =0,x< pelletSoftStart*5*,pelletSoftCount+=divided rezult*16*) = time to increase speed by rezult*16*
int8_t pelletSoftCount = PELLETsOFTsTart.getValue(); // set count rutine using pelletSoftStart variable. Divide maximum value from pelletSoftCount as pelletSoftStart bounderies for start up spin.
int pelletSofRresult = 0;

controls PELLETPUSHERMODE(7, 1); // Select Mode 1)Only Timer, 2)By Temp. Regulating Min or Max Power, 3)Power Regulating By Temp. between Min or Max in Percentages Range

controls PELLETPUSHERMILLISECONDSON(8, 10); //address //  push time until sleep time + seconds
controls PELLETPUSHERMINSPEED(9, 35);

//

// Components regulation
int16_t COMPONENTSTIMEOUT_ON = 0; //time counters
int16_t COMPONENTSTIMEOUT_OFF = 0; //time counters

							   // By Decision Stored Values 
int16_t COMPONENTSTDESISION_ON = 0; //time counters
int16_t COMPONENTSTDESISION_OFF = 0; //time counters


controls COMPONENTSMINSECONDS(10, 30); // Responsible for Pellet Pusher and Fan Working Time   
controls COMPONENTSMAXSECONDS(11, 80); // Responsible for Pellet Pusher and Fan Working Time

controls ONLYTIMERFANSPEED(12, 70);

int16_t LowHightProcetange_value = -3;
int16_t procRatio;
int16_t calculatedRatioProc = 0;
int16_t procRatioStaticCalculation = 0;
int16_t COMPONENTSTIMEOUT_OFF_Static = 0;

bool ScreenStatusDisplay = false;

void init_memory_defaults(bool conditiondefault = false) {

	LCDLIGHT.setDataDefault();


	// Temperature 
	TEMPMIN.setDataDefault();
	TEMPMAX.setDataDefault();


	//Fan
	FANSECONDSHOLD.setDataDefault(); // 

	FANMINSPEED.setDataDefault();
	FANMAXSPEED.setDataDefault();



	// Pellet              
	PELLETPUSHERMODE.setDataDefault();
	PELLETPUSHERMILLISECONDSON.setDataDefault();
	PELLETPUSHERMINSPEED.setDataDefault();

	// Control Components Like Fan And Pellet Pusher
	COMPONENTSMINSECONDS.setDataDefault();
	COMPONENTSMAXSECONDS.setDataDefault();

	ONLYTIMERFANSPEED.setDataDefault();
	PELLETsOFTsTart.setDataDefault();

}

//1408

/// Sring

String notSet = "Nera nustatyta";
String isSet = "Nustatyta";
String workingWay = "Veik. Budas";
String RPM = "RPM:";
String Hot = "Karstas";
String Cold = "Saltas";
String Burning = "Degimas";

void dummyFunc() {};

void disablePelletPusher() { // disable when in setings
	analogWrite(PELLETPUSHERPIN, 0); //Disable Pellet Pusher 

	pelletSoftCount = PELLETsOFTsTart.getValue();
	pelletSofRresult = 0;
}

// START menu functions
void funTEMP() {
	disablePelletPusher();
	switch (PELLETPUSHERMODE.getValue())
	{
	case 1:
		//  printMenuFunc("Min temperature ",&TEMPMIN,"C*:"); //
		sey(notSet, "temperaturos");
		break;
	case 2:
		printMenuFunc("Max temperatura ", &TEMPMAX, "C*:"); // 
		sey();
		break;
	case 3:
		//printMenuFunc("Max temperature", &TEMPMAX, "C* ");
		//printMenuFunc("Min temperature ", &TEMPMIN, "C*:", &dummyFunc, false, "Less", TEMPMAX.getValue()-1); // 
		sey("Isjungti nustatymai", "temperaturos");
		break;
	default:
		sey(notSet, workingWay);
		break;
	}


};
//////////////////

void __FANMINSPEEDMIN() { 
	//analogWrite(FANPIN, FANMINSPEED.getValue()); 

	// if normal DC motor then do PWM
	if (!isMotorSpeedEnabled || isMotorSpeedEGrounded)
		analogWrite(FANPIN, FANMINSPEED.getValue());
	else // else do a AC TRiac Single Pulse Control Until Zero Cross cycle ends
		motorPulseWidthTime = map(FANMINSPEED.getValue(), 255, 0, 150, motorMicroSecondsTriggerTimer); //7200 
};
void __FANMINSPEEDMAX() { 
//	analogWrite(FANPIN, FANMAXSPEED.getValue()); 

	// if normal DC motor then do PWM
	if (!isMotorSpeedEnabled || isMotorSpeedEGrounded)
		analogWrite(FANPIN, FANMAXSPEED.getValue());
	else // else do a AC TRiac Single Pulse Control Until Zero Cross cycle ends
		motorPulseWidthTime = map(FANMAXSPEED.getValue(), 255, 0, 150, motorMicroSecondsTriggerTimer); //7200 
};
void __FANONLYTIMER() { 
//	analogWrite(FANPIN, ONLYTIMERFANSPEED.getValue()); 

	// if normal DC motor then do PWM
	if (!isMotorSpeedEnabled || isMotorSpeedEGrounded)
		analogWrite(FANPIN, ONLYTIMERFANSPEED.getValue());
	else // else do a AC TRiac Single Pulse Control Until Zero Cross cycle ends
		motorPulseWidthTime = map(ONLYTIMERFANSPEED.getValue(), 255, 0, 150, motorMicroSecondsTriggerTimer); //7200 
};
void __DUMMYFUNCTION() {  };


void funFAN() {

	disablePelletPusher(); // disable pellet pusher 

	if (PELLETPUSHERMODE.getValue() == 1)
	{
		printMenuFunc("Fano Greitis", &ONLYTIMERFANSPEED, RPM, &__FANONLYTIMER);
	}
	else {



		printMenuFunc("Fanas " + Hot + "", &FANMAXSPEED, "Deg. " + RPM, &__FANMINSPEEDMAX,false,"Less",129);
		printMenuFunc("Fanas " + Cold + "", &FANMINSPEED, "Deg. " + RPM, &__FANMINSPEEDMIN, false, "Less", FANMAXSPEED.getValue() - 1); //

	}
	sey();
};


//////////////////////////////////////////////////
//////////////////////////////////////////////////
void __PELLETPUSH() { analogWrite(PELLETPUSHERPIN, PELLETPUSHERMINSPEED.getValue()); };
void funPelletModeOnlyTimer()
{
	disablePelletPusher(); // disable pellet pusher 

	printMenuFunc("Fano Greitis", &ONLYTIMERFANSPEED, RPM, &__FANONLYTIMER);
	//  printMenuFunc("Gran. Greitis", &PELLETPUSHERMINSPEED, RPM);// DONT TOCHT ONLY IN PROGRAMMER MODE SETTINGS ALLOWED
	//printMenuFunc("Gran.Veiks", &PELLETPUSHERMILLISECONDSON, "mlsec:");
	printMenuFunc("Gran.Neveiks", &COMPONENTSMINSECONDS, "sec:");




	PELLETPUSHERMODE.writeValue(1); // Set to user state

	sey();

};

void funPelletModeTempMinOrMax()
{
	disablePelletPusher(); // disable pellet pusher 


	printMenuFunc("Gran." + Cold, &COMPONENTSMAXSECONDS, Burning + " Sec:"); // Daugiausia
	printMenuFunc("Gran." + Hot, &COMPONENTSMINSECONDS, Burning + " Sec:", &dummyFunc, false, "Less", COMPONENTSMAXSECONDS.getValue() - 1); // 

	printMenuFunc("Fanas " + Hot, &FANMAXSPEED, Burning + " " + RPM, &__FANMINSPEEDMAX);
	printMenuFunc("Fanas " + Cold, &FANMINSPEED, Burning + " " + RPM, &__FANMINSPEEDMIN, false, "Less", FANMAXSPEED.getValue() - 1);


	printMenuFunc("Temperatura", &TEMPMAX, "Maximum C*:");


	PELLETPUSHERMODE.writeValue(2);
	sey("Gran. Min or Max");
};


void __funLCDLIGHT() {    // Sub Function 

	if (LCDLIGHT.getValue() > 0)lcd.setBacklight(HIGH); else lcd.setBacklight(LOW);
};

//
//
//void ProgrammerSettings() {
//       analogWrite(PELLETPUSHERPIN, 0); // disable pellet pusher 
//       
//    if (ArgueAgree("Jautrus paramet?"))
//    {
//      printMenuFunc("Gran. Galia", &PELLETPUSHERMINSPEED, "J:");
////      pelletSoftStart/ PELLETsOFTsTart
/////      printMenuFunc("Gran.Soft", &PELLETsOFTsTart, "start%:");
//      sey ();
//      
//      }else {
//        
//      sey("Nepakeista");
//      }
//  }

void funLCDLIGHT() {

	disablePelletPusher(); // disable pellet pusher 

//	/printMenuFunc("Ekrano Sviesa", &LCDLIGHT);
	printMenuFunc("Ekrano Sviesa", &LCDLIGHT, "P:", __funLCDLIGHT);

	if (ArgueAgree("%Prog%"))
	{
		printMenuFunc("Gran. Galia", &PELLETPUSHERMINSPEED, "J:");
		printMenuFunc("Gran.Veiks", &PELLETPUSHERMILLISECONDSON, "millisec:");
		//      pelletSoftStart/ PELLETsOFTsTart
		printMenuFunc("Gran.Soft.Start", &PELLETsOFTsTart, "%:", &__DUMMYFUNCTION, false, "Less", PELLETPUSHERMILLISECONDSON.getValue());
		printMenuFunc("Fan.Ideg", &FANSECONDSHOLD, "sec:"); // Delay of Keep  Turn On Fan sum While

		if (PELLETsOFTsTart.getValue() <= 1) PELLETsOFTsTart.writeValue(1);// fix user that rare set to zero and in program cannot react that. Give one instead
		sey();

	}
	else
		sey();

};


//
//void funTestingComponents() { // temporery loaded value that not changed can be tested with commands
//	printMenuFunc("Gran. Greitis", &PELLETPUSHERMINSPEED, "TEST-RPM:", __PELLETPUSH, true);
//	//         printMenuFunc("Max. Fan RPM",&FANMINSPEED,"TEST-RPM:",__FANMINSPEED,true);
//}


void funSettoDefault() {
	disablePelletPusher(); // disable pellet pusher 

	if (ArgueAgree())
	{
		lcd.clear();
		lcd.setCursor(0, 1);
		print(isSet);
		lcd.setCursor(0, 0);
		for (int x = 0; x < 16; x++) //animation arrow
		{

			delay(100);
			print(">");
			//                      
		}
		lcd.setCursor(0, 1);
		print(isSet);
		delay(200);
		init_memory_defaults(true);
		delay(2000);
	}
	else
	{
		lcd.clear();
		lcd.setCursor(0, 1);
		delay(300);
		//		lcd.print("Atsaukiama");
		//		delay(800);
		lcd.setCursor(0, 0);
		print("Buvo Atsaukta");
		delay(3400);
	}

};




void funExit() {
	ScreenStatusDisplay = false;
};

// END menu functions///////////////////////////////////////////


menuLiquidCrystal menu[7]; // alway give exact size of menu
menuLiquidCrystalNavigate navmenu;

// load into menu external functions
void initiate_menu_functions() {

	// include menu objects
	menu[0].IncludeFunction(&funTEMP, "Temperatura");
	menu[1].IncludeFunction(&funFAN, "Oro Put. Fenas");
	menu[2].IncludeFunction(&funPelletModeOnlyTimer, workingWay, "Vienodas Laikas");
	menu[3].IncludeFunction(&funPelletModeTempMinOrMax, workingWay, "Pagal Temp.");
	//menu[4].IncludeFunction(&funPelletModeTempBetweenMinMaxProcentage, "3-Veikimo Budas", "Temp.Min % Max");
	menu[4].IncludeFunction(&funLCDLIGHT, "Ekrano Sviesa");
	//menu[6].IncludeFunction(&funTestingComponents, "Testavimas");

///  menu[5].IncludeFunction(&ProgrammerSettings, "Programuotojo", "Nustatymai"); // Not enotuh space

	menu[5].IncludeFunction(&funSettoDefault, "Prad.Nustyti", "Gamik.Parametrai");

	menu[6].IncludeFunction(&funExit, "Iseiti");

	//total menu available
	navmenu.setmenuLenght(sizeof(menu) / sizeof(menu[0])); // find out about size 
}

// when program loaded newly .
void initiate_updatePins(bool print = true) {
	if (LCDLIGHT.getValue() > 0)lcd.setBacklight(HIGH); else lcd.setBacklight(LOW);




	//     if (print) 
	//      {
	//      Serial.println ("FANMINSPEED:" + String (FANMINSPEED.getValue()));
	//      Serial.println ("PELLETPUSHERMINSPEED:" + String (PELLETPUSHERMINSPEED.getValue()));
	//      Serial.println ("TEMPMAX:" + String (TEMPMAX.getValue()));
	//    }
}

// Update Pins 

void initControlPins() {
	//  analogWrite (FANPIN, FANMINSPEED.getValue()); 
	//////////analogWrite (PELLETPUSHERPIN, PELLETPUSHERMINSPEED.getValue()); Negali buti naudojamas be logikos isikisimo
	__funLCDLIGHT(); // Update Background Light

}

//const static  int pusher = 10;
//const static  int wind = 11;

//b1(12) set, b2(13) down; b3(A0) up
//buttons
////////////////////
//Program Variables




void setup() {
	Wire.begin();

	//    encoder.tick();

	delay(500);
	lcd.begin(16, 2);
	delay(1000);
	lcd.begin(16, 2);
	delay(500);





	// put your setup code here, to run once:
	//pinMode(pusher, OUTPUT);
	//pinMode(wind, OUTPUT);
	pinMode(BUTTON_SET, INPUT);
	pinMode(BUTTON_DOWN, INPUT);
	pinMode(BUTTON_UP, INPUT);
	// Motors
	pinMode(FANPIN, OUTPUT);
	pinMode(PELLETPUSHERPIN, OUTPUT);
	// Temp. Sensor
	pinMode(TEMPSENSORPIN, INPUT);

	//   Serial.begin(9600);
	//   Serial.println ("Load Complete: " + String  ("Structs array") + sizeof (menu) + String (",Menu [0] Size") + sizeof (menu[0]));
	delay(500);

	initiate_menu_functions();
	initiate_updatePins();

	if (EEPROM.read(1000) >= 255) { // if first time are loded to chip when set sum defaults 
		init_memory_defaults();
		EEPROM.write(1000, 0);
	}

	attachInterrupt(INTERRUPT_ZeroCrossDetection, MotorSpeedControlFuncZeroCrossDetected, FALLING); // attach interrupt function 
}




int menuSelectedChanged = -1; // for clearing efficiently a screen in menu 
void printmenu() {
	int8_t menuselected = navmenu.getMenuSelected();
	
	lcd.setCursor(0, 0);

	if (menuSelectedChanged != menuselected) {// for clearing efficiently a screen in menu 
		lcd.clear();
		menuSelectedChanged = menuselected;
	}

	print(String(menuselected + 1) + String(")") + menu[menuselected].functionName);
	lcd.setCursor(0, 1);
	if (menu[navmenu.getMenuSelected()].isEmptyFunctionValue()) // ignore unknown value , no dinamic update support :<
		print(menu[navmenu.getMenuSelected()].functionValue);
};


int8_t printstatuscounter = 0;
int8_t printstatustimer = -1;
int c = 0;
float val;
void printstatus(bool print__ = false) {



	//       c--;
	//    if (c <=0) {
	//////        val = analogRead(TEMPSENSORPIN)/25.7;
	//        val=Temperature.RawValueUpdate () /25.7;
	//        delay(20);
	//        c=10;
	//      }
	//lcd.clear();
	lcd.setCursor(0, 0);
	switch (printstatuscounter)
	{
	case 1:
		print("Gran.stumiklis");
		lcd.setCursor(0, 1);

		if (PELLETPUSHERMODE.getValue() == 1) // by Timer
		{
			if (COMPONENTSTIMEOUT_ON != -1)
				print("Clock>on:" + String(COMPONENTSTIMEOUT_ON));
			else
				print("Clock>off:" + String(COMPONENTSTIMEOUT_OFF));
		}
		else if (PELLETPUSHERMODE.getValue() == 2) //Bitween Hight or Low , Controled by Temperature
		{

			if (COMPONENTSTIMEOUT_ON != -1)

				//if (LowHightProcetange_value == -1) // Hight = -1, Low = -2;
				//	lcd.print("Ijungtas:" + String(COMPONENTSTIMEOUT_ON));
			//	else
				print("Ijungtas:" + String(COMPONENTSTIMEOUT_ON));
			else

				//if (LowHightProcetange_value == -1) // Hight = -1, Low = -2;
				//	lcd.print("Isjungtas:" + String(COMPONENTSTIMEOUT_OFF));
				//else
				print("Isjungtas:" + String(COMPONENTSTIMEOUT_OFF));

		}

		else
		{
			print("Isjungta.");
		}




		break;
	case 2:
		print("Oro Put. Fenas");////////////////////////////////////////////
		lcd.setCursor(0, 1);
		switch (PELLETPUSHERMODE.getValue())
		{
		case 1:
			print("Laikmatis " + RPM + String(FANSPEEDRUN));

			break;
		case 2:


			if (FANFIRESTARTTIMEOUT > 0) // fix showing rise when temperature is to low  //& Temperature.temperature > TEMPMAX.getValue()
				print(RPM + String(FANSPEEDRUN) + " s:" + String(FANFIRESTARTTIMEOUT));
			else if (FANSPEEDRUN == FANMAXSPEED.getValue())
				print("max " + RPM + String(FANSPEEDRUN));
			else
				print("min " + RPM + String(FANSPEEDRUN));




			break;
			//case 3:
			//	int valProc = (FANSPEEDRUN * 100) / FANMAXSPEED.getValue();
			//	//                              100 - ((COMPONENTSTIMEOUT_OFF_Static * 10 ) / COMPONENTSMAXSECONDS.getValue())
			//	//                             lcd.print(String( calculatedRatioProc )+"%RPM:"+ String (FANSPEEDRUN));
			//	//                           lcd.print(String( valProc )+"%"+String(calculatedRatioProc)+"%?RPM:"+ String (FANSPEEDRUN));
			//	lcd.print(String(valProc) + "%RPM:" + String(FANSPEEDRUN));

			//	break;
		}


		if (FANHOLDTIMEOUT_ON > 0)
			print(",H:" + String(FANHOLDTIMEOUT_ON));
		break;
	case 3:
		print("Temp. C*:" + String(Temperature.temperature));
		lcd.setCursor(0, 1);
		switch (PELLETPUSHERMODE.getValue())
		{
		case 1:
			print("---");
			break;
		case 2:
			print("C* max:" + String(TEMPMAX.getValue()));    // Temperature.Temperature
			break;
			//case 3:
			//	lcd.print("C* min:" + String(TEMPMIN.getValue()) + " max:" + String(TEMPMAX.getValue()));    // Temperature.Temperature
			//	break;
		}

		break;
	case 4:
		//
		//                   1-Veikimo Budas","Pagal Laika");
		//   menu[3].IncludeFunction(&funPelletModeTempMinOrMax,"2-Veikimo Budas","Temp.Min - Max");
		//   menu[4].IncludeFunction(&funPelletModeTempBetweenMinMaxProcentage,"3-Veikimo Budas","Temp.Min % Max") ;

		//                      lcd.print("WTFACK");
		//                      lcd.setCursor(0,1);
		//                      lcd.print("WTFACKdick");





		switch (PELLETPUSHERMODE.getValue())
		{
		case 1:
			print("1-" + workingWay);
			lcd.setCursor(0, 1);
			print("Pagal Laika");
			break;

		case 2:
			print("2-" + workingWay);
			lcd.setCursor(0, 1);
			print("Pagal Temp.");
			break;

		default:
			print(workingWay);
			print("Nust.Nepavyko");
			break;
		}
		break;
	case 5:
		switch (PELLETPUSHERMODE.getValue())
		{
		case 1:
			print("Busena:" + String(pelletSofRresult) + "," + String(pelletSoftCount));
			lcd.setCursor(0, 1);
			print("Pagal Laika");
			break;

		case 2:
			print("Busena:" + String(pelletSofRresult) + "," + String(pelletSoftCount));
			lcd.setCursor(0, 1);
			if (LowHightProcetange_value == -1) // Hight = -1, Low = -2;
				print(Hot + " Deg.");
			else
				print(Cold + " Deg.");
			break;
		default:
			print(workingWay);
			print("Nust.Nepavyko");
			break;
		}
		break;

	}




	if (printstatuscounter > 5) //how much status blocks in switch case 
		printstatuscounter = 1;

	// Timer on
	if (printstatustimer == -1 || print__) {
		printstatustimer = 70; // 7 sec
		printstatuscounter++;
		lcd.clear();
	}

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int exitmenutime = 30 * OneSec;//ms
int exitmenutimer = exitmenutime;
int c1Sec = 0;
void loop() {

	

	//  c1Sec--;
	//  if (c1Sec <= 0 )//delay shift between temp function and just time hold function
	//  { 
	//    c1Sec = 10; 
	//     Temperature.RawValueUpdate (); // Measure Temperature 100 ms Hold
	//  }else
	//  {
	//    delay (100);
	//  }

	if (((long)clock_01sec + 100UL) < millis()) // timer delay 
	{

		clock_01sec = millis(); // reset each  1 seconds  time




	// if (!rotaryEnable && !isMotorSpeedEnabled) delay(45);
		Temperature.InCustomTimeAverageUpdate(600);
		// if (!rotaryEnable && !isMotorSpeedEnabled) delay(45);

		int8_t __set;// = digitalRead(BUTTON_SET);
		int8_t __up; // = digitalRead(BUTTON_UP);
		int8_t __down; // = digitalRead(BUTTON_DOWN);

		if (rotaryEnable)
		{
			int side;// = encoder.getPositionSideTimeout(80); //timeout
			__up = 0; // reset
			__down = 0; // reset

			if (side == 1)
				__up = 1; // set condition
			else if (side == -1)
				__down = 1; // set condition
		}
		else
		{

			// if no rotory encoder but only push buttons
			__up = digitalRead(BUTTON_UP);
			__down = digitalRead(BUTTON_DOWN);

		}

		__set = digitalRead(BUTTON_SET);



		initControlPins(); // update output pins


						   //delay (90);
		if (COMPONENTSTIMEOUT_ON > -1)COMPONENTSTIMEOUT_ON--;
		if (COMPONENTSTIMEOUT_OFF > -1)COMPONENTSTIMEOUT_OFF--;
		if (FANHOLDTIMEOUT_ON > -1) FANHOLDTIMEOUT_ON--;

		if (printstatustimer > -1) printstatustimer--;
		if (FANFIRESTARTTIMEOUT > -1) { FANFIRESTARTTIMEOUT--; };






		//     Serial.println("__set:"+ String(__set) + ",__up:"+ String(__up) + ",__down:" + String(__down)  );



		//PELLETPUSHER


		switch (PELLETPUSHERMODE.getValue())
		{


		case 1:



			COMPONENTSTDESISION_OFF = COMPONENTSMINSECONDS.getValue() * OneSec; // min = 60 seconds  + custom seconds           
			FANSPEEDRUN = ONLYTIMERFANSPEED.getValue(); // default
			break;//////////////////////////////////////////////////////////////////////////////////////////////////////
		case 2: // Temp.Low-Hight" // use temperature to shift between low or hight power condition

			if (Temperature.temperature < TEMPMAX.getValue()) {
				COMPONENTSTDESISION_OFF = COMPONENTSMINSECONDS.getValue() * OneSec; // if not worm enough when reduce Time



				if (FANHOLDENABLE_ON == false) // if fan was started from LOW mode then try increase speed very slowly
				{

					//FANFIRESTARTTIMERENABLE = false;
					FANFIRESTARTTIMEOUT = FANSECONDSHOLD.getValue() * OneSec; //FANFIRESTARTTIME; // react if burn cycle is coplete

					FANSPEEDRUNFLOATSUM = float(FANMAXSPEED.getValue() - FANMINSPEED.getValue()) / FANFIRESTARTTIMEOUT;

					FANSPEEDRUNFLOAT = FANMINSPEED.getValue(); // set to minimum , later increases fan speed 
				}

				if (FANFIRESTARTTIMEOUT > 0)// if true to slow start fan
				{
					FANSPEEDRUNFLOAT = FANSPEEDRUNFLOAT + FANSPEEDRUNFLOATSUM; // Minimum Speed plus each bit divided from given time 
					FANSPEEDRUN = FANSPEEDRUNFLOAT;
				}
				else
				{
					FANSPEEDRUN = FANMAXSPEED.getValue(); // if time out slow ignite mode then use this 
					//FANFIRESTARTTIMERENABLE = false;

				}

				FANHOLDENABLE_ON = true;
				LowHightProcetange_value = -1; // Hight                    

			}
			else {
				COMPONENTSTDESISION_OFF = COMPONENTSMAXSECONDS.getValue() * OneSec; // if  enough warm when Increase Time

				if (FANHOLDENABLE_ON) { // turn on fan max speed 
					FANHOLDTIMEOUT_ON = FANSECONDSHOLD.getValue() * OneSec;
					FANHOLDENABLE_ON = false;
					// set value FANFIRESTARTTIMEOUT to kick fan each time 
				}
				LowHightProcetange_value = -2; // Low show condition                   
				FANSPEEDRUN = FANMINSPEED.getValue();
			}

			break;//////////////////////////////////////////////////////////////////////////////////////////////////////
		//case 3: //Temp.Low%Hight // control in low hight range with sum of procenatge from min max temperature range


			////if (Temperature.Temperature <= TEMPMIN.getValue()) // if current temp. is less then minimum  temp then set everything to 100% Heat power
			//	//procRatio = 100;

			////else //Temperature is more then minimum temp , can reduse power in procentage
			//	procRatio = (100 * ((Temperature.Temperature - TEMPMIN.getValue()) / (TEMPMAX.getValue() - TEMPMIN.getValue())));


			//if (procRatio > 100) procRatio = 100; //Protection from wtf logic failure 
			//
			//if (Temperature.Temperature < TEMPMIN.getValue())
			//{
			//	procRatio = 100;
			//	//calculatedRatioProc = 0;
			//}
			//else if (Temperature.Temperature > TEMPMAX.getValue())
			//{
			//	procRatio = 0;
			//	//calculatedRatioProc = 100;
			//}



			//

			//if (COMPONENTSTDESISION_OFF < (COMPONENTSMINSECONDS.getValue() * OneSec))  COMPONENTSTDESISION_OFF = COMPONENTSMINSECONDS.getValue() * OneSec; // Set to minimum
			//if (COMPONENTSTIMEOUT_OFF_Static == 0) { // later use to show  procentage  // also set to default first time in the program
			//	COMPONENTSTIMEOUT_OFF_Static = COMPONENTSTDESISION_OFF;
			//	
			//	calculatedRatioProc = 100 - ( float(COMPONENTSTIMEOUT_OFF_Static * OneSec) / COMPONENTSMAXSECONDS.getValue());

			//	FANSPEEDRUN = (FANMAXSPEED.getValue()* procRatio) / 100;
			//	if (FANSPEEDRUN < FANMINSPEED.getValue()) // fan speed from procentage is lower then minimum posible value , do set minimum value 
			//		FANSPEEDRUN = FANMINSPEED.getValue();

			//}


			//COMPONENTSTDESISION_OFF = (float(COMPONENTSMAXSECONDS.getValue()* OneSec) / 100) * calculatedRatioProc; //set to dinamic procentage of power to work


			//break;/////////////////////////////////////////////////////////////////////////////////////////////////////
		default:
			break;
		}

		COMPONENTSTDESISION_ON = PELLETPUSHERMILLISECONDSON.getValue(); // min = 60 seconds + custom seconds 



	//OFF																	// Fan And Pellet Time Counter
		if (COMPONENTSTIMEOUT_ON == -1 && COMPONENTSTIMEOUT_OFF == -1) // before ON_TIMEOUT become -1 , zero give window to step up a turn of mode enable
		{

			switch (PELLETPUSHERMODE.getValue())
			{
			case 1:

				break;
			case 2:

				//FANFIRESTARTTIMERENABLE = true;
				break;
				//case 3:
				//	COMPONENTSTIMEOUT_OFF_Static = COMPONENTSTDESISION_OFF;

				//	calculatedRatioProc = 100 - ((COMPONENTSTIMEOUT_OFF_Static * OneSec) / COMPONENTSMAXSECONDS.getValue());

				//	FANSPEEDRUN = (FANMAXSPEED.getValue()* procRatio) / 100;

				//	if (FANSPEEDRUN < FANMINSPEED.getValue()) // fan speed from procentage is lower then minimum posible value , do set minimum value 
				//		FANSPEEDRUN = FANMINSPEED.getValue();
				//	break;
			default:
				break;
			}




			COMPONENTSTIMEOUT_OFF = COMPONENTSTDESISION_OFF; // final setup Main timer 
		}






		/////////////////////// ON 
		if (COMPONENTSTIMEOUT_ON == -1 && COMPONENTSTIMEOUT_OFF == 0) // give beggining and turn on pellet pusher
			COMPONENTSTIMEOUT_ON = COMPONENTSTDESISION_ON;

		// Pellet

		if (COMPONENTSTIMEOUT_ON > -1) { // execute rutine   
									   //ON

	//		    / if ( (pelletSoftStart * 10 /*seconds*/ < /* still less*/ PELLETPUSHERMINSPEED.getValue()) && pelletSoftStart * 10  <  pelletSoftCount)









			if (pelletSoftCount >= 1) {
				pelletSoftCount -= 1;// Count down each time where was completed pellet this rutine unti 1

					 //                            Cast a result  
				pelletSofRresult += (PELLETPUSHERMINSPEED.getValue() / PELLETsOFTsTart.getValue()); //   pelletSoftCount never les then 1 to divide correctly

				if (pelletSofRresult > PELLETPUSHERMINSPEED.getValue()) // if rezult become greater then expeted then give original value from the settings
					pelletSofRresult = PELLETPUSHERMINSPEED.getValue();

			}
			else {
				pelletSofRresult = PELLETPUSHERMINSPEED.getValue();  // if rezult less then expected   
			}

			analogWrite(PELLETPUSHERPIN, pelletSofRresult); //Give incresing speed by divided packets  

	   //     /analogWrite(PELLETPUSHERPIN, PELLETPUSHERMINSPEED.getValue()); //Give speed/power to motor Save
		}
		else // OFF
		{
			analogWrite(PELLETPUSHERPIN, 0); //Give speed/power to motor  
			pelletSoftCount = PELLETsOFTsTart.getValue();
			pelletSofRresult = 0;
		}
		// Fan Delay Timer


		if (FANHOLDTIMEOUT_ON > 0) {

			// if normal DC motor then do PWM
			if (!isMotorSpeedEnabled || isMotorSpeedEGrounded)
				analogWrite(FANPIN, FANMAXSPEED.getValue());
			else // else do a AC TRiac Single Pulse Control Until Zero Cross cycle ends
				motorPulseWidthTime = map(FANMAXSPEED.getValue(), 255, 0, 150, motorMicroSecondsTriggerTimer); //7200 
		}
		else {
			// if normal DC motor then do PWM
			if (!isMotorSpeedEnabled || isMotorSpeedEGrounded)
				analogWrite(FANPIN, FANSPEEDRUN);
			else // else do a AC TRiac Single Pulse Control Until Zero Cross cycle ends
				motorPulseWidthTime = map(FANSPEEDRUN, 255, 0, 150, motorMicroSecondsTriggerTimer); //7200

		}

		// Automatiskai iseinti pagal laika is option menu

		// to Those What to print 
		if (ScreenStatusDisplay) {
			printmenu();

			// exit timer 
			if (exitmenutimer > 0)
				exitmenutimer--;
			else
				funExit(); // automaticly exit after sum time from manual seletion menu

		}
		else {
			printstatus();
		}




		if (__up) {//  Serial.print ( "getMenuSelected:" + String (navmenu.getMenuSelected ()) + ",getMenuLenght:" + String (navmenu.getMenuLenght()) );
				   //        FANMINSPEED.addValue();
			navmenu.menuUp();
			printmenu();
			// Wait button realess
			buttonRelease(BUTTON_UP);
			exitmenutimer = exitmenutime; // update each time auto menu exit timer after pressed a button

			ScreenStatusDisplay = true;
		}



		if (__down) {
			//        FANMINSPEED.subValue();
			navmenu.menuDown();
			printmenu();
			// Wait button realess
			buttonRelease(BUTTON_DOWN);
			exitmenutimer = exitmenutime; // update each time auto menu exit timer after pressed a button

			ScreenStatusDisplay = true;
		}


		if (__set && ScreenStatusDisplay) {




			lcd.blink();
			menu[navmenu.getMenuSelected()].DrawFunction();
			lcd.noBlink();


			//                       if (  menu[navmenu.getMenuSelected()].isEmptyFunctionValue() ) // ignore unknown value
			//                          menu[navmenu.getMenuSelected()].functionValue  = 


			if (!ScreenStatusDisplay) {
				navmenu.menuReset(); // When user exit from menu , give ScreenStatusDisplay = false
			}
			//                      Serial.println ("After ScreenStatusDisplay:"+String (ScreenStatusDisplay)  );

			// fast swith status output

		}
		if (__set && !ScreenStatusDisplay)
		{
			printstatus(true);
		}



	}

}