/****************************************************************
* Author:       Jeremy Atkins
* Date:         10/26/2020
* Purpose:      This program controls a lock and unlock system
                on a connected microcontroller. The pass code
                is a 4 digit code which is entered using a matrix
                key pad. LEDs light up with each key press and
                after the code has been entered, whether the 
                code was correct or not will be displayed on
                the connected LCD.
* Extra modules/functions in file:
*               c1isr   -   interrupt service routine for keypad
*               c2isr   -   interrupt service routine for keypad
*               c3isr   -   interrupt service routine for keypad
*               c4isr   -   interrupt service routine for keypad
* Assignment:   CSE 321 Project 2
*****************************************************************
* Inputs:       Matrix key pad buttons from the user
* Outputs:      The entered code and whether the code is correct
*               Colored LEDs corresponding to the user input
* Constraints:  The pass code is only four characters long and
                can not be changed from its default. Once the user
                begins entering the code, they can not go back.
* Source references:    UBLearns and provided course materials
*****************************************************************/
#define PASSLEN 5               // length of the pass code
#define BOUNCEDELAY 500000      // delay between keypad presses in milliseconds

#include "mbed.h"
#include "1802.h"

// ISRs for the keypad interrupts
void c1isr(void);
void c2isr(void);
void c3isr(void);
void c4isr(void);

// LCD constructor using PF_0 and PF_1 pins
CSE321_LCD lcd(16, 2, LCD_5x8DOTS, PF_0, PF_1);

int row = 0;                                // var to use to determine row
const char password [PASSLEN] = "0596";     // default correct password
char attempt[PASSLEN];                      // user entered password
int numChars = 0;                           // the number of characters the user has entered

// setup interrupt objects
InterruptIn int1 (PB_8, PullDown);
InterruptIn int2 (PB_9, PullDown);
InterruptIn int3 (PB_10, PullDown);
InterruptIn int4 (PB_11, PullDown);

int main()
{
    //RCC to enable clock for ports B, C, and E
    RCC->AHB2ENR|=0x16; 

    //MODER
    //GPIOB is the input
    GPIOB->MODER &= ~(0xFF0000);

    //GPIOC is the output
    GPIOC->MODER &= ~(0xAA0000);
    GPIOC->MODER |= 0x550000;

    //Enable output pins for LEDs
    GPIOE->MODER &= ~(0x2AA0);
    GPIOE->MODER |= 0x1550;

    // set up interrupt behavior
    // rise/fall and callback
    int1.rise(&c1isr);
    int2.rise(&c2isr);
    int3.rise(&c3isr);
    int4.rise(&c4isr);

    // enable interrupt requests
    int1.enable_irq();
    int2.enable_irq();
    int3.enable_irq();
    int4.enable_irq();

    // initialize lcd
    lcd.begin();
    lcd.print("Enter passcode:");
   
    // Main loop which controls the program. Loops forever
    while (true)
    {
        
        // Polling for the keypad input
        // Bottom row
        if (row == 0)
        {
            
            row = 1;
            GPIOC->ODR = 0x400;
            
        }
        // Row above bottom row
        else if (row == 1)
        {
            row = 2;
            GPIOC->ODR = 0x800;

        }
        // Row below top row
        else if (row == 2)
        {
            row = 3;
            GPIOC->ODR = 0x200;
        }
        // Top row
        else if (row == 3)
        {
            row = 0;
            GPIOC->ODR = 0x100;
        }
        
        // Checks how many characters have been entered
        if(numChars == 4)
        {
            attempt[PASSLEN-1]='\0';            // null terminator for the users input
            lcd.clear();                        // prepare to print to the lcd
            lcd.print(attempt);                 // display the users entered password
            numChars = 0;                       // reset the number of entered characters to 0

            // If the password is correct, display unlocked
            if(strcmp(attempt, password) == 0)
            {
                lcd.setCursor(0, 1);
                lcd.print("Unlocked!");
            }
            // If the password is incorrect, display locked
            else 
            {
                lcd.setCursor(0, 1);
                lcd.print("Locked!");
            }

            thread_sleep_for(5000);             // delay for the user to read the LCD
            lcd.clear();                        // prepare for next run
            lcd.print("Enter passcode:");       // prompt again for next attempt
            GPIOE->ODR &= 0x83;                 // reset LEDs to off


        }

        thread_sleep_for(50);                   // small delay before starting over
    }
    return 0;
}

/**
 *  void lightLEDs (int numEntered)
 *  Summary:
 *                  The lightLEDs function turns on or turns off LEDs
                    corresponding to the number of buttons the user
                    has pressed on the matrix key pad.
 *  Parameters:     numEntered  -   the number of buttons that have been
                                    pressed by the user
 *  Return Value:   Nothing
 *  Description:    Modifies the corresponding register values to light up
                    five different colored LEDs connected on the bread board
 */
void lightLEDs (int numEntered)
{
    GPIOE->ODR &= 0x83;             // make sure LEDs are off to begin

    // One number has been entered
    if(numEntered == 0)
    {
        GPIOE->ODR |= 0x4;          // turn on white LED
    }
    // Two numbers have been entered
    else if(numEntered == 1)
    {
        GPIOE->ODR |= 0x8;          // turn on yellow LED
    }
    // Three numbers have been entered
    else if(numEntered == 2)
    {
        GPIOE->ODR |= 0x10;         // turn on blue LED
    }
    // Four numbers have been entered
    else 
    {
        // Check if the password entered is correct
        // If correct, turn on the green LED
        // If incorrect, turn on the red LED
        if(strcmp(attempt, password) == 0)
            GPIOE->ODR |= 0x20;     // turn on green LED
        else 
            GPIOE->ODR |= 0x40;     // turn on red LED

    }
}

// ISR for *, 7, 4, 1
void c1isr(void)
{
    // Check which row and call lightLEDs to turn on correct LED
    if (row == 0)
    {
        attempt[numChars] = '*';
        lightLEDs(numChars);
    }
    else if (row == 1)
    {
        attempt[numChars] = '7';
        lightLEDs(numChars);
    }
    else if (row == 2)
    {
        attempt[numChars] = '4';
        lightLEDs(numChars);
    }
    else if (row == 3)
    {
        attempt[numChars] = '1';
        lightLEDs(numChars);
    }
    wait_us(BOUNCEDELAY); // Delay to deal with bounce
    numChars++;
}

//ISR for D, C, B, A
void c2isr(void)
{
    // Check which row and call lightLEDs to turn on correct LED
    if (row == 0)
    {
        attempt[numChars] = 'D';
        lightLEDs(numChars);
        
       // printf("D");
    }
    else if (row == 1)
    {
        attempt[numChars] = 'C';
        lightLEDs(numChars);
    }
    else if (row == 2)
    {
        attempt[numChars] = 'B';
        lightLEDs(numChars);
    }
    else if (row == 3)
    {
        attempt[numChars] = 'A';
        lightLEDs(numChars);
    }
    wait_us(BOUNCEDELAY); // Delay to deal with bounce
    numChars++;

}

void c3isr(void)
{
    // Check which row and call lightLEDs to turn on correct LED
    if (row == 0)
    {
        attempt[numChars] = '0';
        lightLEDs(numChars);
    }
    else if (row == 1)
    {
        attempt[numChars] = '8';
        lightLEDs(numChars);
    }
    else if (row == 2)
    {
        attempt[numChars] = '5';
        lightLEDs(numChars);
    }
    else if (row == 3)
    {
        attempt[numChars] = '2';
        lightLEDs(numChars);
    }
    wait_us(BOUNCEDELAY); // Delay to deal with bounce
    numChars++;
}

void c4isr(void)
{
    // Check which row and call lightLEDs to turn on correct LED
    if (row == 0)
    {
        attempt[numChars] = '#';
        lightLEDs(numChars);
    }
    else if (row == 1)
    {
        attempt[numChars] = '9';
        lightLEDs(numChars);
    }
    else if (row == 2)
    {
        attempt[numChars] = '6';
        lightLEDs(numChars);
    }
    else if (row == 3)
    {
        attempt[numChars] = '3';
        lightLEDs(numChars);
    }
    wait_us(BOUNCEDELAY); // Delay to deal with bounce
    numChars++;
}