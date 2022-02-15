/**********************************************************************/
// ENGR-2350 Activity-Advenced Clock
// Name: Yijia Zhou
// RIN: 661995479
// This is the base project for several activities and labs throughout
// the course.  The outline provided below isn't necessarily *required*
// by a C program; however, this format is required within ENGR-2350
// to ease debugging/grading by the staff.
/**********************************************************************/

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.
#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void Timer_Init();
void GPIO_Init();
void Timer_ISR();
void Port4_ISR();
void UpdateTime();
void UpdateTime2();
// Add global variables here, as needed.

uint32_t array[4] = {0,0,0,0};
//uint32_t array[4] = {0,30,59,10};
uint16_t timer_reset_count = 0; // variable to count timer interrupts
Timer_A_UpModeConfig Timer;

int main(void) /* Main Function */
{
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    GPIO_Init();
    Timer_Init();



    while(1){
        // Place code that runs continuously in here
        if (timer_reset_count != 0){
//            UpdateTime();
            timer_reset_count = 0;
        }
    }
}

// Add function declarations here as needed
void GPIO_Init(){
    GPIO_setAsInputPin(GPIO_PORT_P1,GPIO_PIN1|GPIO_PIN4); // Set Launchpad input
    GPIO_setAsInputPin(GPIO_PORT_P4,GPIO_PIN0|GPIO_PIN2|GPIO_PIN3); // Set TI-RSLK input
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1,GPIO_PIN1|GPIO_PIN4); // Set PullUpResistor in Port 1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4,GPIO_PIN0|GPIO_PIN2|GPIO_PIN3); // Set PullUpResistor in Port 4

    GPIO_registerInterrupt(GPIO_PORT_P4,Port4_ISR);
    GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN2|GPIO_PIN3,GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN2|GPIO_PIN3);
}

void Timer_Init(){

//    Timer_A_configureUpMode(Timer_A1, Timer_A_UpModeConfig & Timer_A0_BASE);
    Timer.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    Timer.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    Timer.timerPeriod = 37500;
    Timer.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A0_BASE, &Timer);
    // Registering the ISR...
    Timer_A_registerInterrupt(TIMER_A0_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Timer_ISR);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}

void Port4_ISR(){
    __delay_cycles(240e3); // 10 ms delay (24 MHz clock)
    uint8_t active_pins = GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);
    if(active_pins & GPIO_PIN0){
        GPIO_clearInterruptFlag(GPIO_PORT_P4,GPIO_PIN0);
        if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0)){
            timer_reset_count++;
            array[3]++;
            UpdateTime2();
        }
    }
    if(active_pins & GPIO_PIN2){
        GPIO_clearInterruptFlag(GPIO_PORT_P4,GPIO_PIN2);
        if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2)){
            timer_reset_count++;
            array[2]++;
            UpdateTime2();
        }
    }
    if(active_pins & GPIO_PIN3){
        GPIO_clearInterruptFlag(GPIO_PORT_P4,GPIO_PIN3);
        if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN3)){
            timer_reset_count++;
            array[1]++;
            UpdateTime2();
        }
    }
}

void Timer_ISR(){
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);   // acknowledge the interrupt
    timer_reset_count++;                         // increment our counter
    UpdateTime();
}


void UpdateTime(){ // For Timer_A Interrupts
    array[0]++;  // Increment tenth of seconds
    if(array[0] == 10){  // If a whole second has passed...
        array[0] = 0;    // Reset tenth of seconds
        array[1]++;      // And increment seconds
        if(array[1] == 60){ // If a minute has passed...
            array[1] = 0;   // Reset seconds
            array[2]++;     // Increment minutes
            if(array[2] == 60){  // and so on...
                array[2] = 0;
                array[3]++;
                if(array[3] == 24){
                    array[3] = 0;
                }
            }
        }
    }
    printf("%2u:%02u:%02u.%u\r",array[3],array[2],array[1],array[0]);
}



void UpdateTime2(){ // For GPIO Interrupts
    if(array[0] == 10){  // If a whole second has passed...
            array[0] = 0;    // Reset tenth of seconds
            array[1]++;      // And increment seconds
    }
    if(array[1] == 60){ // If a minute has passed...
            array[1] = 0;   // Reset seconds
            array[2]++;     // Increment minutes
    }
    if(array[2] == 60){  // and so on...
            array[2] = 0;
            array[3]++;
    }
    if(array[3] == 24){
            array[3] = 0;
    }
    printf("%2u:%02u:%02u.%u\r",array[3],array[2],array[1],array[0]);
}

// Add interrupt functions last so they are easy to find
