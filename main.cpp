#include "mbed.h"

AnalogIn VRx (A0);
AnalogIn VRy (A1);
DigitalIn SW (D6, PullUp);
DigitalOut A (D0);
DigitalOut B (D1);
DigitalOut C (D4);
DigitalOut D (D7);
DigitalOut E (D8);
DigitalOut F (D9);
DigitalOut G (D10);
DigitalOut H (D11);
DigitalOut I (D12);
DigitalOut HIT (D13);
DigitalOut MISS (D14);
InterruptIn button (BUTTON1);

// I placed the BusOut at the start so it was accessible in all the classes
BusOut myleds(D0, D1, D4, D7, D8, D9, D10, D11, D12);

// This class is for the bombs in the game, it checks the joystix location against the location of the bomb and if they are the same the game ends
class bomb {
int bomb_;
int bombguess_;

public:

bomb (int n, int m){
    bomb_ = n;
    bombguess_ = m;
    if (bomb_ == bombguess_){
        printf("BOOM! you got blown up! Press the black button on the board if you wish to try again \n");
        while (true){
        myleds = 0;
        }
    }
}
};

// This semaphore needed to be placed before my check class as it is used in the class
Semaphore BattleshipHit;

// This class checks the joystix location against the battleship locations and flashes the green hit light if you selected a battleship. Also releases a semaphore to show the battleship found count
class check {
int battleship_;
int guess_;

public:
check (int p, int q){
    battleship_ = p;
    guess_ = q;
    if (battleship_ == guess_){
        printf("You hit a battleship!\n");
        HIT = true;
        thread_sleep_for(1000);
        HIT = false;
        // using this for loop you could easily change how many times it flashed
        for (int i=0; i<2; ++i){
        myleds = 511;
        thread_sleep_for(1000);
        myleds = 0;
        thread_sleep_for(1000);
        }
        BattleshipHit.release();
    }
}
};

// This class allows for the correct LED to light up depending on how you move the joystix
class joystix {
float x_direction_;
float y_direction_;
int y_;
public:
// y is passed by reference so it changes globally based on what this function does to it
joystix (float j, float k, int &y){
    x_direction_ = j;
    y_direction_ = k;
    // joystix right, left, up and down controls
    if (((x_direction_ == 1.00)&&(y_direction_ <= 1.00))){
      y = y + 1; 
    }
    if (((x_direction_ <= 0.10)&&(y_direction_ <= 1.00))){
      y = y - 1; 
    }
    if (((0 <= x_direction_ <= 1.00)&&(y_direction_ == 1.00))){
      y = y + 3; 
    }
    if (((0 <= x_direction_ <= 1.00)&&(y_direction_ <= 0.10))){
      y = y - 3; 
    }
    //These are for if you hit the edge of the LED grid. It causes you to circle back to the start of that row or column
    if ((y > 9)&&(y != 10)){
        y = 3;
    }
    if (y == 10){
        y = 7;
    }
    if ((y < 1)&&(y != 0)){
        y = 7;
    }
    if (y == 0){
        y = 3;
    }
    
}
};
// function that makes sure that the random values aren't the same as each other or zero
void checkRandom(int &w, int &r, int &t){
    if(t == 0){
        t = 1;
    }
    if((t == r)||(t == w)){
        t = 2;
        w = 6;
        r = 7;
    }
    if(w == 0){
        w = 1;
    }
    if(r == 0){
        r = 1;
    }
    if(w == r){
        w = 2;
        r = 7;
    }
}
// semaphore that is only released when a button is pressed
Semaphore start;

void onPress(){
    start.release();
}
// A side thread that keeps track of how many battleships have been found so it knows when to end the game
Thread sideThread;

void sidethreadFunction(){
    // v is the number of battleships sunk
    int v = 0;
    while(true){
        BattleshipHit.acquire();
        v = v + 1;
        if (v == 1){
            printf("1/2 battleships sunk \n");
        }
        if (v == 2){
            printf("You won! Press the black button on the board to restart the game  \n");
            while(true){
                myleds = 0;
            }
        }
    }
    
}
int main() {
    // x is battleship 1 location
    // z is battleship 2 location
    // u is bomb location
    int x = 0;
    int z = 0;
    int u = 0;
    printf("Press the blue button on the board to start! \n");
    button.rise(onPress);
    // randomises the values whilst the button is pressed so it is actually random
    while(button == true){
        x = rand() % 9;
        thread_sleep_for(5);
        z = rand() % 9;
        thread_sleep_for(5);
        u = rand() % 9;
        // this function prevents the battleships and the bombs from being zero or being the same as eachother
        checkRandom(x,z,u);
    }
    // when the button is being pressed all the LEDs turn on to indciate it is being pressed
    myleds = 511;
    sideThread.start(sidethreadFunction);
    start.acquire();
    // can uncomment these to see the locations of battleships/bombs, used for testing
    // printf("the value of x is %d\n", x); 
    // printf("the value of z is %d\n", z);
    // printf("the value of u is %d\n", u);
    // y is the joystix location
    int y = 1; 
    // an array was needed so the value of the location, y, could be converted into the values needed for the BusOut function to light up the corresponding LED
    int array[10] = {0,1,2,4,8,16,32,64,128,256};
    while(true){
        joystix movement(VRx,VRy,y);
        myleds = array[y];
        // can uncomment to check the joystix location is working
        //printf("the value of y is %d\n", y);
        // checks if you have hit anything when joystix button is pressed
        if (SW == false){
            check one(x, y);
            check two(z, y);
            bomb first(u, y);
            // This was originally in check class but needed to be elsewhere when multiple battleships where added or the LED would still flash red if one battleships was found
            if ((x != y)&&(z != y)&&(u != y)){
                printf("Nothing there! \n");
                MISS = true;
                thread_sleep_for(1000);
                MISS = false;
            }
            // ensures that you cannot hit the same battleship twice
            if(x == y){
                x = -1;
            }
            if(z == y){
                z = -1;
            }
        }
        // I had to add this in because the joystix movement was occuring to fast and it made the joystix too sensitive
        thread_sleep_for(500);
    }
}



