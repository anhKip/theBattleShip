NEW TEST

#ifndef F_CPU
#define F_CPU 16000000UL    // Set 16 MHz clock speed
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <SevSeg.h>

#define DELAY_TIME 200
//#define USART_BAUDRATE 9600
//#define BAUD_PRESCALE ((((F_CPU / 16) + (USART_BAUDRATE / 2)) / (USART_BAUDRATE )) - 1)



volatile int player[2] = {0, 0};                // Row - Col
//volatile char gameMap[8][8] = {'\0'};
volatile int row, col;
volatile int received = 0;
volatile char playerMap[8][8] = {'\0'};
volatile int shots = 16;                        // Shots left
volatile int numberOfSunk = 0;
volatile int numberOfHit = 0;

volatile char firstDigitOfShots, secondDigitOfShots;

bool game_over();
void move_up(bool dir);
void move_down(bool dir);

bool orientation = true;

char gameMap[8][8] = {'0','0','0','0','0','0','0','0',
                      '0','1','1','0','0','0','0','0',
                      '0','0','0','0','0','1','1','0',
                      '0','0','0','0','0','0','0','0',
                      '0','0','1','1','0','0','0','0',
                      '0','0','0','0','0','0','0','0',
                      '1','1','0','0','1','1','0','0',
                      '0','0','0','0','0','0','0','0'};

int main()
{
    // OUTPUT -----------------------------------------
        // FEDCBA
    DDRD |= (1 << DDD7)|(1 << DDD6)|(1 << DDD5)|(1 << DDD4)|(1 << DDD3)|(1 << DDD1)|(1 << DDD0);
        // D5-D1
    DDRB |= (1 << DDB4)|(1 << DDB3)|(1 << DDB2)|(1 << DDB1)|(1 << DDB0);
        // HIT LED
    DDRB |= (1 << DDB5);

    // INPUT ------------------------------------------
        // DOWN/UP/XY Button 
    DDRC &= ~((1 << 2)|(1 << 1)|(1 << 0));
        // SHOOT Button - INT0
    DDRD &= ~(1 << 2);
    EICRA |= (1 << ISC01);          // Set INT0 trigger on falling-edge
    EIMSK |= (1 << INT0);           // Turns on INT0

//    // UART --------------------------------------------
//        // Setup the Baud Rate
//    UBRR0H = (BAUD_PRESCALE >> 8);  // Load upper 8-bits of the baud rate value into the high byte of the UBRR0H register
//    UBRR0L = BAUD_PRESCALE ;        // Load lower 8-bits of the baud rate value into the low byte of the UBRR0L register
//    
//        // Configure data format for transmission
//    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);     // Use 8-bit character sizes
//    UCSR0B = (1 << RXEN0) | (1 << TXEN0);       // Turn on the transmission and reception circuitry
//    
//        // Setup for interrupts
//    UCSR0B |= (1 << RXCIE0);        // Enable RX Complete Interrupt
    
    // Timer 1 for scanning rate of the LED compound
    DDRB |= (1 << 5);               // Set LED as output
    TCCR1B |= (1 << WGM12);         // Turn on the CTC mode for Timer 1
    TCCR1B |= (1 << CS11);          // Set up Timer 1 with the prescaler of 8
    OCR1A = 1999;                   // Set CTC compare value to 0.001s at 16 MHz AVR clock , with a prescaler of 8
    TIMSK1 = (1 << OCIE1A);         // Enable Output Compare A Match Interrupt
    sei();                          // Enable the Global Interrupt Bit

    while(1)
    {
        firstDigitOfShots = shots / 10;
        secondDigitOfShots = shots % 10;
        // Cloning Game Map
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
                playerMap[i][j] = gameMap[i][j];
                
        // UP Button - PORTC1
        if(!(PINC & (1 << 1))) 
        {
            _delay_ms(300);
            move_up(orientation);
        }
        
        // DOWN Button - PORTC2
        if(!(PINC & (1 << 2))) 
        {
            _delay_ms(300);
            move_down(orientation);
        }
        
        // X/Y Button - PORTC0
        if(!(PINC & (1 << 0))) 
        {
            _delay_ms(300);
            orientation = !orientation;
        }
    }
}

// MOVE PLAYER -------------------------------------------------------------
void move_up(bool dir) 
{
    if(dir)                             // x direction
    {      
        player[0]++;
        if (player[0] > 7) player[0] = 0;
    } 
    else                                // y direction
    {                                   
        player[1]++;
        if (player[1] > 7) player[1] = 0;
    }
}

void move_down(bool dir)
{
    if(dir)                             // x direction
    {       
        player[0]--;
        if (player[0] < 0) player[0] = 7;
    }
    else                                // y direction
    {                                   
        player[1]--;
        if (player[1] < 0) player[1] = 7;
    }
}

// Check for ships sunk
void check_ships()
{
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++) 
            if(playerMap[i][j] == 'x' && playerMap[i][j+1] == 'x')
            {
                playerMap[i][j] = 'h'; playerMap[i][j+1] = 'h';
                numberOfSunk++;
            }
}

/* 
 *  return false if missed, return true if hit
*/
bool hit() {
    bool result = false;
    int row = player[0];
    int col = player[1];

    //check if hit same position twice
    if(playerMap[row][col] == 'x' || playerMap[row][col] == 'm' || playerMap[row][col] == 'h')
    {
//        PORTB |= (1 << PORTB5);
//        _delay_ms(2000);
//        PORTB &= ~(1 << PORTB5);
    }

    else if(playerMap[row][col] == '1')
    {
        playerMap[row][col] = 'x';
        result = true;
        numberOfHit++;
        for (int i = 0; i < 3; i++)
        {
            PORTB |= (1 << PORTB5);
            _delay_ms(200);
            PORTB &= ~(1 << PORTB5);
        }
    } 
    else if(playerMap[row][col] == '0')
    {
        playerMap[row][col] = 'm';
    }

    return result;
}

/*
 * return true if shot the same position
 * return false if position never shot before
 */
bool check_same_position (int row, int col)
{
    bool same = false;

    if(playerMap[row][col] != '0')
    {
        PORTB |= (1 << PORTB5);       
        same = true;
    } 
    else same = false;

    return same;
}//end of check_same_position

//done, not?
bool game_over(){
    if(shots == 0) {
        return true;
    }

    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            if(playerMap[i][j] == '1')
                return true;
        }
    }
    return false;
}

void show_coordinate(bool dir)
{
    short coor = 0;
    if(dir) coor = player[0];
    else if(!dir) coor = player[1];
    show_number(coor);
}

// LED COMPOUND SCANNING --------------------------------------------------------------------------------
volatile char digitCount = 1, msCount = 0;
ISR(TIMER1_COMPA_vect)
{
    msCount++;
    if(msCount == 5) msCount = 0;
    off_digit(1);
    off_digit(2);
    off_digit(3);
    off_digit(4);
    off_digit(5);
    if(msCount < 4)
    {
        on_digit(digitCount);
        if(digitCount == 1) show_number(numberOfSunk);
        else if(digitCount == 2) show_number(numberOfHit);
        else if(digitCount == 3) show_number(firstDigitOfShots);
        else if(digitCount == 4) show_number(secondDigitOfShots);
        else if(digitCount == 5) show_coordinate(orientation);
        digitCount++;
        if(digitCount > 5) digitCount = 1;
    }
}

// SHOOT - PORTD2 -----------------------------------------------------------------------------------------
ISR(INT0_vect)
{
    shots--;
    hit();
}

//ISR (USART_RX_vect)
//{
//    char ReceivedByte ;         // Variable to store the data (1 byte) read from the register
//    ReceivedByte = UDR0;        // Read the received byte value
//
//    for (int i = 0; i < 8; i++)
//        for (int j = 0; j < 8; j++)  
//            game_map[i][j] = ReceivedByte;
//    
//    for (int i = 0; i < 8; i++)
//        for (int j = 0; j < 8; j++)
//        {
//            UDR0 = game_map[i][j];
//            UDR0 = '\n';
//        }
//}

// TURN ON NUMBERS -------------------------------------------------------------------------------------
void show_number(int number)
{
    switch(number)
    {
        case 0:
            PORTD &= ~((1 << 6)|(1 << 5)|(1 << 4)|(1 << 3)|(1 << 1)|(1 << 0));
            PORTD |= (1 << 7);
            break;  
        case 1:
            PORTD &= ~((1 << 3)|(1 << 1));
            PORTD |= (1 << 7)|(1 << 6)|(1 << 5)|(1 << 4)|(1 << 0);
            break;
        case 2:
            PORTD &= ~((1 << 7)|(1 << 5)|(1 << 4)|(1 << 1)|(1 << 0));
            PORTD |= (1 << 6)|(1 << 3);
            break;
        case 3:
            PORTD &= ~((1 << 7)|(1 << 4)|(1 << 3)|(1 << 1)|(1 << 0));
            PORTD |= (1 << 6)|(1 << 5);
            break;
        case 4:
            PORTD &= ~((1 << 7)|(1 << 6)|(1 << 3)|(1 << 1));
            PORTD |= (1 << 5)|(1 << 4)|(1 << 0);
            break;
        case 5:
            PORTD &= ~((1 << 7)|(1 << 6)|(1 << 4)|(1 << 3)|(1 << 0));
            PORTD |= (1 << 5)|(1 << 1);
            break;
        case 6:
            PORTD &= ~((1 << 7)|(1 << 6)|(1 << 5)|(1 << 4)|(1 << 3)|(1 << 0));
            PORTD |= (1 << 1);
            break;
        case 7:
            PORTD &= ~((1 << 3)|(1 << 1)|(1 << 0));
            PORTD |= (1 << 7)|(1 << 6)|(1 << 5)|(1 << 4);
            break;
        case 8:
            PORTD &= ~((1 << 7)|(1 << 6)|(1 << 5)|(1 << 4)|(1 << 3)|(1 << 1)|(1 << 0));
            break;
        case 9:
            PORTD &= ~((1 << 7)|(1 << 6)|(1 << 4)|(1 << 3)|(1 << 1)|(1 << 0));
            PORTD |= (1 << 5);
            break;
        case 10:
            PORTD |= (1 << 7)|(1 << 6)|(1 << 5)|(1 << 4)|(1 << 3)|(1 << 1)|(1 << 0);
            break;
    }
}

// TURN DIGIT ON/OFF --------------------------------------------------------------------------
void on_digit(int digit)
{
    switch(digit)
    {
        case 1:
        PORTB |= (1 << 0);break;
        case 2:
        PORTB |= (1 << 1);break;
        case 3:
        PORTB |= (1 << 2);break;
        case 4:
        PORTB |= (1 << 3);break;
        case 5:
        PORTB |= (1 << 4);break;
    }
}

void off_digit(int digit)
{
    switch(digit)
    {
        case 1: 
        PORTB &= ~(1 << 0);break;
        case 2: 
        PORTB &= ~(1 << 1);break;
        case 3: 
        PORTB &= ~(1 << 2);break;
        case 4: 
        PORTB &= ~(1 << 3);break;
        case 5: 
        PORTB &= ~(1 << 4);break;
    }
}
