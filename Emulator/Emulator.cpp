#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")

#define STUDENT_NUMBER    "17009972"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x10 //one bit each
#define FLAG_V  0x08
#define FLAG_N  0x04
#define FLAG_Z  0x02
#define FLAG_C  0x01
#define REGISTER_A	5
#define REGISTER_F	4
#define REGISTER_E	3
#define REGISTER_D	2
#define REGISTER_C	1
#define REGISTER_B	0
#define REGISTER_X 0
#define REGISTER_Y 1
BYTE Index_Registers[2];

BYTE Registers[6];
BYTE Flags; //to store flgs
BYTE saved_flags;
WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200
#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
"ILLEGAL     ", 
"ILLEGAL     ", 
"STX abs      ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"MV  #,B      ", 
"MV  #,C      ", 
"MV  #,D      ", 
"MV  #,E      ", 
"MV  #,F      ", 
"MAY impl     ", 
"MYA impl     ", 
"MAS impl     ", 
"CSA impl     ", 

"ILLEGAL     ", 
"ILLEGAL     ", 
"STX abs,X    ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"SWI impl     ", 
"RTI impl     ", 
"CLC impl     ", 
"SEC impl     ", 
"CLI impl     ", 
"STI impl     ", 
"STV impl     ", 
"CLV impl     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 

"ILLEGAL     ", 
"ILLEGAL     ", 
"STX abs,Y    ", 
"ADD A,B      ", 
"SUB A,B      ", 
"CMP A,B      ", 
"OR A,B       ", 
"AND A,B      ", 
"EOR A,B      ", 
"BT A,B       ", 
"LD A,A       ", 
"LD B,A       ", 
"LD C,A       ", 
"LD D,A       ", 
"LD E,A       ", 
"LD F,A       ", 

"ILLEGAL     ", 
"LDX  #       ", 
"STX abs,XY   ", 
"ADD A,C      ", 
"SUB A,C      ", 
"CMP A,C      ", 
"OR A,C       ", 
"AND A,C      ", 
"EOR A,C      ", 
"BT A,C       ", 
"LD A,B       ", 
"LD B,B       ", 
"LD C,B       ", 
"LD D,B       ", 
"LD E,B       ", 
"LD F,B       ", 

"ILLEGAL     ", 
"LDX abs      ", 
"STX (ind),XY ", 
"ADD A,D      ", 
"SUB A,D      ", 
"CMP A,D      ", 
"OR A,D       ", 
"AND A,D      ", 
"EOR A,D      ", 
"BT A,D       ", 
"LD A,C       ", 
"LD B,C       ", 
"LD C,C       ", 
"LD D,C       ", 
"LD E,C       ", 
"LD F,C       ", 

"ILLEGAL     ", 
"LDX abs,X    ", 
"ILLEGAL     ", 
"ADD A,E      ", 
"SUB A,E      ", 
"CMP A,E      ", 
"OR A,E       ", 
"AND A,E      ", 
"EOR A,E      ", 
"BT A,E       ", 
"LD A,D       ", 
"LD B,D       ", 
"LD C,D       ", 
"LD D,D       ", 
"LD E,D       ", 
"LD F,D       ", 

"ILLEGAL     ", 
"LDX abs,Y    ", 
"ILLEGAL     ", 
"ADD A,F      ", 
"SUB A,F      ", 
"CMP A,F      ", 
"OR A,F       ", 
"AND A,F      ", 
"EOR A,F      ", 
"BT A,F       ", 
"LD A,E       ", 
"LD B,E       ", 
"LD C,E       ", 
"LD D,E       ", 
"LD E,E       ", 
"LD F,E       ", 

"ILLEGAL     ", 
"LDX abs,XY   ", 
"ILLEGAL     ", 
"NOP impl     ", 
"HLT impl     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"LD A,F       ", 
"LD B,F       ", 
"LD C,F       ", 
"LD D,F       ", 
"LD E,F       ", 
"LD F,F       ", 

"ILLEGAL     ", 
"LDX (ind),XY ", 
"ADI  #       ", 
"SBI  #       ", 
"CPI  #       ", 
"ORI  #       ", 
"ANI  #       ", 
"XRI  #       ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 

"LDA  #       ", 
"TST abs      ", 
"INC abs      ", 
"DEC abs      ", 
"RCR abs      ", 
"RLC abs      ", 
"ASL abs      ", 
"SAR abs      ", 
"COM abs      ", 
"RAL abs      ", 
"ROR abs      ", 
"LX  #,A      ", 
"ILLEGAL     ", 
"LODS  #      ", 
"PUSH  ,A     ", 
"POP A,       ", 

"LDA abs      ", 
"TST abs,X    ", 
"INC abs,X    ", 
"DEC abs,X    ", 
"RCR abs,X    ", 
"RLC abs,X    ", 
"ASL abs,X    ", 
"SAR abs,X    ", 
"COM abs,X    ", 
"RAL abs,X    ", 
"ROR abs,X    ", 
"ILLEGAL     ", 
"STO abs      ", 
"LODS abs     ", 
"PUSH  ,s     ", 
"POP s,       ", 

"LDA abs,X    ", 
"TST abs,Y    ", 
"INC abs,Y    ", 
"DEC abs,Y    ", 
"RCR abs,Y    ", 
"RLC abs,Y    ", 
"ASL abs,Y    ", 
"SAR abs,Y    ", 
"COM abs,Y    ", 
"RAL abs,Y    ", 
"ROR abs,Y    ", 
"ILLEGAL     ", 
"STO abs,X    ", 
"LODS abs,X   ", 
"PUSH  ,B     ", 
"POP B,       ", 

"LDA abs,Y    ", 
"TST abs,XY   ", 
"INC abs,XY   ", 
"DEC abs,XY   ", 
"RCR abs,XY   ", 
"RLC abs,XY   ", 
"ASL abs,XY   ", 
"SAR abs,XY   ", 
"COM abs,XY   ", 
"RAL abs,XY   ", 
"ROR abs,XY   ", 
"ILLEGAL     ", 
"STO abs,Y    ", 
"LODS abs,Y   ", 
"PUSH  ,C     ", 
"POP C,       ", 

"LDA abs,XY   ", 
"TSTA A,A     ", 
"INCA A,A     ", 
"DECA A,A     ", 
"RCRA A,A     ", 
"RLCA A,A     ", 
"ASLA A,A     ", 
"SARA A,A     ", 
"COMA A,A     ", 
"RALA A,A     ", 
"RORA A,A     ", 
"RTN impl     ", 
"STO abs,XY   ", 
"LODS abs,XY  ", 
"PUSH  ,D     ", 
"POP D,       ", 

"LDA (ind),XY ", 
"DEX impl     ", 
"INX impl     ", 
"DEY impl     ", 
"INCY impl    ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"ILLEGAL     ", 
"JSR abs      ", 
"JMP abs      ", 
"ILLEGAL     ", 
"STO (ind),XY ", 
"LODS (ind),XY", 
"PUSH  ,E     ", 
"POP E,       ", 

"BRA rel      ", 
"BCC rel      ", 
"BCS rel      ", 
"BNE rel      ", 
"BEQ rel      ", 
"BVC rel      ", 
"BVS rel      ", 
"BMI rel      ", 
"BPL rel      ", 
"BGE rel      ", 
"BLE rel      ", 
"BGT rel      ", 
"BLT rel      ", 
"ILLEGAL     ", 
"PUSH  ,F     ", 
"POP F,       ", 

}; 

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////

BYTE fetch() //dont edit
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}


void set_flag_n(BYTE inReg) 
{
	BYTE reg; 
	reg = inReg; 

	if ((reg & 0x80) != 0) // msbit set 
	{ 
		Flags = Flags | FLAG_N;
	}
	else 
	{ 
		Flags = Flags & (0xFF - FLAG_N);
	}
}
void set_flag_v(BYTE in1, BYTE in2, BYTE out1)
{
	BYTE reg1in;
	BYTE reg2in;
	BYTE regOut;
	reg1in = in1;
	reg2in = in2;
	regOut = out1;
	if ((((reg1in & 0x80) == 0x80) && ((reg2in & 0x80) == 0x80) && ((regOut & 0x80) != 0x80)) || (((reg1in & 0x80) != 0x80) && ((reg2in & 0x80) != 0x80) && ((regOut & 0x80) == 0x80))) //overflow
	{
		Flags = Flags | FLAG_V;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_V);
	}
}

void set_flag_z(BYTE inReg)
{

	if (inReg == 0x00)
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

void Group_1(BYTE opcode)
{
	BYTE LB = 0;
	BYTE HB = 0;
	WORD address = 0;
	WORD data = 0;
	WORD temp_word = 0;
	BYTE param1, param2;
	WORD offset;

	switch(opcode) 
	{
		case 0x90: //LDA - Loads Memory into Accumulator
			data = fetch();
			Registers[REGISTER_A] = data;
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xA0:
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE){
				Registers[REGISTER_A] = Memory[address];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xB0:
			address += Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE) {
				Registers[REGISTER_A] = Memory[address];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xC0:
			address += Index_Registers[REGISTER_Y];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE){
				Registers[REGISTER_A] = Memory[address];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xD0:
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8) + Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE){
				Registers[REGISTER_A] = Memory[address];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xE0:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address]; 
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			address += Index_Registers[REGISTER_X];
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8);
			if (address >= 0 && address < MEMORY_SIZE)	{
				Registers[REGISTER_A] = Memory[address];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break; 


		case 0xAC: //STO - Stores Accumulator into Memory
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = Registers[REGISTER_A];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xBC:
			address += Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = Registers[REGISTER_A];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xCC:
			address += Index_Registers[REGISTER_Y];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = Registers[REGISTER_A];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xDC:
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8) + Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Memory[address] = Registers[REGISTER_A];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xEC:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address];
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			address += Index_Registers[REGISTER_X];
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8);
			if (address >= 0 && address < MEMORY_SIZE) {
				Memory[address] = Registers[REGISTER_A];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
			

		case 0x23: //ADD - Register added to Accumulator with carry
			param1 = Registers[REGISTER_A]; 
			param2 = Registers[REGISTER_B];

			temp_word = (WORD)param1 + (WORD)param2;
			if ((Flags & FLAG_C) != 0)
			{
				temp_word++;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x33: 
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_C];

			temp_word = (WORD)param1 + (WORD)param2;
			if ((Flags & FLAG_C) != 0)
			{
				temp_word++;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x43: 
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_D];

			temp_word = (WORD)param1 + (WORD)param2;
			if ((Flags & FLAG_C) != 0)
			{
				temp_word++;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x53: 
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_E];

			temp_word = (WORD)param1 + (WORD)param2;
			if ((Flags & FLAG_C) != 0)
			{
				temp_word++;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x63: 
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_F];

			temp_word = (WORD)param1 + (WORD)param2;
			if ((Flags & FLAG_C) != 0)
			{
				temp_word++;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		

		case 0x24: //SUB - Register Subtracted to accumulator with carry
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_B];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_B];
			if ((Flags & FLAG_C) != 0)
			{
				temp_word--;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x34:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_C];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_C];
			if ((Flags & FLAG_C) != 0)
			{
				temp_word--;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x44:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_D];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_D];
			if ((Flags & FLAG_C) != 0)
			{
				temp_word--;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x54:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_E];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_E];
			if ((Flags & FLAG_C) != 0)
			{
				temp_word--;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x64:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_F];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_F];
			if ((Flags & FLAG_C) != 0)
			{
				temp_word--;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		

		case 0x25: //CMP - Register compared to Accumulator
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_B];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_B];
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			break;
		case 0x35:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_C];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_C];
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			break;
		case 0x45:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_D];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_D];
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			break;
		case 0x55:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_E];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_E];
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			break;
		case 0x65:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_F];
			temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_F];
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_v(param1, -param2, (BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			break;


		case 0x26: //OR - Register bitwise inclusive or with Accumulator
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_B];

			temp_word = (WORD)param1 | (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x36:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_C];

			temp_word = (WORD)param1 | (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x46:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_D];

			temp_word = (WORD)param1 | (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x56:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_E];

			temp_word = (WORD)param1 | (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x66:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_F];

			temp_word = (WORD)param1 | (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;


		case 0x27: //AND - Register	bitwise and with accumulator
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_B];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x37:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_C];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x47:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_D];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x57:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_E];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x67:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_F];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;


		case 0x28: //EOR - Register bitwise exclusive or with Accumulator
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_B];

			temp_word = (WORD)param1 ^ (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x38:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_C];

			temp_word = (WORD)param1 ^ (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x48:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_D];

			temp_word = (WORD)param1 ^ (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x58:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_E];

			temp_word = (WORD)param1 ^ (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
		case 0x68:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_F];

			temp_word = (WORD)param1 ^ (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;


		case 0x29: //BT Register bit tested with accumulator
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_B];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			break;
		case 0x39:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_C];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			break;
		case 0x49:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_D];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V);
			break;
		case 0x59:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_E];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			break;
		case 0x69:
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_F];

			temp_word = (WORD)param1 & (WORD)param2;
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Flags = Flags & (0xFF - FLAG_V); 
			break;


		case 0x82: //ADI - Data added to Accumulator with carry
			param1 = Registers[REGISTER_A];
			param2 = Registers[REGISTER_B];

			temp_word = (WORD)param1 + (WORD)param2;
			if ((Flags & FLAG_C) != 0)
			{
				temp_word++;
			}
			if (temp_word >= 0x100)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			set_flag_v(param1, param2, (BYTE)temp_word);
			set_flag_n((BYTE)temp_word);
			set_flag_z((BYTE)temp_word);
			Registers[REGISTER_A] = (BYTE)temp_word;
			break;
	
		

		case 0x83: //SBI - Data subtracted to Accumulator with carry
			break;


		case 0x84: //CPI - Data Compared to Accumulator
			break;


		case 0x85: //ORI - Data bitwise inclusive or with Accumulator
			break;


		case 0x86: //ANI - Data bitwise and with Accumulator
			break;


		case 0x87: //XRI - Data bitwise exclusive or with Accumulator
			break;

		case 0x91: //TST - Bit test memory or Accumulator
			break;
		case 0xA1: 
			break;
		case 0xB1: 
			break;
		case 0xC1: 
			break;

		
		case 0xD1: //TSTA - Bit test Memory or Accumulator
			break;


		case 0x92: //INC - Increment Memory or Accumulator
			break;
		case 0xA2: 
			break;
		case 0xB2: 
			break;
		case 0xC2: 
			break;


		case 0xD2: //INCA - Increment Memory or Accumulator
			++Registers[REGISTER_A];
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x93: //DEC - Decrement Memory or Accumulator
			break;
		case 0xA3: 
			break;
		case 0xB3: 
			break;
		case 0xC3: 
			break;


		case 0xD3: //DECA - Decrement Memory or Accumulator
			--Registers[REGISTER_A];
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x94: //RCR - Rotate Right through carry Memory or Accumulator
			break;
		case 0xA4: 
			break;
		case 0xB4: 
			break;
		case 0xC4: 
			break;


		case 0xD4: //RCRA - Rotate right through carry Memory or Accumulator
			saved_flags = Flags;
			if ((Registers[REGISTER_A] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0xFE;
			if ((saved_flags & FLAG_C) == FLAG_C)
			{
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x95: //RLC - Rotate left through carry Memory or Accumulator
			break;
		case 0xA5:
			break;
		case 0xB5:
			break;
		case 0xC5:
			break;


		case 0xD5: //RLCA - Rotate left thorugh carry memory or accumulator
			saved_flags = Flags;
			if ((Registers[REGISTER_A] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
			if ((saved_flags & FLAG_C) == FLAG_C)
			{
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x96: //ASL - Arithmetic shift left Memory or Accumulator
			break;
		case 0xA6:
			break;
		case 0xB6:
			break;
		case 0xC6:
			break;


		case 0xD6: //ASLA - Arithmetic shift left Memory or Accumulator
			saved_flags = Flags;
			if ((Registers[REGISTER_A] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x97: //SAR - Arithmetic shift right Memory or Accumulator
			break;
		case 0xA7:
			break;
		case 0xB7:
			break;
		case 0xC7:
			break;


		case 0xD7: //SARA - Arithmetic shift right Memory or Accumulator
			saved_flags = Flags;
			if ((Registers[REGISTER_A] & 0x01) == 0x01)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;
			if ((Flags & FLAG_N) == FLAG_N)
			{
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x98: //COM - Negate Memory or Accumulator
			break;
		case 0xA8:
			break;
		case 0xB8:
			break;
		case 0xC8:
			break;


		case 0xD8: //COMA - Negate Memory or Accumulator
			break;

		case 0x99: //RAL - Rotate left without carry Memory or Accumulator
			break;
		case 0xA9:
			break;
		case 0xB9:
			break;
		case 0xC9:
			break;


		case 0xD9: //RALA - Rotate left without carry memory or Accumulator

			Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x9A: //ROR - Rotate right without carry Memory or Accumulator
			break;
		case 0xAA:
			break;
		case 0xBA:
			break;
		case 0xCA:
			break;


		case 0xDA: //RORA - Rotate right without carry memory or Accumulator
			if ((Registers[REGISTER_A] & 0x80) == 0x80)
			{
				Flags = Flags | FLAG_C;
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}
			Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0xFE;
			if ((saved_flags & FLAG_C) == FLAG_C)
			{
				Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x31: //LDX - Loads Memory into Register X
			data = fetch();
			Index_Registers[REGISTER_X] = data;
			set_flag_n(Index_Registers[REGISTER_X]);
			set_flag_z(Index_Registers[REGISTER_X]);
			break;
		case 0x41:
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_X]);
			set_flag_z(Index_Registers[REGISTER_X]);
			break;
		case 0x51:

			address += Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_A]);
			set_flag_z(Index_Registers[REGISTER_A]);
			break;
		case 0x61:
			address += Index_Registers[REGISTER_Y];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_A]);
			set_flag_z(Index_Registers[REGISTER_A]);
			break;
		case 0x71:
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8) + Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_A]);
			set_flag_z(Index_Registers[REGISTER_A]);
			break;
		case 0x81:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address];
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			address += Index_Registers[REGISTER_X];
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8);
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_X]);
			set_flag_z(Index_Registers[REGISTER_X]);
			break;


		case 0x02: //STX - Stores Register X into Memory
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_X]);
			set_flag_z(Index_Registers[REGISTER_X]);
			break;
		case 0x12:

			address += Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_X]);
			set_flag_z(Index_Registers[REGISTER_X]);
			break;
		case 0x22:
			address += Index_Registers[REGISTER_Y];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_X]);
			set_flag_z(Index_Registers[REGISTER_X]);
			break;
		case 0x32:
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8) + Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_X]);
			set_flag_z(Index_Registers[REGISTER_X]);
			break;
		case 0x42:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address];
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			address += Index_Registers[REGISTER_X];
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8);
			if (address >= 0 && address < MEMORY_SIZE)
			{
				Index_Registers[REGISTER_X] = Memory[address];
			}
			set_flag_n(Index_Registers[REGISTER_X]);
			set_flag_z(Index_Registers[REGISTER_X]);
			break;


		case 0xE1: //DEX decrements register X
			--Index_Registers[REGISTER_X];
			set_flag_z(Index_Registers[REGISTER_X]);
			break;


		case 0xE2: //INX - Increments Register X
			++Index_Registers[REGISTER_X];
			set_flag_z(Index_Registers[REGISTER_X]);
			break;


		case 0x0C: //MAY - Transfers Accumulator to Register Y
			data = Registers[REGISTER_A];
			Registers[REGISTER_Y] = data;
			set_flag_n(Registers[REGISTER_A]);
			break;


		case 0x0D: //MYA - Transfers register Y to Accumulator 
			data = Registers[REGISTER_A];
			Registers[REGISTER_Y] = data;
			set_flag_n(Registers[REGISTER_A]);
			break;


		case 0xE3: //DEY decrements register Y
			--Index_Registers[REGISTER_Y];
			set_flag_z(Index_Registers[REGISTER_Y]);
			break;


		case 0xE4: //INCY - Increments Register Y
			++Index_Registers[REGISTER_Y];
			set_flag_z(Index_Registers[REGISTER_Y]);
			break;


		case 0x9D: //LODS - Loads Memory into Stackpointer
			data = fetch();
			StackPointer = data << 8;
			StackPointer += fetch();
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xAD:
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE - 1)
			{
				StackPointer = (WORD)(Memory[address] << 8);
				StackPointer += Memory[address + 1];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xBD:
			address += Index_Registers[REGISTER_X];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE - 1)
			{
				StackPointer = (WORD)Memory[address] << 8;
				StackPointer += Memory[address + 1];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xCD:
			address += Index_Registers[REGISTER_Y];
			HB = fetch();
			LB = fetch();
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE - 1)
			{
				StackPointer = (WORD)Memory[address] << 8;
				StackPointer += Memory[address + 1];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xDD:
			address += (WORD((WORD)Index_Registers[REGISTER_Y] << 8) + Index_Registers[REGISTER_X]);
			HB = fetch();
			LB = fetch();
			address += (WORD((WORD)HB << 8) + LB);
			if (address >= 0 && address < MEMORY_SIZE - 1)
			{
				StackPointer = (WORD)Memory[address] << 8;
				StackPointer += Memory[address + 1];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;
		case 0xED:
			HB = fetch();
			LB = fetch();
			address = (WORD)((WORD)HB << 8) + LB;
			HB = Memory[address];
			LB = Memory[address + 1];
			address = (WORD)((WORD)HB << 8) + LB;
			address += Index_Registers[REGISTER_X];
			address += (WORD)((WORD)Index_Registers[REGISTER_Y] << 8);
			if (address >= 0 && address < MEMORY_SIZE - 1)
			{
				StackPointer = (WORD)Memory[address] << 8;
				StackPointer += Memory[address + 1];
			}
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0x0E: //MAS - Transters Accumulator to status register
			Flags = Registers[REGISTER_A];
			break;


		case 0x0F: //CSA - Transfers Status register to Accumulator
			Registers[REGISTER_A] = Flags;
			break;


		case 0x9E: // PUSH - Pushes Register onto the stack
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				Memory[StackPointer] = Registers[REGISTER_A];
				StackPointer--;
			}
			break;
		case 0xAE: // Status Register?
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				Memory[StackPointer] = Registers[REGISTER_A];
				StackPointer--;
			}
			break;
		case 0xBE:
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				Memory[StackPointer] = Registers[REGISTER_B];
				StackPointer--;
			}
			break;
		case 0xCE:
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				Memory[StackPointer] = Registers[REGISTER_C];
				StackPointer--;
			}
			break;
		case 0xDE:
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				Memory[StackPointer] = Registers[REGISTER_D];
				StackPointer--;
			}
			break;
		case 0xEE:
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				Memory[StackPointer] = Registers[REGISTER_E];
				StackPointer--;
			}
			break;
		case 0xFE:
			if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE))
			{
				Memory[StackPointer] = Registers[REGISTER_F];
				StackPointer--;
			}
			break;


		case 0x9F: //POP - Pop the top of the Stack into the Register
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
			{
				StackPointer++;
				Registers[REGISTER_A] = Memory[StackPointer];
			}
			break;
		case 0xAF: // Status Register?
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
			{
				StackPointer++;
				Registers[REGISTER_A] = Memory[StackPointer];
			}
			break;
		case 0xBF:
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
			{
				StackPointer++;
				Registers[REGISTER_B] = Memory[StackPointer];
			}
			break;
		case 0xCF:
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
			{
				StackPointer++;
				Registers[REGISTER_C] = Memory[StackPointer];
			}
			break;
		case 0xDF:
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
			{
				StackPointer++;
				Registers[REGISTER_D] = Memory[StackPointer];
			}
			break;
		case 0xEF:
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
			{
				StackPointer++;
				Registers[REGISTER_E] = Memory[StackPointer];

			}
			break;
		case 0xFF:
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1))
			{
				StackPointer++;
				Registers[REGISTER_F] = Memory[StackPointer];
			}
			break;


		case 0x9B: //LX - loads memory into register pair AB,#
			data = fetch();
			data = Registers[REGISTER_A];
			data = Registers[REGISTER_B];
			set_flag_n(Registers[REGISTER_A]);
			set_flag_z(Registers[REGISTER_A]);
			break;


		case 0xEA://JMP - Loads memory in ProgramCounter
			HB = fetch();
			LB = fetch();
			address = ((WORD)HB << 8) + (WORD)LB;
			ProgramCounter = address;
			break;


		case 0x07: //MV - loads Memory into register
			data = fetch();
			Registers[REGISTER_B] = data;
			set_flag_n(Registers[REGISTER_B]);
			set_flag_z(Registers[REGISTER_B]);
			break; 
		case 0x08:
			data = fetch();
			Registers[REGISTER_C] = data;
			set_flag_n(Registers[REGISTER_C]);
			set_flag_z(Registers[REGISTER_C]);
			break; 
		case 0x09:
			data = fetch();
			Registers[REGISTER_D] = data;
			set_flag_n(Registers[REGISTER_D]);
			set_flag_z(Registers[REGISTER_D]);
			break; 
		case 0x0A:
			data = fetch();
			Registers[REGISTER_E] = data;
			set_flag_n(Registers[REGISTER_E]);
			set_flag_z(Registers[REGISTER_E]);
			break; 
		case 0x0B:
			data = fetch();
			Registers[REGISTER_F] = data;
			set_flag_n(Registers[REGISTER_F]);
			set_flag_z(Registers[REGISTER_F]);
			break; 


		case 0xE9://JSR - Jump To subroutine
			HB = fetch();
			LB = fetch();
			address = ((WORD)HB << 8) + (WORD)LB;
			if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
			{
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
			}
			break;
		

		case 0xDB: //RTN - Return from subroutine
			if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 2))
			{
				StackPointer++;
				HB = Memory[StackPointer];
				StackPointer++;
				LB = Memory[StackPointer];
			}
			ProgramCounter = ((WORD)HB << 8) + (WORD)LB;
			break;

		
		case 0xF0: //BRA - Branch always
			LB = fetch();
			offset = (WORD)LB;
			if ((offset & 0x80) != 0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
			break;


		case 0xF1: //BCC Branch on Carry clear
			LB = fetch();
			if ((Flags & FLAG_C) == 0)
			{
				offset = (WORD)LB;
				if ((offset & 0x80) != 0)
				{
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;
				ProgramCounter = address; //EXACT SAME CHANGE  != to == fr carry clear for multiple flags set up multiple varibles
			}
			break;


		case 0xF2: //BCS Branch on Carry set
			LB = fetch();
			if ((Flags & FLAG_C) != 0)
			{
				offset = (WORD)LB;
				if ((offset & 0x80) != 0)
				{
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;
				ProgramCounter = address; //EXACT SAME CHANGE  != to == fr carry clear for multiple flags set up multiple varibles
			}
			/*if ((Flags & FLAG_C) != 0)
			{
			BYTE temp.carry = 1;
			}
			else temp.carry = 0
			}
			*/
			//sor overflow and negative flag ! for other way rount
			break;


		case 0xF3: //BNE Branch on Result not zero
			LB = fetch();
			if ((Flags & FLAG_Z) != 0)
			{
				offset = (WORD)LB;
				if ((offset & 0x80) != 0)
				{
					offset = offset + 0xFF00;
				}
				address = ProgramCounter + offset;
				ProgramCounter = address; //EXACT SAME CHANGE  != to == fr carry clear for multiple flags set up multiple varibles
			}
			break;

		
		case 0xF4: //BEQ - Branch on carry set

			break;


		case 0xF5: //BVC - Branch on overflow clear

			break;


		case 0xF6: //BVS - branch on overflow set

			break;


		case 0xF7: //BMI - Breanch on negative result

			break;


		case 0xF8: //BPL - Branch on positive result

			break;


		case 0xF9: //BGE - Branch on result less than or equal to zero

			break;


		case 0xFA: //BLE - Branch on result Greater than or equal to zero

			break;


		case 0xFB: //BGT - Branch on result less than zero

			break;


		case 0xFC: //BLT - Branch on result greater than zero

			break;

		case 0x18: //CLC - Clear Carry flag
			Flags = Flags & (0xFF - FLAG_C);
			break;


		case 0x19: //SEC - Set Carry Flag
			Flags = Flags | FLAG_C;
			break;


		case 0x1A: //CLI - Clear Interupt flag
			Flags = Flags & (0xFF - FLAG_I);
			break;


		case 0x1B: //STI - Set interupt flag
			Flags = Flags | FLAG_I;
			break;


		case 0x1C: //STV - Set overflow flag
			Flags = Flags | FLAG_V;
			break;


		case 0x1D: //CLV - Clear overflow flag
			Flags = Flags & (0xFF - FLAG_V);
			break;


		case 0x73: //NOP - no operation
			break;


		case 0x74: //HLT - Wait for interupt
			halt = true;
			break;


		case 0x16: //SWI - Software interupt
			break;


		case 0x17: //RTI - Return from software interupt
			break;

		
	}
}

void Group_2_Move(BYTE opcode)
{
	BYTE destination = opcode >> 4; //top four bits point at one register....shift right to keep only top four
	BYTE source = opcode & 0x0F;//takes bottom four bits
	int destReg = 0;
	int sourceReg = 0;
	switch (destination)//COMBO OF DEST AND SOURCE GIVE REGISTERS ADDRESSES TO FOR MEMORY ADRESSER AT BOTTOM
	{
		case 0x02://top four bits from op code
			destReg = REGISTER_A;
			break;
		case 0x03:
			destReg = REGISTER_B;
			break;
		case 0x04:
			destReg = REGISTER_C;
			break;
		case 0x05:
			destReg = REGISTER_D;
			break;
		case 0x06:
			destReg = REGISTER_E;
			break;
		case 0x07:
			destReg = REGISTER_F;
			break;
	}

	switch (source)
	{
		case 0x0A:
			sourceReg = REGISTER_A;
			break;
		case 0x0B:
			sourceReg = REGISTER_B;
			break;
		case 0x0C:
			sourceReg = REGISTER_C;
			break;
		case 0x0D:
			sourceReg = REGISTER_D;
			break;
		case 0x0E:
			sourceReg = REGISTER_E;
			break;
		case 0x0F:
			sourceReg = REGISTER_F;
			break;
	}
	Registers[sourceReg] = Registers[destReg];//code to assign sourcereg to dest reg

}


void execute(BYTE opcode) //Dont edit
{
	if (((opcode >= 0x2A) && (opcode <= 0x2F))
		|| ((opcode >= 0x3A) && (opcode <= 0x3F))
		|| ((opcode >= 0x4A) && (opcode <= 0x4F))
		|| ((opcode >= 0x5A) && (opcode <= 0x5F))
		|| ((opcode >= 0x6A) && (opcode <= 0x6F))
		|| ((opcode >= 0x7A) && (opcode <= 0x7F)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}

void emulate()
{
	BYTE opcode;
	int sanity;

	sanity = 0;
	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	printf("                    A  B  C  D  E  F  X  Y  SP\n");

	while ((!halt) && (memory_in_range)) {
		sanity++;
		if (sanity > 500) halt = true;
		printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);
		printf("%02X ", Registers[REGISTER_B]);
		printf("%02X ", Registers[REGISTER_C]);
		printf("%02X ", Registers[REGISTER_D]);
		printf("%02X ", Registers[REGISTER_E]);
		printf("%02X ", Registers[REGISTER_F]);
		printf("%02X ", Index_Registers[REGISTER_X]);
		printf("%02X ", Index_Registers[REGISTER_Y]);
		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_I) == FLAG_I)	
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}
		if ((Flags & FLAG_V) == FLAG_V)	
		{
			printf("V=1 ");
		}
		else
		{
			printf("V=0 ");
		}
		if ((Flags & FLAG_N) == FLAG_N)	
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}
		if ((Flags & FLAG_Z) == FLAG_Z)	
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}
		if ((Flags & FLAG_C) == FLAG_C)	
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}

////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////

void initialise_filenames() {
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++) {
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}




int find_dot_position(char *filename) {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		if (chr == '.') {
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename) {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename) {
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) {
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename) {
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) {
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer) {
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect) {
		c = getc(fp);

		switch (c) {
		case EOF:
			if (i > 0) {
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0) {
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}






void load_and_run(int args,_TCHAR** argv) {
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if(args == 2){
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	} else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch(chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE)	{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0) {
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	} else {
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	} else {
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file)) {
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_F] = 0;
		Index_Registers[REGISTER_X] = 0;
		Index_Registers[REGISTER_Y] = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i=0; i<MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				} else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				} else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	} else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args,_TCHAR** argv){
	char buffer[1024];
	load_and_run(args,argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", 
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4], 
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6], 
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8], 
		Memory[TEST_ADDRESS_9], 
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
		);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
}


void test_and_mark() {
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			} else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			} else {
				// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_F] = 0;
		Index_Registers[REGISTER_X] = 0;
		Index_Registers[REGISTER_Y] = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE *ofp;
				fopen_s(&ofp ,"branch.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						} else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						} else {
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1) {
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X", 
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4], 
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6], 
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8], 
						Memory[TEST_ADDRESS_9], 
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
						);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");
	printf("%d",Registers[REGISTER_A]);
	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {	
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if(argc == 2){ building(argc,argv); exit(0);}
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc,argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}


