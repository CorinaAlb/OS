#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

int recursive = 0;
char path[200];
char filter[50];

void list_function();
void extract_path(char* full_arg);

int main(int argc, char **argv)
{
    if(argc >= 2)
    {
        if(strstr(argv[1], "variant"))
        {
            printf("80090\n");
        }
        else if (strstr(argv[1], "list"))
        {
            if (argv[2] != NULL)
            {
                if (strstr(argv[2], "path"))
                {
                    extract_path(argv[2]);
                }
                else if (strstr(argv[2], "recursive"))
                {
                    recursive = 1;
                }
                else
                {
                    strcpy(filter, argv[2]);
                }
            }

            if (argv[3] != NULL)
            {
                if (strstr(argv[3], "path"))
                {
                    extract_path(argv[3]);
                }
                else if (strstr(argv[3], "recursive"))
                {
                    recursive = 1;
                }
                else
                {
                    strcpy(filter, argv[3]);
                }
            }

            if (argv[4] != NULL)
            {
                if (strstr(argv[4], "path"))
                {
                    extract_path(argv[4]);
                }
                else if (strstr(argv[4], "recursive"))
                {
                    recursive = 1;
                }
                else
                {
                    strcpy(filter, argv[4]);
                }
            }

            list_function(path, filter, recursive);
        }
    }

    return 0;
}

void list_function()
{
    if (path != NULL)
    {
        if (chdir(path) == 0)
        {
            char command[100];

            if (recursive == 1)
            {
                strcpy(command, "find ");
            }
            else
            {
                strcpy(command, "find -maxdepth 1 ");
            }

            strcat(command, "> temp.txt");
            system(command);

            int fd = open("temp.txt", O_RDONLY);
            char buffer[7000];
            int n = read(fd, buffer, 7000);

            unlink("temp.txt");

            char *end_str;
            char *token = strtok_r(buffer, "\n", &end_str);


            while (token != NULL && n> 0)
            {
                char element_name[150];

                if (strlen(token) == 1)
                {
                    strcpy(element_name, "SUCCESS");
                }
                else
                {
                    strcpy(element_name, path);
                    strncat(element_name, token + 1, strlen(token) - 1);
                }

                if (!strstr(element_name, "temp.txt"))
                {
                    if (strstr(filter, "name_starts_with="))
                    {
                        char start[40];
                        strncpy(start, filter + 17, strlen(filter) - 17);

                        char buffer_subdir[150];
                        strcpy(buffer_subdir, element_name);

                        char *end_token;
                        char *token_subdir = strtok_r(buffer_subdir, "/", &end_token);
                        char token_element[40];

                        while (token_subdir != NULL)
                        {
                            strcpy(token_element, token_subdir);
                            token_subdir = strtok_r(NULL, "/", &end_token);
                        }

                        if (token_element != NULL)
                        {
                            int starts_with = 1;

                            for (int i=0; i<strlen(start); i++)
                            {
                                if (start[i] != token_element[i])
                                {
                                    starts_with = 0;
                                }
                            }

                            if (starts_with == 1)
                            {
                                printf("%s\n", element_name);
                            }
                        }
                    }
                    else if (strstr(filter, "permissions="))
                    {
                        char permission[40];
                        strncpy(permission, filter + 12, strlen(filter) - 12);

                        printf("%s\n", permission);


                    }
                    else
                    {
                        printf("%s\n", element_name);
                    }
                }

                n = n -1 - strlen(token);

                token = strtok_r(NULL, "\n", &end_str);
            }
        }
        else
        {
            printf("ERROR\ninvalid directory path\n");
        }
    }
    else
    {
        printf("ERROR\ninvalid directory path\n");
    }


}

void extract_path(char* full_arg)
{
    strncpy(path, full_arg + 5, strlen(full_arg) - 5);
}
