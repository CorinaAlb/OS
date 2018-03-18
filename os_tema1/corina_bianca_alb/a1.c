#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include <fcntl.h>

#define NO_SECTION_TYPES 4

int parse_cmd = 0;
int findall_cmd = 0;

int recursive = 0;
char path[200];
char filter[50];
char section[100];
char line[5];

int wrong_type = 0;
int parse_error = 0;
char error_str[40];
int section_error = 0;
char error_str_sect[40];

void print_to_file();
void extract_path_arg(char* full_arg);
void extract_section_arg(char* full_arg);
void extract_line_arg(char* full_arg);
void analyze_file();
void analyze_agrs(char *arg2, char *arg3, char *arg4);
void parse_sf();
void parse_err(char *err);
void extract_section();
void findall();

struct SF_header
{
    uint32_t magic;
    uint16_t header_size;
    uint32_t version;
    uint8_t no_of_sections;
};

struct SF_section_header
{
    uint8_t name[9];
    uint16_t sect_type;
    uint32_t sect_offset;
    uint32_t sect_size;

};

struct SF_header header;
struct SF_section_header section_header[17];

int section_types[] = {22, 41, 95, 55};

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
            analyze_agrs(argv[2], argv[3], argv[4]);
            print_to_file();
            analyze_file();
        }
        else if (strstr(argv[1], "parse"))
        {
            parse_cmd = 1;
            extract_path_arg(argv[2]);
            parse_sf();
        }
        else if (strstr(argv[1], "findall"))
        {
            findall_cmd = 1;
            extract_path_arg(argv[2]);
            print_to_file();
        }
        else if (strstr(argv[1], "extract"))
        {
            analyze_agrs(argv[2], argv[3],argv[4]);
            extract_section();
        }
    }

    return 0;
}

void print_to_file()
{
    if (path != NULL)
    {
        if (chdir(path) == 0)
        {
            printf("SUCCESS\n");

            char command[100];

            if (recursive == 1 || findall_cmd == 1)
            {
                // if recursivity enabled, print to file all elements
                // with the given parent path. Also print the permissions
                if (findall_cmd == 1)
                {
                    strcpy(command, "find -type f -printf '%M %p\r\n' ");
                }
                else
                {
                    strcpy(command, "find -printf '%M %p\r\n' ");
                }
            }
            else
            {
                // if recursivity disabled, print to file all elements
                // with the given path. Also print the permissions
                strcpy(command, "find -maxdepth 1 -printf '%M %p\r\n' ");
            }

            strcat(command, "> temp.txt");
            system(command);
        }
        else
        {
            printf("ERROR\ninvalid directory path");
            exit(1);
        }
    }
    else
    {
        printf("ERROR\ninvalid directory path");
        exit(1);
    }
}

void extract_path_arg(char* full_arg)
{
    strncpy(path, full_arg + 5, strlen(full_arg) - 5);
}

void extract_section_arg(char* full_arg)
{
    strncpy(section, full_arg + 8, strlen(full_arg) - 8);
}

void extract_line_arg(char* full_arg)
{
    strncpy(line, full_arg + 5, strlen(full_arg) - 5);
}

void analyze_agrs(char *arg2, char *arg3, char *arg4)
{
    if (arg2 != NULL)
    {
        if (strstr(arg2, "path"))
        {
            extract_path_arg(arg2);
        }
        else if (strstr(arg2, "recursive"))
        {
            recursive = 1;
        }
        else
        {
            strcpy(filter, arg2);
        }
    }

    if (arg3 != NULL)
    {
        if (strstr(arg3, "path"))
        {
            extract_path_arg(arg3);
        }
        else if (strstr(arg3, "recursive"))
        {
            recursive = 1;
        }
        else if (strstr(arg3, "section"))
        {
            extract_section_arg(arg3);
        }
        else
        {
            strcpy(filter, arg3);
        }
    }

    if (arg4 != NULL)
    {
        if (strstr(arg4, "path"))
        {
            extract_path_arg(arg4);
        }
        else if (strstr(arg4, "recursive"))
        {
            recursive = 1;
        }
        else if (strstr(arg4, "line"))
        {
            extract_line_arg(arg4);
        }
        else
        {
            strcpy(filter, arg4);
        }
    }
}

void analyze_file()
{
    int fd = open("temp.txt", O_RDONLY);
    char buffer[700000];
    int n = read(fd, buffer, 700000);

    // remove temp file
    close(fd);
    unlink("temp.txt");

    // tokenize lines
    char *end_str;
    char *token = strtok_r(buffer, "\n", &end_str);

    while (token != NULL && n > 0)
    {
        char element_name[200];

        // tokenize at space. element of form
        // drwxrwxr-x ./test_root/RXJTY

        char *end_line;
        char *permission_token = strtok_r(token, " ", &end_line);
        char *name_token = strtok_r(NULL, " ", &end_line);

        strcpy(element_name, path);
        strncat(element_name, name_token + 1, strlen(name_token) - 1);

        if (!strstr(element_name, "temp.txt") && strlen(name_token) > 2)
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

                    for (int i=0; i < strlen(start); i++)
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
                char permission[10];
                strncpy(permission, filter + 17, strlen(filter) - 17);

                if (strstr(permission_token, permission))
                {
                    printf("%s\n", element_name);
                }
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

void parse_sf()
{
    struct stat path_stat;
    stat(path, &path_stat);

    if (S_ISREG(path_stat.st_mode))
    {
        int fd = open(path, O_RDONLY);

        if (fd < 0)
        {
            printf("ERROR!\ninvalid file");
            exit(1);
        }
        else
        {
            long size_file = lseek(fd, 0, SEEK_END);

            lseek(fd, size_file - 2, SEEK_SET);
            if (read(fd, &header.magic, 4) < 0)
            {
                printf("Cannot read the magic");
                exit(1);
            }
            if (header.magic != 0x3462)
            {
                parse_err("magic");
            }

            lseek(fd, size_file - 4, SEEK_SET);
            if (read(fd, &header.header_size, 4) < 0)
            {
                printf("Cannot read the header size");
                exit(1);
            }

            lseek(fd, size_file - header.header_size, SEEK_SET);
            if (read(fd, &header.version, 8) < 0)
            {
                printf("Cannot read the version");
                exit(1);
            }
            if (header.version < 120 || header.version > 235)
            {
                parse_err("version");
            }

            int no_of_sections_offset = size_file - (header.header_size - 4);
            lseek(fd, no_of_sections_offset, SEEK_SET);
            if (read(fd, &header.no_of_sections, 2) < 0)
            {
                printf("Cannot read the no. of sections");
                exit(1);
            }
            if (header.no_of_sections < 8 || header.no_of_sections > 17)
            {
                parse_err("sect_nr");
            }

            int section_offset = no_of_sections_offset + 1;
            for (int i = 0; i < header.no_of_sections; i++)
            {
                lseek(fd, section_offset, SEEK_SET);
                if (read(fd, &section_header[i].name, 18) < 0)
                {
                    printf("Cannot read section name %d", i);
                    exit(1);
                }

                section_offset += 9;
                lseek(fd, section_offset, SEEK_SET);
                if (read(fd, &section_header[i].sect_type, 4) < 0)
                {
                    printf("Cannot read section type %d", i);
                    exit(1);
                }

                // if all section types are right so far
                // check for wrong section type in order to
                // update the error string
                if (wrong_type == 0)
                {
                    int check_type = 1;
                    for (int j=0; j < NO_SECTION_TYPES; j++)
                    {
                        if (section_types[j] == section_header[i].sect_type)
                        {
                            check_type = 0;
                            break;
                        }
                    }

                    if (check_type == 1)
                    {
                        wrong_type = 0;
                        parse_err("sect_types");
                    }
                }

                section_offset += 2;
                lseek(fd, section_offset, SEEK_SET);
                if (read(fd, &section_header[i].sect_offset, 8) < 0)
                {
                    printf("Cannot read section offset %d", i);
                    exit(1);
                }

                section_offset += 4;
                lseek(fd, section_offset, SEEK_SET);
                if (read(fd, &section_header[i].sect_size, 8) < 0)
                {
                    printf("Cannot read section size %d", i);
                    exit(1);
                }

                section_offset += 4;
            }
        }

        if (parse_cmd == 1)
        {
            if (parse_error == 1)
            {
                printf("%s", error_str);
            }
            else
            {
                printf("SUCCESS\n");
                printf("version=%d\n", header.version);
                printf("nr_sections=%d\n", header.no_of_sections);
                for (int i=0; i<header.no_of_sections; i++)
                {
                    printf("section%d: %s %d %d\n", i+1, section_header[i].name, section_header[i].sect_type, section_header[i].sect_size);
                }
            }
        }
    }
    else
    {
        printf("ERROR!\ninvalid file");
        exit(1);
    }
}

void parse_err(char *err)
{
    if (parse_error == 1)
    {
        strcat(error_str, "|");
    } else
    {
        strcpy(error_str, "ERROR\nwrong ");
        parse_error = 1;
    }
    strcat(error_str, err);
}

void extract_section()
{
    parse_sf();
    if (parse_error == 0)
    {
        printf("SUCESS\n");
    }
    else
    {
        printf("%s\n", error_str_sect);
    }
}
