/* ------------------------------------------
  	Ka Yi Cheng
	ECS642 Lab 6
	KL28Z Version

 Start with LEDs displaying Pattern A (red full on)
 Smoothly transition colors by adjusting brightness levels based on the sequence
 Timing control via B1 button
 Switch patterns via B2 button
  -------------------------------------------- */

#include <MKL28Z7.h>
#include <stdbool.h>
#include "../inc/SysTick.h"
#include "../inc/button.h"

#include "../inc/clock.h"
#include "../inc/lpit.h"
#include "../inc/TPMPWM.h"
#include "../inc/triColorLedPWM.h"
 
/* --------------------------------------------
  Variables for communication
*----------------------------------------------------------------------------*/
bool pressedB1_ev ;  // set by task1 (polling) and cleared by task 2

/*----------------------------------------------------------------------------
  task1pollB1
  
  This task polls button B1. 
*----------------------------------------------------------------------------*/
int b1State ;        // Current state - corresponds to position
int b1BounceCount ;

void initPollB1Task() {
    b1State = BOPEN ;
    pressedB1_ev = false ; 
    b1BounceCount = 0 ;
}

void pollB1Task() {
    if (b1BounceCount > 0) b1BounceCount -- ;
    switch (b1State) {
        case BOPEN:
            if (isPressed(B1)) {
                b1State = BCLOSED ;
                pressedB1_ev = true ; 
            }
          break ;

        case BCLOSED:
            if (!isPressed(B1)) {
                b1State = BBOUNCE ;
                b1BounceCount = BOUNCEDELAY ;
            }
            break ;

        case BBOUNCE:
            if (isPressed(B1)) {
                b1State = BCLOSED ;
            }
            else if (b1BounceCount == 0) {
                b1State = BOPEN ;
            }
            break ;
    }                
}

/* --------------------------------------------
  Variables for communication
*----------------------------------------------------------------------------*/
bool pressedB2_ev ; 

/*----------------------------------------------------------------------------
  task1pollB2
  
  This task polls button B2. 
*----------------------------------------------------------------------------*/
int b2State ; 
int b2BounceCount ;

void initPollB2Task() {
    b2State = BOPEN ;
    pressedB2_ev = false ; 
    b2BounceCount = 0 ;
}

void pollB2Task() {
    if (b2BounceCount > 0) b2BounceCount -- ;
    switch (b2State) {
        case BOPEN:
            if (isPressed(B2)) {
                b2State = BCLOSED ;
                pressedB2_ev = true ; 
            }
          break ;

        case BCLOSED:
            if (!isPressed(B2)) {
                b2State = BBOUNCE ;
                b2BounceCount = BOUNCEDELAY ;
            }
            break ;

        case BBOUNCE:
            if (isPressed(B2)) {
                b2State = BCLOSED ;
            }
            else if (b2BounceCount == 0) {
                b2State = BOPEN ;
            }
            break ;
    }                
}

/* -------------------------------------
    Global variables in transition
   ------------------------------------- */
#define MAXBRIGHTNESS 32
unsigned int redBrightness = MAXBRIGHTNESS;
unsigned int greenBrightness =0;
unsigned int blueBrightness =0;

/* -------------------------------------
    The time needed to complete the cycle (6 for each state and 32 for every level of the brightness)
   ------------------------------------- */
const uint32_t pitSlowCount = PITCLOCK * 9 / (6 * 32) ; // in 9 s
const uint32_t pitMediumCount = PITCLOCK * 5 / (6 * 32) ; // in 5 s
const uint32_t pitFastCount = PITCLOCK * 2 / (6 * 32) ; // in 2 s

/* -------------------------------------
    The states in pattern A
   ------------------------------------- */
enum ColorState {
    BlueInc,
    RedDec,
    GreenInc,
    BlueDec,
    RedInc,
    GreenDec
} colorState;

/* -------------------------------------
    The states in pattern B
   ------------------------------------- */

enum PatternBState {
    StateW,
    StateX,
    StateY,
    StateZ,
} patternBState;

/*----------------------------------------------------------------------------
   Task: togglePatternTask

   Toggle the pattern of upadtes to the LEDs on every signal
*----------------------------------------------------------------------------*/
bool patternB =false;
int patternState;  // this variable holds the current state

void togglePatternTask() {
    
  if (pressedB2_ev){
      patternB = !patternB;
    pressedB2_ev = false;
  }
}

/* -------------------------------------
    Transition of colours
   ------------------------------------- */

void LEDsInPattern(){
     if (patternB ==false) // in pattern A
				switch (colorState) {
            case BlueInc: // red in full, blue increase
                if (blueBrightness < MAXBRIGHTNESS) {
                    blueBrightness ++;
                }                    
                else colorState = RedDec;
                break;

            case RedDec: // blue in full, red decrease
                if (redBrightness > 0) {
                    redBrightness --;  
                }
                else colorState = GreenInc;  
                break;

            case GreenInc: // blue in full, green increase
                if (greenBrightness < MAXBRIGHTNESS) {
                    greenBrightness ++;  
                }
                else colorState = BlueDec; 
                break;

            case BlueDec: // green in full, blue decrease
                if (blueBrightness > 0) {
                    blueBrightness --; 
                }
                else colorState = RedInc;  
                break;

            case RedInc: // green in full, red increase
                if (redBrightness < MAXBRIGHTNESS) {
                    redBrightness ++;  
                }
                else colorState = GreenDec;  
                break;

            case GreenDec: // red in full, green decrease
                if (greenBrightness > 0) {
                    greenBrightness --;
                }
                else colorState = BlueInc;  
                break;
        }
     else { // in pattern B
        switch(patternBState){
              case StateW: // all off
                  if (redBrightness > 0 || greenBrightness > 0 || blueBrightness > 0) {
                      if (redBrightness > 0) {
                          redBrightness--; 
                      }
                      if (greenBrightness > 0) {
                          greenBrightness--; 
                      }
                      if (blueBrightness > 0) {
                          blueBrightness--; 
                      }
                  }
                  else patternBState = StateX; 
                  break;
            
              case StateX: // red and blue increse
                  if (redBrightness < MAXBRIGHTNESS && blueBrightness < MAXBRIGHTNESS) {
                      redBrightness++;
                      blueBrightness++;
                  }
                  else patternBState = StateY;
                  break;
            
              case StateY: // red in full, green increase ,blue decrease
                  if (greenBrightness < MAXBRIGHTNESS) {
                      greenBrightness++;
                  }
                  if (blueBrightness > 0) {
                      blueBrightness--; 
                  }
                  else patternBState = StateZ; 
                  break;
            
              case StateZ: // green in full, blue increase, red decrease
                  if (redBrightness > 0 && blueBrightness < MAXBRIGHTNESS) {
                      redBrightness--; 
                      blueBrightness++;
                  }
                  else patternBState = StateW; 
                  break;
           }
	}
        //update colour
        setLEDBrightness(Red, redBrightness);  
        setLEDBrightness(Green, greenBrightness);  
        setLEDBrightness(Blue, blueBrightness);  
}

/* -------------------------------------
    Programmable Interrupt Timer (PIT) interrupt handler

      Check each channel to see if caused interrupt
      Write 1 to TIF to reset interrupt flag

    Changes the LED brightness. 
   ------------------------------------- */

// ISR for handling PIT interrupt and updating LED brightness
void LPIT0_IRQHandler() {
    NVIC_ClearPendingIRQ(LPIT0_IRQn);

    // Check if PIT interrupt occurred on channel 0
    if (LPIT0->MSR & LPIT_MSR_TIF0_MASK) {
        togglePatternTask();
        LEDsInPattern();
    }


   // Clear all
   LPIT0->MSR = LPIT_MSR_TIF0(1) | LPIT_MSR_TIF1(1) | LPIT_MSR_TIF2(1)
             | LPIT_MSR_TIF3(1) ; // write to clear
}


/*----------------------------------------------------------------------------
   Task: toggleRateTask

   Toggle the rate of upadtes to the LEDs on every signal

   KEEP THIS TASK, but changes may be needed
*----------------------------------------------------------------------------*/

#define SLOW 0
#define MEDIUM 1
#define FAST 2

int rateState ; 

// initial state of task
void initToggleRateTask() {
    setTimer(0, pitSlowCount) ;
    rateState = SLOW ;
}

void toggleRateTask() {
    switch (rateState) {
        case FAST:  
            if (pressedB1_ev) {                   
                pressedB1_ev = false ;           
                setTimer(0, pitSlowCount) ; 
                rateState = SLOW ;           
            break ;
            }
        case SLOW:
            if (pressedB1_ev) {                  
                pressedB1_ev = false ;            
                setTimer(0, pitMediumCount) ;  
                rateState = MEDIUM ;          
            }
            break ;
						
	case MEDIUM:
	    if (pressedB1_ev){
		pressedB1_ev =false;
		setTimer(0, pitFastCount);
		rateState = FAST;
	    }
	    break ;
    }
    startTimer(0);
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
    // enable the peripheralclock
    enablePeripheralClock() ;

    // Configure pin multiplexing
    configureLEDforPWM() ;            // Configure LEDs for PWM control
  
    // Configure button B1
    configureButtons(B1, false) ; // ConfigureButtons B1 for polling
		configureButtons(B2, false) ;
  
    // Configure LPIT0 channel 0 to generate interrupts
    configureLPIT_interrupt(0) ;

    // Configure TPM
    configureTPMClock() ;    // clocks to all TPM modules
    configureTPM0forPWM() ;  // configure PWM on TPM0 (all LEDs)
   
    Init_SysTick(1000) ;  // initialse SysTick every 1 ms

    // start everything
    setLEDBrightness(Red, 0) ;
    setLEDBrightness(Green, 0) ;
    setLEDBrightness(Blue, 0) ;

    initPollB1Task() ;       // initialise task state
    initToggleRateTask() ;   // initialise task state
		
    initPollB2Task() ;
		
    // start the PIT
    startTimer(0) ;
    waitSysTickCounter(10) ;  // initialise delay counter
    while (1) {      // this runs forever
        pollB1Task() ;       // Poll button B1
				pollB2Task() ;
        toggleRateTask();    // Toggle LED update rate on every press signal
        // delay
        waitSysTickCounter(10) ;  // cycle every 10 ms 
    }
}
