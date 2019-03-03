#include <game.h>

int W,H;

int main() {
  // Operating system is a C program
  _ioe_init();
  init_screen();
  reset_game();
  unsigned long next_frame = 0;
  uint32_t reset_clock = 0;
  int keycode;
  while (1) {
  	while(uptime() < next_frame);
  	next_frame += 1000 / FPS;
  	if(left > 0 && chance > 0){
  		if((keycode = read_key()) != _KEY_NONE  
    		&& (!(keycode >> 15))){ //only keyup is detected
			check_letter(keycode);
			if(!left || !chance) {
				finish();
				reset_clock = uptime();
			}
    	}
  	}
    
    else if((!left || !chance) && uptime() - reset_clock > 1000){
    	reset_game();
    }
    redraw();
  }
  return 0;
}

char *wordset[] = {"operating","system","laboratory","hangman","game","deep","dark","fantasy",
"philosopher","atmosphere","utopia","congratulations","environmental",
"transformation"};
void reset_game(){
	chance = 7;
	srand(uptime());
	int random = rand()%14;
	strcpy(answer,wordset[random]);
	left = strlen(answer);
	init_gameboard();
}

static int letter_code[] = {
  _KEY_A, _KEY_B, _KEY_C, _KEY_D, _KEY_E, _KEY_F, _KEY_G,
  _KEY_H, _KEY_I, _KEY_J, _KEY_K, _KEY_L, _KEY_M, _KEY_N, _KEY_O, _KEY_P,
  _KEY_Q, _KEY_R, _KEY_S, _KEY_T, _KEY_U, _KEY_V,
  _KEY_W, _KEY_X, _KEY_Y, _KEY_Z,
};
void check_letter(int keycode){
	int i;
	char letter = ' ';
	for(i = 0;i < 26;i++){
		if(letter_code[i] == keycode){
			letter = 'a' + i;
			break;
		}
	}
	int found = 0;
	for(i = 0;i < strlen(answer);i++){
		if(answer[i] == letter){
			found = 1;
			answer[i] = '*'; //set guessed letter to *, in case of repeated key decreases "left" again
			draw_character(letter,8*i,0,WHITE,GBD);
			left--;
		}
	}
	if(!found){
		chance--;
		draw_character(letter,8*(12-chance),0,WHITE,USEDBD);
		update_hangman();
	}
}
