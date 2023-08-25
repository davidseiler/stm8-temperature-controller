/* Memory mappings and constants for the STM8S003K3 */

// Macro function for calculating register locations
#define _SFR_(mem_addr)     (*(volatile uint8_t *)(0x5000 + (mem_addr)))

// Port A
#define PA_ODR      _SFR_(0x00)
#define PA_IDR      _SFR_(0x01)
#define PA_DDR      _SFR_(0x02)
#define PA_CR1      _SFR_(0x03)
#define PA_CR2      _SFR_(0x04)

// Port B
#define PB_ODR      _SFR_(0x05)  
#define PB_IDR      _SFR_(0x06)
#define PB_DDR      _SFR_(0x07)    
#define PB_CR1      _SFR_(0x08)     
#define PB_CR2      _SFR_(0x09)

// Port C
#define PC_ODR      _SFR_(0x0A)  
#define PC_IDR      _SFR_(0x0B)
#define PC_DDR      _SFR_(0x0C)
#define PC_CR1      _SFR_(0x0D)
#define PC_CR2      _SFR_(0x0E)

// Port D
#define PD_ODR      _SFR_(0x0F)
#define PD_IDR      _SFR_(0x10)
#define PD_DDR      _SFR_(0x11)
#define PD_CR1      _SFR_(0x12)
#define PD_CR2      _SFR_(0x13)

// Port E
#define PE_ODR      _SFR_(0x14)  
#define PE_IDR      _SFR_(0x15)
#define PE_DDR      _SFR_(0x16) 
#define PE_CR1      _SFR_(0x17)
#define PE_CR2      _SFR_(0x18)

// Port F
#define PF_ODR      _SFR_(0x19)
#define PF_IDR      _SFR_(0x1A)
#define PF_DDR      _SFR_(0x1B)
#define PF_CR1      _SFR_(0x1C)
#define PF_CR2      _SFR_(0x1D)

// UART
#define UART_SR     _SFR_(0x230)
#define UART_DR     _SFR_(0x231)
#define UART_BRR1   _SFR_(0x232)
#define UART_BRR2   _SFR_(0x233)
#define UART_CR1    _SFR_(0x234)
#define UART_CR2    _SFR_(0x235)

// TIM2 registers
#define TIM2_ARRH    _SFR_(0x30F)
#define TIM2_ARRL    _SFR_(0x310)
#define TIM2_CCR1H   _SFR_(0x311)
#define TIM2_CCR1L   _SFR_(0x312)

#define TIM2_CCMR1   _SFR_(0x307)
#define TIM2_CCER1   _SFR_(0x30A)
#define TIM2_PSCR    _SFR_(0x30E)
#define TIM2_IER     _SFR_(0x303)
#define TIM2_EGR     _SFR_(0x306)
#define TIM2_CR1     _SFR_(0x300)