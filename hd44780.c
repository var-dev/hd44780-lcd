#include "hd44780.h"

// ChangeMe: Timing constants, LCDII_Handle.LCDII_DELAY_PTR() function implementation specific
#define INIT_30mS 31
#define INIT_4mS 5
#define INIT_1mS 2
#define E_PULSE_WIDTH_450nS 1


// DO NOT CHANGE: HD44780 private constants



const enum {
	OFFSET_SIG_RW = 4,
	OFFSET_SIG_RS = 5,
	OFFSET_SIG_E  = 6
} OFFSET_SIG = OFFSET_SIG_RW;

typedef const enum {READ_ADDR = 1, READ_DATA} LCDII_Read_t;

#define LCDII_DELAY_LOOP(CYCLES) for (uint32_t j = 0; j < CYCLES; j++){ __NOP(); }

// Private function prototypes
static void LCDII_Set_Nibble(LCDII_t LCDII_Handle, uint8_t nibble);
static void LCDII_Pulse(LCDII_t LCDII_Handle);
static uint8_t LCDII_Read(LCDII_t LCDII_Handle, LCDII_Read_t read_op);


/**
  * @brief LCD Initialization Function
  * @param LCDII_Handle - LCDII_t struct instance
  * @retval None
  *
  * See LCDII_t description for details
  */
void LCDII_Init4(LCDII_t LCDII_Handle)
{
  LCDII_Handle.LCDII_DELAY_PTR(INIT_30mS);
  LCDII_Handle.LCDII_PORT_PTR->MODER &= ~(0x3FFFU << LCDII_Handle.LCDII_D0_D4_PIN * 2); // reset 7 GPIOx pins starting with LCDII_Handle.LCDII_D0_D4_PIN
  LCDII_Handle.LCDII_PORT_PTR->MODER |= (0x1555U << LCDII_Handle.LCDII_D0_D4_PIN * 2);  // write 01 (GP output) to 7 GPIOx starting with LCDII_Handle.LCDII_D0_D4_PIN
  LCDII_Handle.LCDII_PORT_PTR->OTYPER &= ~(0x7FU << LCDII_Handle.LCDII_D0_D4_PIN);   // output type: 0 - normal, 1 - open drain
  LCDII_Handle.LCDII_PORT_PTR->OSPEEDR &= ~(0x3FFFU << LCDII_Handle.LCDII_D0_D4_PIN * 2); // output speed 00 - low, 01 - med, 10 - high, 11 - very high
  LCDII_Handle.LCDII_PORT_PTR->PUPDR &= ~(0x3FFFU << LCDII_Handle.LCDII_D0_D4_PIN * 2);   // 00 - no pull up or down, 01 - pull up, 10 - down
  
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RS)); //Set pin RS low (Command mode)
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RW)); //Set pin RW low (Write mode)
  
  for(uint8_t i = 0; i < 3; i++){
    LCDII_Set_Nibble(LCDII_Handle, 0x30U >> 4); // Send 0x30 command 3 times
    LCDII_Pulse(LCDII_Handle);  // Toggle LCD E pin
    LCDII_Handle.LCDII_DELAY_PTR(INIT_4mS);
  }
  
  LCDII_Set_Nibble(LCDII_Handle, LCDII_INIT_DNF_418 >> 4); // Send 0b100000/0x20/32 command to turn on 4 bit mode
  LCDII_Pulse(LCDII_Handle);  // Toggle LCD E pin
  LCDII_SendCommand(LCDII_Handle, LCDII_INIT_DNF_428);
  LCDII_SendCommand(LCDII_Handle, LCDII_CMD_CLS); // Send "clear display"
  LCDII_Handle.LCDII_DELAY_PTR(INIT_1mS);
  LCDII_SendCommand(LCDII_Handle, LCDII_CMD_ENT_MODE6); // Send "entry mode"
  LCDII_SendCommand(LCDII_Handle, LCDII_CMD_CTL_D_111); // Send "display on, cursor on, blink on"
}

/**
  * @brief PRIVATE: Set 4 bit nibble
  * @param LCDII_Handle - LCDII struct instance
  * @param nibble
  * @retval None
  */
static void LCDII_Set_Nibble(LCDII_t LCDII_Handle, uint8_t nibble)
{
  nibble &= 0xF; //Sanity check. Only 4 lower bits are allowed
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(0xFU << LCDII_Handle.LCDII_D0_D4_PIN); // We shouldn't touch upper 4 bits so we reset the lower 4 first;
  LCDII_Handle.LCDII_PORT_PTR->ODR |= (uint32_t)(nibble << LCDII_Handle.LCDII_D0_D4_PIN);// and than OR them with 4 GPIO bits
}

/**
  * @brief PRIVATE: Toggle E pin
  * @param LCDII_Handle - LCDII struct instance
  * @retval None
  */
static void LCDII_Pulse(LCDII_t LCDII_Handle){
  LCDII_Handle.LCDII_PORT_PTR->ODR |= (1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_E)); //Set LCD E pin high, then wait
  LCDII_Handle.LCDII_DELAY_PTR(E_PULSE_WIDTH_450nS);
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_E)); //Now set pin E low
  LCDII_Handle.LCDII_DELAY_PTR(E_PULSE_WIDTH_450nS);
}  

/**
  * @brief Send command to HD44780.
  * @param LCDII_Handle - LCDII struct instance
  * @param cmd - command to be sent to HD44780;
  * @retval None
  */
void LCDII_SendCommand(LCDII_t LCDII_Handle, uint8_t cmd){
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RS)); //Set pin RS low (Command mode)
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RW)); //Set pin RW low (Write mode)
  LCDII_Set_Nibble(LCDII_Handle, cmd >> 4); // Set high order nibble
  LCDII_Pulse(LCDII_Handle);  // Toggle LCD E pin
  LCDII_Set_Nibble(LCDII_Handle, cmd); // Set lower order nibble
  LCDII_Pulse(LCDII_Handle);  // Toggle LCD E pin 
}


/**
  * @brief Send single char to HD44780.
  * @param LCDII_Handle - LCDII struct instance
  * @param one_char - single character to send to HD44780;
  * @retval None
  */
void LCDII_SendData(LCDII_t LCDII_Handle, uint8_t one_char){
  LCDII_Handle.LCDII_PORT_PTR->ODR |= (1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RS)); //Set LCD pin RS high (Data mode)
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RW)); //Set pin RW low (Write mode)
  LCDII_Set_Nibble(LCDII_Handle, one_char >> 4); // Set high order nibble
  LCDII_Pulse(LCDII_Handle);  // Toggle LCD E pin
  LCDII_Set_Nibble(LCDII_Handle, one_char); // Set lower order nibble
  LCDII_Pulse(LCDII_Handle);  // Toggle LCD E pin 
}

/**
  * @brief Print a number on HD44780.
  * @param LCDII_Handle - LCDII struct instance
  * @param msg[] - string to display on HD44780
  * @param msg_len - number of characters to be printed
  * @param position - starting position. 0 - first line, 0x40 - second
  * @retval None
  */
void LCDII_Print(LCDII_t LCDII_Handle, char msg[], uint8_t msg_len, uint8_t position) {
  LCDII_SendCommand(LCDII_Handle,0x80 | position); // 0x80 is "set DRAM addr" cmd , position is the LCD DRAM address
  for (uint8_t i = 0; i < msg_len; i++) {
    LCDII_SendData(LCDII_Handle, msg[i]);
  }
  return;
}

/**
  * @brief Print a number on HD44780.
  * @param LCDII_Handle - LCDII struct instance
  * @param number_to_print - 32bit number to display on HD44780
  * @param base - base of the numerical system: BIN, OCT, DEC, HEX
  * @param msg_len - number of characters to be printed
  * @param position - starting position. 0 - first line, 0x40 - second
  * @retval None
  *
  * LCDII_PrintN is meant to be used instead of sprintf
  * E.g. LCDII_PrintN(lcd1, 65535, DEC, 5, 0x42); will print a 5 char long decimal number on 2nd line, 3rd position.
  */
void LCDII_PrintN(LCDII_t LCDII_Handle, uint32_t number_to_print, Base_t base, uint8_t msg_len, uint8_t position) {
  if( msg_len == 0 ) {
    return;
  }
  char buffer[0x32];
  for(uint8_t i = 0; i < 32; i++) {
    buffer[i] = 0x20;   // Fill buffer with spaces
  }
  int8_t i = (int8_t)msg_len - 1;
  do {
    uint8_t remainder = (uint8_t)( number_to_print % base );
    if ( remainder > 9 ) {
      buffer[i--] = remainder + 0x37;   //ASCII ABCDF (add 7 to skip 3A-40 characters)
    } else {
      buffer[i--] = remainder + 0x30;   //ASCII 0123456789
    }
    number_to_print /= base;
  } while (i > -1);
  
  LCDII_Print(LCDII_Handle, buffer, msg_len, position);
  return;
}

static uint8_t LCDII_Read(LCDII_t LCDII_Handle, LCDII_Read_t read_op){
  uint8_t nibble_h = 0, nibble_l = 0, byte = 0;
  if (read_op == READ_ADDR){
	  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RS)); //Set pin RS low (cmd mode)
  } else if (read_op == READ_DATA) {
	  LCDII_Handle.LCDII_PORT_PTR->ODR |= (1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RS)); //Set pin RS high (Data mode)
  } else {
	  return 0;
  }
  LCDII_Handle.LCDII_PORT_PTR->ODR |= (1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_RW)); //Set pin RW high (Read mode)
  LCDII_Handle.LCDII_PORT_PTR->MODER &= ~(uint32_t)(0xFFU << LCDII_Handle.LCDII_D0_D4_PIN * 2);  // write 00 (GP input) to 4 GPIOx data pins

  // Getting the higher bits nibble
  LCDII_Handle.LCDII_PORT_PTR->ODR |= (1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_E)); //Set LCD E pin high
  LCDII_Handle.LCDII_DELAY_PTR(E_PULSE_WIDTH_450nS); // Keep E high for  a few CPU cycles
  nibble_h = (uint8_t)((LCDII_Handle.LCDII_PORT_PTR->IDR)>>LCDII_Handle.LCDII_D0_D4_PIN); // Get IDR register and shift it by offset of D4 (first nibble) bit
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_E)); //Now set pin E low
  nibble_h = nibble_h << 4; // Shift MSB to the left where it belongs
  LCDII_Handle.LCDII_DELAY_PTR(E_PULSE_WIDTH_450nS); // Keep E low for  a few CPU cycles

  // Getting the lower bits nibble
  LCDII_Handle.LCDII_PORT_PTR->ODR |= (1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_E)); //Set LCD E pin high, then wait
  LCDII_Handle.LCDII_DELAY_PTR(E_PULSE_WIDTH_450nS); // Keep E high for a few CPU cycles
  nibble_l = (uint8_t)((LCDII_Handle.LCDII_PORT_PTR->IDR)>>LCDII_Handle.LCDII_D0_D4_PIN); // Get IDR register and shift IDR register by offset of D0
  LCDII_Handle.LCDII_PORT_PTR->ODR &= ~(uint32_t)(1 << (LCDII_Handle.LCDII_D0_D4_PIN + OFFSET_SIG_E)); //Now set pin E low
  LCDII_Handle.LCDII_DELAY_PTR(E_PULSE_WIDTH_450nS); // Keep E low for  a few CPU cycles
  byte = ((nibble_h & 0xF0) | (nibble_l & 0x0F));
  LCDII_Handle.LCDII_PORT_PTR->MODER |= (0x55U << LCDII_Handle.LCDII_D0_D4_PIN * 2); // write 01 (GP output) to 4 GPIOx data pins
  return byte;
}

uint8_t LCDII_Read_BF_Addr(LCDII_t LCDII_Handle) {
  return LCDII_Read(LCDII_Handle, READ_ADDR);
}

uint8_t LCDII_ReadData(LCDII_t LCDII_Handle){
  return LCDII_Read(LCDII_Handle, READ_DATA);
}


