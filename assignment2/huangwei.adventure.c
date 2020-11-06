/*
* cs344 assignment 2 buildroom.c
* The file builds rooms for adventure game.
* Wei Huang
* 11/1/2019
* reference:
https://oregonstate.instructure.com/courses/1738955/pages/2-dot-1-c-structs
jackrwood(2018) cs344 Assignment-assignment2.github.com. Retrieved from 9 May 2018 from https://github.com/jackrwoods/CS344_Assignments/blob/master/assignment%202/woodjack.buildrooms.c
https://oregonstate.instructure.com/courses/1738955/pages/block-2
https://www.thegeekstuff.com/2012/05/c-mutex-examples/?refcom
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_t tid;
typedef int bool;
enum { false, true };

//array
int type_array[3];
int room_connect[7][7];
int room_array[7];
int step_info[100];
int stepNum = 0;


// the room names
char* name[10] = {
"horse",
"cat",
"dog",
"bird",
"mouse",
"rabbit",
"fish",
"tiger",
"wolf",
"lion"};

//room type
char* room_type[3] = {
  "START_ROOM",
  "MID_ROOM",
  "END_ROOM"};


  // room array
  void set_roomarray(){
  	int i=0;
  	for (i=0; i<7; i++){
  		room_array[i] = -1;
  	}

  }

  //type array
  void set_typearray(){
  	int i=0;
  	for (i=0; i<7; i++){
  		type_array[i] = -1;
  	}
  }

  // init step info array
  void set_stepinfo(){
  	int i=0;
  	for (i=0; i<100; i++){
  		step_info[i] = -1;
  	}
  }


  //  init 2d room connection array
  void set_connect(){
  	int i = 0;
  	int j = 0;

  	for (i=0;i<7;i++){
  		for (j=0; j< 7; j++ ){
  			room_connect[i][j] = -1;
  		}
  	}
  }



// read the room array information
void loadROOM(){
  DIR* dirname = opendir(".");
  struct dirent *de;
  struct stat sub;

	char dir_name[256];
	memset(dir_name,'\0', sizeof(dir_name));

  int recent_time = -1;
	char recent_dir_name[256];
	char first_char[50] = "huangwei.rooms.";
	memset(recent_dir_name,'\0', sizeof(recent_dir_name));

	if (dirname > 0){
		while ((de = readdir(dirname)) != NULL){
			if (strstr(de->d_name, first_char) != NULL){

				stat(de -> d_name, &sub);// get attributes

				if ((int)sub.st_mtime > recent_time) {
					recent_time = (int)sub.st_mtime;
					memset(recent_dir_name, '\0', sizeof(recent_dir_name));
					strcpy(recent_dir_name, de->d_name);
				}
			}
		}
	}

	closedir(dirname);
	memset(dir_name, '\0', sizeof(dir_name));
	strcpy(dir_name, recent_dir_name);
	char find_char[50] = "room";
	char room_name[256];
	memset(room_name, '\0', sizeof(room_name));

	int room_num = 0;
	dirname = opendir(dir_name);
	if (dirname > 0){
		while ((de = readdir(dirname)) != NULL){
			if (strstr(de->d_name, find_char) != NULL){
				memset(room_name, '\0', sizeof(room_name));// declear
				strcpy(room_name, de -> d_name);// copy

        char choose_path[50];
        memset(choose_path, '\0', sizeof(choose_path));
        sprintf(choose_path, "./%s/%s", dir_name, room_name);
        char name1[20];
        char name2[20];
        char value[20];
        int connection_num = 0;
        int val_encoded = -1;
        FILE *write;
        char *path = NULL;
        size_t len = 0;
        ssize_t read;
        write = fopen(choose_path, "r");

        while ((read = getline(&path, &len, write)) != -1){
          memset(name1, '\0', sizeof(name1));
          memset(name2, '\0', sizeof(name2));
          memset(value, '\0', sizeof(value));

          sscanf(path, "%s %s %s\n", name1, name2, value);

          int i=0;
          for (i=0; i<10; i++){
            if (strcmp(value, name[i]) == 0){
              val_encoded = i;
              break;

            }
          }

          int j=0;
          for (j = 0; j< 3; j++ ){
            if(strcmp(value, room_type[j])  == 0){
              val_encoded = j;
              break;
            }
          }
          if (strcmp(name1, "ROOM")==0 && strcmp(name2, "NAME:") == 0){
          //  int room array get val
            room_array[room_num] = val_encoded;

          }else if ( strcmp(name1, "CONNECTION") == 0){
            //connection
            room_connect[room_num][connection_num] = val_encoded;
            connection_num += 1;
          } else if ( strcmp(name1, "ROOM")==0 && strcmp(name2, "TYPE:") == 0){
            //type room
            type_array[room_num] = val_encoded;
          }
        }
        free(path);
        fclose(write);
				room_num += 1;// increase count
			}
		}
	}
	closedir(dirname);

}
//  get the time and writ the time
  void* getTime(void* get_time){
    pthread_mutex_lock(&mutex);
    char str[256];
    struct tm *info;
    time_t my_time;
    time(&my_time);
    info = localtime(&my_time);
    memset(str, '\0', sizeof(str));
    strftime(str, sizeof(str), "%l:%M%P, %A, %B %d, %Y", info);
    //open file writ
    FILE *fd = fopen("./currentTime.txt", "w+");
    fprintf(fd, "%s", str);
    fclose(fd);
    pthread_mutex_unlock(&mutex);
  }

  // this function to read time and print it
  void printTime(){
    char str[100];
    pthread_mutex_unlock(&mutex);
    pthread_join(tid, NULL);
    pthread_mutex_lock(&mutex);
    //open file read
    FILE *fd = fopen("./currentTime.txt", "r");
    memset(str, '\0', sizeof(str));
    fgets(str, sizeof(str), fd);
    fclose(fd);
    printf("\n%s\n\n", str);
  }


// find the start
int find_start(){
	int current_room = 0;
	while (type_array[current_room] != 0){
		current_room += 1;
	}
  int start_room = current_room;
	stepNum = 0;
	return start_room;
}

// user input and go to the next connect room
int CONNECT(int current_room){
	int input_room_num = -1;
	// ask user input

	printf("WHERE TO? >");

	char user_input[20];
	memset(user_input, '\0', sizeof(user_input));

	fgets(user_input, sizeof(user_input), stdin);
	user_input[strlen(user_input)-1] = '\0';
	// call get time function to write time
	if (strcmp(user_input, "time") == 0){
		printTime();
		return -1;
	}else{
		int i = 0;
		bool check_room_list = false;
		for (i = 0; i < 7; i++){
			if (strcmp(user_input, name[room_array[i]]) == 0){
				int j = 0;
				for (j=0; j< 7; j++ ){
					if (room_connect[current_room][j] == room_array[i]){
						input_room_num = i;
						check_room_list = true;
						break;
					}
				}
				break;
			}

		}
		// check user input
		if (check_room_list == true){
			printf("\n");
			return input_room_num;
		}else {
			printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
			return -1;
		}

	}
}

// this function to update info
int UPDATA(int current_room,int next_room){

	if (next_room != -1){
		step_info[stepNum] = next_room;
		stepNum += 1;
		return next_room;
	}else{
		return current_room;
	}



}

// this function is start game for user
void Game(){
  //set array before the game
  set_roomarray();
  set_typearray();
  set_connect();
  set_stepinfo();
  //loade the information of room
  loadROOM();

	// chek the current information
	int current_room = find_start(stepNum);
	int next_room = -1;

	// while if room type is not end room, game go on
	while (type_array[current_room] != 2){

    printf("CURRENT LOCATION: %s\n", name[room_array[current_room]]);
  	printf("POSSIBLE CONNECTIONS: ");

  	int i = 0;
  	bool get_connection = true;
  	for (i=0; i<7; i++ ){
  		if ( room_connect[current_room][i] != -1){
  			if (get_connection == true){
  				printf(" %s", name[room_connect[current_room][i]]);
  				get_connection = false;
  			}else {
  				printf(", %s", name[room_connect[current_room][i]]);
  			}
  		}
  	}
  	printf("\n");
		next_room = CONNECT(current_room);
		current_room = UPDATA(current_room, next_room);
	}

	if (type_array[current_room] == 2){
		printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
		printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepNum);

		int x = 0;
		for (x=0; x < stepNum; x++){
			printf("%s\n", name[room_array[step_info[x]]]);
		}

	}
}



int main(){
  int i;
	int err;
	if(pthread_mutex_init(&mutex, NULL) != 0){
		printf("cannot init mutex\n");
		return -1;
	}
	pthread_mutex_lock(&mutex);
	// call gettime function for time keeping
	err = pthread_create(&(tid), NULL, &getTime, NULL);
	if ( err != 0 ){
		printf("error for thread%s\n", strerror(err));
	}

	Game();
	//clear the pthread
	pthread_cancel(tid);
	pthread_mutex_destroy(&mutex);


	return 0;
}
