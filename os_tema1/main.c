#include <stdio.h>
#include <string.h>

void function(char *path, char *filter);

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
            int recursive = 0;
            char *path;
            char *filter;

            if (argv[2] != NULL)
            {
                if (strstr(argv[2], "path"))
                {
                    path = strdup(argv[2]);
                    printf("path %s\n", path);
                }
                else if (strstr(argv[2], "recursive"))
                {
                    recursive = 1;
                }
                else
                {
                    filter = strdup(argv[2]);
                    printf("filter %s\n", filter);
                }
            }

            if (argv[3] != NULL)
            {
                if (strstr(argv[3], "path"))
                {
                    path = strdup(argv[3]);
                    printf("path %s\n", path);
                }
                else if (strstr(argv[3], "recursive"))
                {
                    recursive = 1;
                }
                else
                {
                    filter = strdup(argv[3]);
                    printf("filter %s\n", filter);
                }
            }

            if (argv[4] != NULL)
            {
                if (strstr(argv[4], "path"))
                {
                    path = strdup(argv[4]);
                    printf("path %s\n", path);
                }
                else if (strstr(argv[4], "recursive"))
                {
                    recursive = 1;
                }
                else
                {
                    filter = strdup(argv[4]);
                    printf("filter %s\n", filter);
                }
            }

//            if (recursive != NULL)
//            {
//                while()
//                {
//                    function(path, filter);
//                }
//           }
//            else
//            {
                function(path, filter);
//            }
        }
    }

    return 0;
}

void function(char *path, char *filter)
{
    if (path != NULL)
    {
        char command[100];
        strcpy(command, "cd ");
        strcat(command, "..");
        system(command);
    }
}
