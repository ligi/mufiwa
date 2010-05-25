/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as 
 * published by the Free Software Foundation; 
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * 2010 by Marcus -LiGi- Bueschleb
*/

#include <avr/interrupt.h>

#define CLK 20000000
#define USART_BAUDRATE 57600
#define BAUD_PRESCALE ((CLK / (USART_BAUDRATE * 16UL)) - 1)   //CLK/16/BaudRate-1
#define AVERAGE 10                                    

static int uart_putchar(char c);         
static int uart_putchar_esc(char c);         

void uart_init(void)                        
{
   
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);               

  UCSR0C = (0<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);   //Set 1 stop bit and 8-bit char size
   
  UBRR0L = BAUD_PRESCALE;
  UBRR0H = (BAUD_PRESCALE >> 8);               

}

void adc_init(void)
{

  ADCSRA = (1<<ADEN) | ((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0)) ; // Prescale value 128
  ADCSRA |= _BV(ADSC);   // Start initial ADC cycle
}

uint16_t adc_sample(uint8_t adc_input) //Thanks to SPARKFUn WiiTILT!
{
                     
  uint16_t temp, result=0;
  uint8_t k=0;

  ADMUX = adc_input;   
  
  for(k = 0; k < AVERAGE; k++)   //Read ADC Port
    {
      
      ADCSRA |= (1<<ADSC);
      while (ADCSRA & (1<<ADSC));
      
      temp = ADCL;
      temp |= ADCH<<8;
      
      result += temp;      
    }

  return result/AVERAGE;
}

int16_t main(void)
{
  DDRB =0xFF;

  DDRC = 0x01;
  adc_init();
  uart_init();

  sei();
  int loop=0;

  while(1)
    {
      // blink the led
      if ( ((loop++)&64)!=0)
	PORTB = 0xFF;
      else
	PORTB =0;

      PORTB|=2;


      int rest=0;
      int i=0;
      for (i=0;i<4;i++)
	{
	  int sample=adc_sample(5);
	  uart_putchar_esc(sample&0xFF);
	  rest|= (sample>>8)<< (i*2);
	}

      uart_putchar_esc(rest);

      uart_putchar('\r');


    }
  return(0);
}

static int uart_putchar_esc(char c)          {
  if (c!='\r')
    {
      if (c==42)
	{
	  uart_putchar(42);
	  uart_putchar(1);
	}
      else 
	uart_putchar(c);
    }
  else {
    uart_putchar(42);
    uart_putchar(0);
  }
  return 0;
}

static int uart_putchar(char c)         
{
  cli();

  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  sei();
  return 0;
} 
