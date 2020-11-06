/*
* cs344 assignment 2 buildroom.c
* The file builds rooms for adventure game.
* Wei Huang
* 10/31/2019
* reference:
https://oregonstate.instructure.com/courses/1738955/pages/2-dot-1-c-structs
jackrwood(2018) cs344 Assignment-assignment2.github.com. Retrieved from 9 May 2018 from https://github.com/jackrwoods/CS344_Assignments/blob/master/assignment%202/woodjack.buildrooms.c
https://oregonstate.instructure.com/courses/1738955/pages/block-2
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

//struct contin rooms information
typedef struct Room Room;
struct Room{
  char* name;
  int outgoing[6];
  char* roomType;
};

//room's names
char* names[10]={
  "horse",
  "cat",
  "dog",
  "bird",
  "mouse",
  "rabbit",
  "fish",
  "tiger",
  "wolf",
  "lion"
};


Room* rooms[7];//room pointer
int ran[10];

//select the name from the list at random.
char* random_name() {
  	while(1) {
	    int NAME = rand() % 10;
	    if (ran[NAME] != 1) {
	      char* name = names[NAME];
	      ran[NAME] = 1;
	      return name;
    }
  }
}

//build the room type of the 7 from the 10
void build_type(){
  	int i;
   	for (i = 0; i < 7; i++) {

     rooms[i] = (Room*) malloc(sizeof(Room));

     rooms[i]->name = random_name();
      if(i==0){
      	rooms[i]->roomType = "START_ROOM";
      }
      else if(i==6){
      	 rooms[i]->roomType = "END_ROOM";
      }
      else{
      	rooms[i]->roomType ="MID_ROOM" ;
      }
        int j;
      for (j = 0; j < 6; j++) {
        rooms[i]->outgoing[j] = -1;
      }
    }
}
// make the room connect to eachother
 void ConnectRoom() {
 	build_type();
 	int i;
   int j;
   for (j = 0; j < 7; j++) {
     int numConnections = rand() % 4 + 3;
     for (i = 0; i < numConnections; i++) {
      int roomNum;
      do{
        roomNum = rand() % 7;
      }while (roomNum == j);

      int f = 0;
      int l = 0;
       while(l<6){
       if (rooms[j]->outgoing[l] == roomNum) {
         f = 1;
         break;
       }
       l++;
      }

      if (!f) {
        Room* A = rooms[j];
  		Room* B = rooms[roomNum];

  	//connect B to A
		  int locB=0;
		 while(locB<6) {
		    if (B->outgoing[locB] == -1) {
		      B->outgoing[locB] = j;
		      break;
		    }
		    locB++;
		  }

		  // Connect A to B
		  int locA=0;
		  while(locA<6) {
		    if (A->outgoing[locA] == -1) {
		      A->outgoing[locA] = roomNum;
		      break;
		    }
		    locA++;
		  }
      }
     }
   }
 }
// writing the room information in the directory
void makefile(){
  char dirName[50];
  int pid =getpid();
   sprintf(dirName, "huangwei.rooms.%ld", pid);
   mkdir(dirName, 0755);
   FILE *write;
   char file_info[256];
   char file_name[256];
   	memset(file_name, '\0', sizeof(file_name));
   	memset(file_info, '\0', sizeof(file_info));
   int i=0;
   // store room infomation into file
   for(i=0; i< 7; i++){
     // declear file_info
     memset(file_info, '\0', sizeof(file_info));
     sprintf(file_info, "ROOM NAME: %s\n", rooms[i]->name);


     int j=0;

     // store connection room
     while (rooms[i]->outgoing[j] > -1 && j < 6){

        sprintf(file_info + strlen(file_info), "CONNECTION %d: %s\n", j, rooms[rooms[i]->outgoing[j]]->name);
        j++;
     }

     // stroe room type
     sprintf(file_info + strlen(file_info), "ROOM TYPE: %s\n", rooms[i]->roomType);

     memset(file_name, '\0', sizeof(file_info));
     sprintf(file_name, "%s/%sroom", dirName, rooms[i]->name);


     // open differnt file and write info

     write = fopen(file_name, "w+");
     if (write == NULL){
       printf("faild to open file\n");
     }

     fprintf(write, "%s", file_info);
     fclose(write);
   }
  }
//main function that make all functions works together
int main(int argc, char* argv[])
{
 srand(time(NULL));
 int i;
  for (i = 0; i < 10; i++) {
    ran[i] = 0;
  }
  ConnectRoom();
  makefile();
	for (i = 0; i < 7; i++) {
  	  free(rooms[i]);
	  }

  return 0;
}
