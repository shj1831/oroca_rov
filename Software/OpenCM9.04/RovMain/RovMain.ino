//----------------------------------------------------------------------------
//    프로그램명 	: RovMain
//
//    만든이     	:  
//
//    날  짜     : 
//    
//    최종 수정  	: 
//
//    MPU_Type	: 
//
//    파일명     	: RovMain.ino
//----------------------------------------------------------------------------
#include "RSP.h"
#include <Servo.h> 
 

#define USE_RC_SETUP	0


#define RC_MOTOR_L		0
#define RC_MOTOR_C		1
#define RC_MOTOR_R		2

#define RC_MOTOR_PIN_L	2
#define RC_MOTOR_PIN_C	3
#define RC_MOTOR_PIN_R	4




RSP     RovSerial;
Servo   RovServo[3];  


bool IsConnected;



void rc_setup( void );
void process_recv_cmd( void );

void send_cmd_info( void );
void recv_cmd_control( RSP_CMD_OBJ *pCmd );

void setup() 
{
	Serial.begin(115200);

	RovSerial.begin(115200); 
	RovServo[RC_MOTOR_L].attach(RC_MOTOR_PIN_L, 1000, 2000, 0, 255 );
	RovServo[RC_MOTOR_C].attach(RC_MOTOR_PIN_C, 1000, 2000, 0, 255 );
	RovServo[RC_MOTOR_R].attach(RC_MOTOR_PIN_R, 1000, 2000, 0, 255 );

	RovServo[RC_MOTOR_L].write(0);
	RovServo[RC_MOTOR_C].write(0);
	RovServo[RC_MOTOR_R].write(0);


	IsConnected = false;
}



void loop() 
{
	static uint32_t tTime[8];


	//-- 명령어 수신 처리 
	//
	process_recv_cmd();


	#if USE_RC_SETUP ==  1
	rc_setup();
	#endif


	//-- 100ms마다 ROV정보 전달
	//
	if( (tTime[0] - millis()) >= 100 )
	{
		tTime[0] = millis();

		send_cmd_info();

		if( IsConnected == true )
		{
			//Serial.println("Connected");
		}
	}


	//-- 연결이 끊어진 상태 
	//
	if( IsConnected == false )
	{
		RovServo[RC_MOTOR_L].write(0);
		RovServo[RC_MOTOR_C].write(0);
		RovServo[RC_MOTOR_R].write(0);		
	}
}



void rc_setup( void )
{
	char ch;
	static int  rc_pwm = 0;

	if( Serial.available() )
	{
		ch = Serial.read();

		if( ch == '1' )
		{
			rc_pwm = 0;
			RovServo[0].write(rc_pwm);
			Serial.println("RC 0");
		}

		if( ch == '2' )
		{
			rc_pwm = 255;
			RovServo[0].write(rc_pwm);
			Serial.println("RC 255");
		}

		if( ch == 'q' )
		{
			rc_pwm = constrain(rc_pwm++, 0, 255);
			RovServo[0].write(rc_pwm);
			Serial.println(rc_pwm);
		}

		if( ch == 'a' )
		{
			rc_pwm = constrain(rc_pwm--, 0, 255);
			RovServo[0].write(rc_pwm);
			Serial.println(rc_pwm);
		}
	}
}

void process_recv_cmd( void )
{
	bool Ret;
	uint8_t i;
	RSP_CMD_OBJ    *pCmd;
	static uint32_t tTime;


	Ret = RovSerial.update();

	if( Ret == true )
	{
        pCmd  = RovSerial.Get_CmdPtr();	

		/*	
		Serial.print("Cmd : ");
		Serial.print( pCmd->Cmd, HEX);
		Serial.print(" Size : ");
		Serial.print( pCmd->Length, DEC);
		Serial.print(" ");

		for( i=0; i<pCmd->Length; i++ )
		{
			Serial.print(pCmd->Data[i]);
			Serial.print(" ");
		}
		Serial.println(" ");
		*/

		switch( pCmd->Cmd )
		{
			case 0x01:
				IsConnected = true;
				recv_cmd_control( pCmd );
				break;

			case 0xFE:
				IsConnected = true;
				break;
		}	
	}
	else
	{
		if( (tTime-millis()) >= 1000 )
		{
			IsConnected = false;
		}
		tTime = millis();
	}
}



void send_cmd_info( void )
{
	RSP_CMD_OBJ Cmd;

	Cmd.Cmd     = 0xFF;
	Cmd.Length  = 1;
	Cmd.Data[0] = 100;

	RovSerial.SendCmd( &Cmd );
}



void recv_cmd_control( RSP_CMD_OBJ *pCmd )
{
	uint16_t Pwm[3];

	//-- 모터 제어 
	//
	Pwm[0] = (pCmd->Data[8]<<8) | (pCmd->Data[7]);
	Serial.print("RC : ");
	Serial.println(Pwm[0]);
	RovServo[RC_MOTOR_L].write(Pwm[0]);


	//-- LED 제어 
	//
}

