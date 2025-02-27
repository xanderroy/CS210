void parse(char* buffer, char** tokens);

void execute(char** command);

int checkSpecialCommands(char** command);

void getpath(char** command);

void setpath(char** command);

void returnPath();

void cd(char** command);

void alias(char** command);

void unalias(char** command);

int addAlias(char** command);

void printAliases();

typedef struct alias{
    char* command;
    char* alias;
} Alias;

int removeAlias(char** command);

void saveAliases();

void loadAliases();

#define MAX_ALIASES 10