#ifndef hd44780_h
#define hd44780_h

#include <stdint.h>
#include <stm32f4xx.h>

//

/**
  * @brief DO NOT CHANGE: HD44780 constants
  *
  */
enum {
	LCDII_CMD_CLS = 1,        // Clear LCD; takes >1.5ms
	LCDII_CMD_CR = 2,         //Return home, set addr to 0
	LCDII_CMD_ENT_MODE4 = 4,  //Entry mode. Cursor decrement; Display shift OFF
	LCDII_CMD_ENT_MODE5 = 5,  //Entry mode. Cursor decrement; Display shift ON
	LCDII_CMD_ENT_MODE6 = 6,  //Entry mode. Cursor increment; Display shift OFF
	LCDII_CMD_ENT_MODE7 = 7,  //Entry mode. Cursor increment; Display shift ON
	LCDII_CMD_CTL_D_000 = 8,      // Display control. Display - off, cursor - off, blink - off
	LCDII_CMD_CTL_D_001 = 9,      // Display control. Display - off, cursor - off, blink - ON
	LCDII_CMD_CTL_D_010 = 10,     // Display control. Display - off, cursor - ON,  blink - off
	LCDII_CMD_CTL_D_011 = 11,     // Display control. Display - off, cursor - ON,  blink - ON
	LCDII_CMD_CTL_D_100 = 12,     // Display control. Display - ON,  cursor - off, blink - off
	LCDII_CMD_CTL_D_101 = 13,     // Display control. Display - ON,  cursor - off, blink - ON
	LCDII_CMD_CTL_D_110 = 14,     // Display control. Display - ON,  cursor - ON,  blink - off
	LCDII_CMD_CTL_D_111 = 15,     // Display control. Display - ON,  cursor - ON,  blink - ON
	LCDII_CMD_MOVE_00 = 16,     // Cursor/display shift. Move cursor - 0, left - 0
	LCDII_CMD_MOVE_01 = 20,     // Cursor/display shift. Move cursor - 0, right - 1
	LCDII_CMD_MOVE_10 = 24,     // Cursor/display shift. Move display - 1, left - 0
	LCDII_CMD_MOVE_11 = 28,     // Cursor/display shift. Move display - 1, right - 1
	LCDII_CMD_CG_ADDR = 64,     // Set Code Generator address
	LCDII_CMD_DM_ADDR = 128     // Set Display Memory DRAM address
} ;


enum {
	LCDII_INIT_DNF_418  = 32,  // INIT sequence only. Data bus Length 4 bit - 0, Number of lines 1 - 0, Font 5x8 - 0
	LCDII_INIT_DNF_4111 = 36,  // INIT sequence only. Data bus Length 4 bit - 0, Number of lines 1 - 0, Font 5x11 - 1
	LCDII_INIT_DNF_428  = 40,  // INIT sequence only. Data bus Length 4 bit - 0, Number of lines 2 - 1, Font 5x8 - 0
	LCDII_INIT_DNF_4211 = 44,  // INIT sequence only. Data bus Length 4 bit - 0, Number of lines 2 - 1, Font 5x11 - 1
	LCDII_INIT_DNF_818  = 48,  // INIT sequence only. Data bus Length 8 bit - 1, Number of lines 1 - 0, Font 5x8 - 0
	LCDII_INIT_DNF_8111 = 52,  // INIT sequence only. Data bus Length 8 bit - 1, Number of lines 1 - 0, Font 5x11 - 1
	LCDII_INIT_DNF_828  = 56,  // INIT sequence only. Data bus Length 8 bit - 1, Number of lines 2 - 1, Font 5x8 - 0
	LCDII_INIT_DNF_8211 = 60,  // INIT sequence only. Data bus Length 8 bit - 1, Number of lines 2 - 1, Font 5x11 - 1
} ;


/**
  * @brief LCDII_t Data type
  * @param GPIO_TypeDef *, Delay_Function, GPIOx first pin. Eg: Pin0 or Pin3
  * @retval None
  *
  * For LCDII to work in 4 bit mode it requires 7 consecutive GPIO ports. They are split as follows:
  * pinX+0 .. pinX+3 - 4 data bits
  * pinX+4 - LCD pin RW; high - read mode, low - write mode
  * pinX+5 - LCD pin RS; high - data mode, low - command mode
  * pinX+6 - LCD pin E; enable, signal High->Low latches data in register
  */
typedef struct  
{
  GPIO_TypeDef * LCDII_PORT_PTR; // GPIO_TypeDef
  void (* LCDII_DELAY_PTR)(uint32_t);  // Address of a custom delay function (ms)
  volatile uint8_t LCDII_D0_D4_PIN; // GPIOx pin number connected to LCD D4 (or D0 in 8 bit mode)
} LCDII_t;



/**
  * @brief Base_t type, output in BIN, OCT, DEC or HEX formats
  *
  * Used by LCDII_PrintN function to output numbers in BIN, OCT, DEC or HEX formats
  */
typedef enum {
  BIN = 2,
  OCT = 8,
  DEC = 10,
  HEX = 16
} Base_t;


// Public functions
void LCDII_SendCommand(LCDII_t LCDII_Handle, uint8_t cmd);
void LCDII_SendData(LCDII_t LCDII_Handle, uint8_t one_char);
void LCDII_Init4(LCDII_t LCDII_Handle);
void LCDII_Print(LCDII_t LCDII_Handle, char msg[], uint8_t msg_len, uint8_t position);
void LCDII_PrintN(LCDII_t LCDII_Handle,uint32_t number_to_print, Base_t base, uint8_t msg_len, uint8_t position);
uint8_t LCDII_Read_BF_Addr(LCDII_t LCDII_Handle);
uint8_t LCDII_ReadData(LCDII_t LCDII_Handle);

#endif
