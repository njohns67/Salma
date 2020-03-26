#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

using namespace std;

int main(){
    struct dirent *entry;
    string path = "./";
    int i, original = 0;
    DIR *dir;
    while(true){
        dir = opendir(path.c_str());
        i = 0;
        if (dir == NULL) {
            printf("Couldn't open directory\n");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL)
            i++;
        if(original == 0)
            original = i;
        else if(i > original){
            printf("Files have been added\nNumber is now %d\n", i);
            return 1;
        }
        closedir(dir);
    }
}