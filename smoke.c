#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>


#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__)
#else
#define VERBOSE_PRINT(S, ...) ((void) 0) // do nothing
#endif

int sum = 0;
uthread_cond_t match_and_paper;
uthread_cond_t paper_and_tobacco;
uthread_cond_t match_and_tobacco;
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};
int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked
bool hasPaper;
bool hasMatches;
bool hasTobacco;
int firstIterationFlag;


struct Agent {
  uthread_mutex_t mutex;
  uthread_cond_t  match;
  uthread_cond_t  paper;
  uthread_cond_t  tobacco;
  uthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create(agent->mutex);
  agent->match   = uthread_cond_create(agent->mutex);
  agent->tobacco = uthread_cond_create(agent->mutex);
  agent->smoke   = uthread_cond_create(agent->mutex);
  return agent;
}



//
// TODO
// You will probably need to add some procedures and struct etc.
//


void try_wake_up_smoker(int s,void* av){
    struct Agent* a = av;
    VERBOSE_PRINT("Sum = %d\n",sum);
  
          if(hasMatches && hasPaper){
      // wake up tabacco smoker
      VERBOSE_PRINT ("Wake up Tobacco smoker.\n");
      VERBOSE_PRINT ("Tobacco smoker is smoking.\n");
      uthread_cond_signal(a->smoke);
      smoke_count [TOBACCO]++;
              hasMatches = false;
              hasPaper = false;
              hasTobacco = false;
      
          }
    if(hasPaper && hasTobacco){
      // wake up match smoker
      VERBOSE_PRINT ("Wake up Match smoker.\n");
       VERBOSE_PRINT ("Match smoker is smoking.\n");
         uthread_cond_signal(a->smoke);
         smoke_count [MATCH]++;
       hasMatches = false;
       hasPaper = false;
       hasTobacco = false;
    }
    if (hasMatches && hasTobacco){
      // wake up paper smoker
      VERBOSE_PRINT ("Wake up Paper smoker.\n");
       VERBOSE_PRINT ("Paper smoker is smoking.\n");
         uthread_cond_signal(a->smoke);
         smoke_count [PAPER]++;
       hasMatches = false;
       hasPaper = false;
       hasTobacco = false;
    
    }
  
}


void* tobacco_listener (void* av){
  struct Agent* a = av;
    VERBOSE_PRINT ("In tobacco listener.\n");
  uthread_mutex_lock(a->mutex);
  while(1){
      VERBOSE_PRINT ("waiting for tabacco.\n");
    uthread_cond_wait(a->tobacco);
      VERBOSE_PRINT ("tabacco received .\n");
      VERBOSE_PRINT("Sum before tobacco = %d\n",sum);
      hasTobacco = true;
      VERBOSE_PRINT("Sum after tobacco = %d\n",sum);
      VERBOSE_PRINT ("about to call try wake up smoker .\n");
    try_wake_up_smoker(sum,av);
      VERBOSE_PRINT ("smoker finished .\n");

  }
  uthread_mutex_unlock(a->mutex);
}

void* paper_listener (void* av){
  struct Agent* a = av;
    VERBOSE_PRINT ("in paper listener.\n");
  uthread_mutex_lock(a->mutex);
  while(1){
      VERBOSE_PRINT ("waiting for paper.\n");
    uthread_cond_wait(a->paper);
      VERBOSE_PRINT ("received paper.\n");
      hasPaper = true;
      VERBOSE_PRINT ("trying for wake up smoker.\n");
    try_wake_up_smoker(sum,av);
      VERBOSE_PRINT ("back from wake up smoker.\n");

      
  }
  uthread_mutex_unlock(a->mutex);
}

void* match_listener (void* av){
  struct Agent* a = av;
    VERBOSE_PRINT ("in match listener.\n");
  uthread_mutex_lock(a->mutex);
  while(1){
      //uthread_cond_signal(a->smoke);
      VERBOSE_PRINT ("waiting for match.\n");

    uthread_cond_wait(a->match);
      VERBOSE_PRINT ("received match.\n");

      hasMatches = true;
      VERBOSE_PRINT ("trying for wake up smoker.\n");
    try_wake_up_smoker(sum,av);
      VERBOSE_PRINT ("back from wake up smoker.\n");
  }
  uthread_mutex_unlock(a->mutex);
}







/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */


// # of threads waiting for a signal. Used to ensure that the agent
// only signals once all other threads are ready.
int num_active_threads = 0;



/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can modify it if you like, but be sure that all it does
 * is choose 2 random resources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */

//void* agent (void* av) {
//  struct Agent* a = av;
//  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
//  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
//
//  srandom(time(NULL));
//
//  uthread_mutex_lock (a->mutex);
//  // Wait until all other threads are waiting for a signal
////  while (num_active_threads < 3)
////    uthread_cond_wait (a->smoke);
//
//  for (int i = 0; i < NUM_ITERATIONS; i++) {
//    int r = random() % 6;
//    switch(r) {
//    case 0:
//      signal_count[TOBACCO]++;
//      VERBOSE_PRINT ("match available\n");
//      uthread_cond_signal (a->match);
//      VERBOSE_PRINT ("paper available\n");
//      uthread_cond_signal (a->paper);
//      break;
//    case 1:
//      signal_count[PAPER]++;
//      VERBOSE_PRINT ("match available\n");
//      uthread_cond_signal (a->match);
//      VERBOSE_PRINT ("tobacco available\n");
//      uthread_cond_signal (a->tobacco);
//      break;
//    case 2:
//      signal_count[MATCH]++;
//      VERBOSE_PRINT ("paper available\n");
//      uthread_cond_signal (a->paper);
//      VERBOSE_PRINT ("tobacco available\n");
//      uthread_cond_signal (a->tobacco);
//      break;
//    case 3:
//      signal_count[TOBACCO]++;
//      VERBOSE_PRINT ("paper available\n");
//      uthread_cond_signal (a->paper);
//      VERBOSE_PRINT ("match available\n");
//      uthread_cond_signal (a->match);
//      break;
//    case 4:
//      signal_count[PAPER]++;
//      VERBOSE_PRINT ("tobacco available\n");
//      uthread_cond_signal (a->tobacco);
//      VERBOSE_PRINT ("match available\n");
//      uthread_cond_signal (a->match);
//      break;
//    case 5:
//      signal_count[MATCH]++;
//      VERBOSE_PRINT ("tobacco available\n");
//      uthread_cond_signal (a->tobacco);
//      VERBOSE_PRINT ("paper available\n");
//      uthread_cond_signal (a->paper);
//      break;
//    }
//    VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
//    uthread_cond_wait (a->smoke);
//  }
//
//  uthread_mutex_unlock (a->mutex);
//  return NULL;
//}


void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};

  srandom(time(NULL));

  VERBOSE_PRINT ("waiting for mutex");
  uthread_mutex_lock (a->mutex);
  // Wait until all other threads are waiting for a signal
    VERBOSE_PRINT ("before while loop\n");
    
    while (num_active_threads < 3){
        if(firstIterationFlag==0){
            firstIterationFlag =1;
            break;
        }
        uthread_cond_wait(a->smoke);
    }


  for (int i = 0; i < NUM_ITERATIONS; i++) {
      VERBOSE_PRINT ("ITERATION NUMBER ============= %d\n", i);

    int r = random() % 6;
    switch(r) {
    case 0:
      signal_count[TOBACCO]++;
      VERBOSE_PRINT ("match available\n");
      uthread_cond_signal (a->match);
      VERBOSE_PRINT ("paper available\n");
      uthread_cond_signal (a->paper);
      break;
    case 1:
      signal_count[PAPER]++;
      VERBOSE_PRINT ("match available\n");
      uthread_cond_signal (a->match);
      VERBOSE_PRINT ("tobacco available\n");
      uthread_cond_signal (a->tobacco);
      break;
    case 2:
      signal_count[MATCH]++;
      VERBOSE_PRINT ("paper available\n");
      uthread_cond_signal (a->paper);
      VERBOSE_PRINT ("tobacco available\n");
      uthread_cond_signal (a->tobacco);
      break;
    case 3:
      signal_count[TOBACCO]++;
      VERBOSE_PRINT ("paper available\n");
      uthread_cond_signal (a->paper);
      VERBOSE_PRINT ("match available\n");
      uthread_cond_signal (a->match);
      break;
    case 4:
      signal_count[PAPER]++;
      VERBOSE_PRINT ("tobacco available\n");
      uthread_cond_signal (a->tobacco);
      VERBOSE_PRINT ("match available\n");
      uthread_cond_signal (a->match);
      break;
    case 5:
      signal_count[MATCH]++;
      VERBOSE_PRINT ("tobacco available\n");
      uthread_cond_signal (a->tobacco);
      VERBOSE_PRINT ("paper available\n");
      uthread_cond_signal (a->paper);
      break;
    }
    VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
    uthread_cond_wait (a->smoke);
    
  }

  uthread_mutex_unlock (a->mutex);
  return NULL;
}

int main (int argc, char** argv) {
  
  struct Agent* a = createAgent();
  uthread_t agent_thread;

  uthread_init(4);
    
    VERBOSE_PRINT ("Before creates in main\n");
    hasMatches = false;
    hasPaper = false;
    hasTobacco = false;
    firstIterationFlag = 0;
  
     
  uthread_create (tobacco_listener, a);
  uthread_create (paper_listener, a);
  uthread_create (match_listener, a);
    
    int i = 0;
    while (i < 10000){
        i++;
    }
    agent_thread = uthread_create(agent, a);
    
    

 
//    uthread_detatch(match_listener);
//    uthread_detatch(tobacco_listener);
//    uthread_detatch(paper_listener);
  uthread_join(agent_thread, NULL);
    


  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);

  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);

  return 0;
}
