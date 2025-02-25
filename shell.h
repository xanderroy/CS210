void parse(char* buffer, char** tokens);

void execute(char** command);

int checkSpecialCommands(char** command);

void getpath(char** command);

void setpath(char** command);

void returnPath();

void cd(char** command);

void history_add(char** command);

void history_print();

void delete_history();


