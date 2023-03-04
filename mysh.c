#include "mysh.h"

int tokenMaker(char *input, char **tokens) {
  int x = 0;
  char *token = strtok(input, " ");
  while (token != NULL) {
    tokens[x++] = token;
    token = strtok(NULL, " ");
  }
  return x;
}

pid_t run_program(char **tokens, int num_tokens) {
  pid_t pid;
  int status;
  char *path = tokens[1];
  char *args[9]; 
  
  for(int i = 1; i < num_tokens; i++){
    args[i-1] = malloc(strlen(tokens[i]+1));
    strcpy(args[i-1],tokens[i]);
  }

  pid = fork();
  if (pid == 0) {
    if (path[0]== '/') {
      if (execv(args[0], args) == -1) {
        perror("ERROR: UNABLE TO RUN PROCESS");
      }
      exit(EXIT_FAILURE);
    } else {
      char cwd[256];
      getcwd(cwd, 256);
      char full_path[50];
      snprintf(full_path, sizeof(full_path), "%s/%s", cwd, args[0]);
      args[0] = full_path;
      //printf("%s\n",path);
      if (execv(args[0], args) == -1) {
        perror("ERROR: UNABLE TO RUN PROCESS");
      }
      exit(EXIT_FAILURE);
    }
  } else if (pid < 0) {
    printf("ERROR: FORK DIDNT FORK");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  for(int i = 0; i < num_tokens; i++){
    free(args[i]);
  }
  printf("The PID for the program is: %ld\n",(long) pid);
  return pid;
}

void changeDir(char path[50]) {
  int ch = chdir(path);
  if (ch != 0) {
    printf("Error: Unable to change directory\n");
  }
}

void cwd() {
  char cwd[256];
  getcwd(cwd, 256);
  printf("The current working directory is: %s\n", cwd);
}

void userInput(char *input, int maxLen, char *cwd) {
  printf("mysh-%s-#", cwd);
  fgets(input, maxLen, stdin);
  input[strcspn(input, "\n")] = 0;
}

void print_history(char **history, int *ticker) {
  for (int i = 0; i < *ticker; i++) {
    char hist_temp[256];
    strcpy(hist_temp,history[i]);
    char *hist_tok = strtok(hist_temp, " ");
    printf("%i : ", i + 1);
    while (hist_tok != NULL) {
      char word[20];
      strcpy(word, hist_tok);
      printf("%s ", word);
      hist_tok = strtok(NULL, " ");
    }
    printf("\n");
  }
}

void kill_process(pid_t pid) {
  if (kill(pid, SIGKILL) != 0) {
    perror("Error killing process");
  } else {
    printf("Process %d killed\n", pid);
  }
}

int command_exec(char **tokens, char **history, int *ticker, pid_t *new_pid,int num_tokens) {
  
  char command[15];
  strcpy(command, tokens[0]);
  for (int i = 0; i < strlen(command); i++) // LOWERCASES THE COMMAND
  {
    command[i] = tolower(command[i]);
  }

  if (strcmp(command, "cd") == 0) // IF: CHANGE DIRECTORY
  {
    char path[50];
    strcpy(path, tokens[1]);
    changeDir(path);
    return 0;
  } else if (strcmp(command, "exit") == 0) // IF: EXIT
  {
    return 1;
  }

  else if (strcmp(command, "history") == 0) // IF: HISTORY
  {
    if (strcmp(tokens[1], "-c") == 0) // CLEAR HISTORY
    {
      for (int i; i < *ticker; i++) {
        free(history[i]);
        history[i] = NULL;
      }
      *ticker = 0;
      return 0;
    } else {
      if (*ticker > 0){
        print_history(history, ticker);
      }
      else{
        printf("ERROR: No history to print\n");
      }
      return 0;
    }
  }

  else if (strcmp(command, "cwd") == 0) // IF: CWD
  {
    cwd();
    return 0;
  }

  else if (strcmp(command, "run") == 0) {
    *new_pid = run_program(tokens, num_tokens);
    printf("Double Check for PIDS: %ld\n", (long) *new_pid);
    return 0;
  }

  else if (strcmp(command, "kill") == 0) {
    int temp_pid;
    temp_pid = atoi(tokens[1]);
    kill_process(temp_pid);
    return 0;
  }
  
  printf("Error: Command not found, please try again\n");
  return 0;
}

int main() {
  char input[256];
  char *tokens[10];
  char *history[50];
  char cwd[256];
  int x = 0;
  int num_tokens;
  int ticker = 0;
  pid_t new_pid[10];

  while (x == 0) {
    if (ticker == 50) {
      printf("Maximum number of commands entered, please reload the shell\n");
      break;
    }
    getcwd(cwd, 256);
    userInput(input, 256, cwd);
    int i;
    for (i = 0; input[i] != '\0'; i++);
    history[ticker] = malloc(i + 1);
    strcpy(history[ticker], input);
    //printf("%i", i);
    num_tokens = tokenMaker(input, tokens);
    x = command_exec(tokens, history, &ticker, new_pid, num_tokens);
    //printf("%lu\n",(long) *new_pid);

    
    ticker++;
  }

  for (int i = 0; i < ticker; i++) {
    free(history[i]);
  }
  return 0;
}
