#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <fcntl.h>

#define NO_SECTION_TYPES 4

int parse_cmd = 0;
int findall_cmd = 0;

int recursive_arg = 0;
char path_arg[200];
char filter_arg[50];
char section_arg[100];
char line_arg[5];

int wrong_type = 0;
int parse_error = 0;
char error_str[40];
int section_error = 0;
char error_str_sect[40];

void list_cmd();
void print_dir_content(char* path);
void extract_path_arg(char* full_arg);
void extract_section_arg(char* full_arg);
void extract_line_arg(char* full_arg);
void analyze_file();
void find_file_permission(char* file_permission, struct stat file_stat);
void analyze_agrs(char *arg2, char *arg3, char *arg4);
void parse_sf();
void parse_err(char *err);
void extract_section();
void findall();

void print_from_test_root(char *full_path);

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
            list_cmd();
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
            list_cmd();
        }
        else if (strstr(argv[1], "extract"))
        {
            analyze_agrs(argv[2], argv[3],argv[4]);
            extract_section();
        }
    }

    return 0;
}

void list_cmd()
{
    if (path_arg != NULL)
    {
        if (chdir(path_arg) == 0)
        {
            printf("SUCCESS\n");

            char parent_path[200];
            getcwd(parent_path, sizeof(parent_path));

            print_dir_content(parent_path);
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

void print_dir_content(char* parent_path)
{
    DIR *dir;
    struct dirent *entry;
    char full_path[200];

    if (!(dir = opendir(parent_path)))
    {
        return;
    }
        
    while ((entry = readdir(dir)) != NULL) {
        strcpy(full_path, parent_path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        char full_path_cpy[200];    
        strcpy(full_path_cpy, full_path);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {                   
                print_from_test_root(full_path_cpy);
                if (recursive_arg == 1)
                {
                    print_dir_content(full_path);
                }
            }                            
        }
        else
        {
            print_from_test_root(full_path_cpy);
        }
    }

    closedir(dir);
}

void print_from_test_root(char *full_path)
{
    char file_permission[10];
    struct stat file_stat;
    if(stat(full_path, &file_stat) >= 0)
    {
       find_file_permission(file_permission, file_stat);
    }

    int found_root = 0;
    char *end_str;
    char *token = strtok_r(full_path, "/", &end_str);

    char element_name[100];
    char print_path[200];
    strcpy(print_path, "test_root");

    while (token != NULL)
    {
        if (strstr(token, "test_root"))
        {
            found_root = 1;
        } 
        else if (found_root == 1)
        {
            strcat(print_path, "/");
            strcat(print_path, token);

            strcpy(element_name, token);
        }
        
        token = strtok_r(NULL, "/", &end_str);
    }
    
    if (strstr(filter_arg, "name_starts_with"))
    {
        char start[40];
        strncpy(start, filter_arg + 17, strlen(filter_arg) - 17);

        if (strstr(element_name, start))
        {
            printf("%s\n", print_path);
        }
    }
    else if (strstr(filter_arg, "permissions"))
    {            
        char permission[10];
        strncpy(permission, filter_arg + 12, strlen(filter_arg) - 12);

        if (strstr(file_permission, permission))
        {
            printf("%s\n", print_path);
        }
    }
    else
    {
        printf("%s\n", print_path);
    }
}

void find_file_permission(char* file_permission, struct stat file_stat)
{
    strcpy(file_permission, (file_stat.st_mode & S_IRUSR) ? "r" : "-");
    strcat(file_permission, (file_stat.st_mode & S_IWUSR) ? "w" : "-");
    strcat(file_permission, (file_stat.st_mode & S_IXUSR) ? "x" : "-");
    strcat(file_permission, (file_stat.st_mode & S_IRGRP) ? "r" : "-");
    strcat(file_permission, (file_stat.st_mode & S_IWGRP) ? "w" : "-");
    strcat(file_permission, (file_stat.st_mode & S_IXGRP) ? "x" : "-");
    strcat(file_permission, (file_stat.st_mode & S_IROTH) ? "r" : "-");
    strcat(file_permission, (file_stat.st_mode & S_IWOTH) ? "w" : "-");
    strcat(file_permission, (file_stat.st_mode & S_IXOTH) ? "x" : "-");
}

void extract_path_arg(char* full_arg)
{
    strncpy(path_arg, full_arg + 5, strlen(full_arg) - 5);
}

void extract_section_arg(char* full_arg)
{
    strncpy(section_arg, full_arg + 8, strlen(full_arg) - 8);
}

void extract_line_arg(char* full_arg)
{
    strncpy(line_arg, full_arg + 5, strlen(full_arg) - 5);
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
            recursive_arg = 1;
        }
        else
        {
            strcpy(filter_arg, arg2);
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
            recursive_arg = 1;
        }
        else if (strstr(arg3, "section"))
        {
            extract_section_arg(arg3);
        }
        else
        {
            strcpy(filter_arg, arg3);
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
            recursive_arg = 1;
        }
        else if (strstr(arg4, "line"))
        {
            extract_line_arg(arg4);
        }
        else
        {
            strcpy(filter_arg, arg4);
        }
    }
}

void parse_sf()
{
    struct stat path_stat;
    stat(path_arg, &path_stat);

    if (S_ISREG(path_stat.st_mode))
    {
        int fd = open(path_arg, O_RDONLY);

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