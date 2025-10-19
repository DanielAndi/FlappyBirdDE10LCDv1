#include <stdio.h>  // Add this for printf
#include "game_state.h"

int main(void) {
    game_state_t game_state;
    game_state_init(&game_state);
   
    printf("Game initialized successfully!\n");
    printf("Score: %d, Level: %d, Lives: %d\n", 
           game_state.score, game_state.level, game_state.lives);
    printf("High Score: %d\n", game_state.high_score);
    printf("Current Time: %d\n", game_state.current_time);
    printf("Total Time: %d\n", game_state.total_time);
    printf("Current Level: %d\n", game_state.current_level);
    printf("Total Levels: %d\n", game_state.total_levels);
    printf("Current Score: %d\n", game_state.current_score);
    printf("Total Score: %d\n", game_state.total_score);

    return 0;
}