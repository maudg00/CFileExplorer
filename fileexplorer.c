//
//  fileexplorer.c
//
//
//  Created by Mauricio de Garay on 25/01/2021.
//
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
/* Function prototypes. */

/*
* The function openFolder opens a folder, shows its contents on the window, and allows the user to navigate those contents with the up/down arrows,
* as well as allowing them to choose a file/directory with the enter key.
*
*
* @params
*   @param char * folderName: the name of the folder that you currently want to see.
*   @param char * lastFolder: the absolute route of the last navigated folder, used to keep track of the routes.
* @returns
*   char *: a string with the current chosen route which is obtained after the user has selected a file/directory. Can be "exit" if the user presses 'q' key.
*
*/
char * openFolder(char * folderName, char *lastFolder);
/*
* The function showFileInfo utilizes stat() to display the properties of a file (size, name, permissions, number of links and inodes).
*
*
* @params
*   @param char * filename: the route of the file which properties you wish to see.
* @returns
*   void
*
*/
void showFileInfo(char * filename);
int main()
{
    char *option;
    char lastFolder[PATH_MAX+1];
    //Since we will be freeing option after every iteration but we have to keep its value, we need a copy.
    char optionCopy[PATH_MAX+1];
    int i=0;
    //We obtain the absolute path to our starting directory and place it in the string lastFolder
    realpath(".", lastFolder);
    //We initialize the curses screen
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    refresh();
    //Main loop: keep the program running until the user wishes to exit.
    do
    {

        //In the first case, our parameter will be "." (current directory).
        if(i==0)
        {
            option=openFolder(".", lastFolder);
            strcpy(optionCopy, option);
        }
        //In every other case, our parameter will be the last chosen directory.
        else
        {
            option=openFolder(optionCopy, lastFolder);
            strcpy(optionCopy, option);
        }
        //Refresh screen with new contents
        refresh();
        //Increase i, just for the first-case validation
        i++;
        //free option. Since we are using return for strings, both option and the return value must be dinamically generated and freed.
        free(option);
    }
    while ((strcmp(optionCopy,"exit"))!=0);
    //End window.
    endwin();
}
char * openFolder(char * folderName, char *lastFolder)
{
    //Declare variables. nextFolder will be the returned value (chosen directory).
    char *nextFolder=malloc(PATH_MAX+1);
    DIR * openFolder;
    //Number of Files for cursor control
    int numFile=0;
    struct dirent *File;
    //Allow us to move around the screen and to control the option that is chosen.
    int y=1, arrow, flag=0, readType;
    openFolder=opendir(folderName);
    //if folder doesnt exist
    printw("Current directory: %s\n", lastFolder);
    if(openFolder==NULL)
    {
        printw("ERROR: couldn't open folder.\n");
        refresh();
        strcpy(nextFolder, ".");
        return nextFolder;
    }
    //If it does exist, we want to print out all of the contents in the screen.
    while(File=readdir(openFolder))
    {
        //Increase the number of files in this directory.
        numFile++;
        //Print content found
        printw("File %d. Name: %s \\NameSize: %hu \\", numFile, File->d_name, File->d_reclen);
        //Determine if it's a folder or a file
        switch(File->d_type)
        {
            case 4:
                printw("Type: Folder \n");
            break;
            case 8:
                printw("Type: File \n");
            break;
        }
        
    }
    printw("Press q to Exit, b to go back to starting folder.\n");
    //Center cursor in 2nd row (below the name of our current directory).
    move(y,0);
    refresh();
    //We've obtained every content, so we can close the directory now.
    closedir(openFolder);
    //React to up, down, b, q, and enter key presses.
    do
    {
        arrow=getch();
        switch(arrow)
        {
            //Temp will be a copy of the line printed out on the screen where the cursor if positioned, if the user presses enter.
            char temp[300];
            case KEY_DOWN:
                //only allow down key press if user is navigating folder contents.
                if(y<(numFile))
                {
                    y++;
                    move(y,0);
                }
                
            break;
            case KEY_UP:
                //only allow up key press if user is navigating folder contents.
                if(y>1)
                {
                    y--;
                    move(y,0);
                }
            break;
            //case when user presses enter
            case 10:
                flag=1;
                //obtain current line which is printed out on the screen where the cursor is positioned, and save it in temp.
                winstr(stdscr, temp);
                int i=0;
                //separate the string to obtain name and type.
                char *token=strtok(temp, ".");
                char *tokenName, *endTokenName;
                char Type[20];
                char realPath[PATH_MAX + 1]; 
                //Check every substring
                while(token!=NULL)
                {
                    //Check every substring. i=0 is name substring, i=1 namesize substring, i=2 type substring.
                    token=strtok(NULL, "\\");
                    switch(i)
                    {
                        //Obtain file/folder name
                        case 0:
                            //We have to separate this substring again into 2 substrings, the 2nd one will have the file/folder name.
                            nextFolder[0]=0;
                            tokenName=strtok_r(token, ":", &endTokenName);
                            tokenName=strtok_r(NULL, " ", &endTokenName);
                            //Obtain the absolute route of the chosen file/folder with the help of lastFolder by doing concat of lastFolder (starting route) and the name of the chosen file.
                            strcat(nextFolder, lastFolder);
                            strcat(nextFolder, "/");
                            strcat(nextFolder, tokenName);
                        break;
                        //Check if it's file or folder.
                        case 2:
                            //We have to separate this substring again into 2 substrings, the 2nd one will have the type.
                            tokenName=strtok_r(token, ":", &endTokenName);
                            tokenName=strtok_r(NULL, " ", &endTokenName);
                            strcpy(Type, tokenName);
                            printw("%s\n", Type);
                        break;
                    }
                    i++;
                }
                clear();
                //If the type is a file, we will show its info with the help of our showFileInfo(char * filename) function.
                //If it's a folder, we will return it so the main loop enters this function again but now in the chosen directory (switch folders).
                //If it's a file.
                if((strcmp(Type, "File"))==0)
                {
                    //Show chosen file's info.
                    showFileInfo(nextFolder);
                    //Stay in the same folder as we started out on.
                    strcpy(nextFolder, folderName);
                }
                //If it's a folder
                else
                {
                    //Make sure lastFolder changes value, nextFolder will return the chosen folder's route.
                    strcpy(lastFolder, nextFolder);
                }
                refresh();
            break;
            case 'q':
                //If he quits, flag becomes 1 to break out of loop.
                flag=1;
                //Make nextFolder="exit" so the program can end.
                strcpy(nextFolder, "exit");
            break;
            case 'b':
                //If he wants to go back to starting directory, flag becomes 1 to break out of loop.
                flag=1;
                //Make lastFolder and nextFolder which will be returned equal "." (current directory) absolute path.
                realpath(".", lastFolder);
                strcpy(nextFolder, lastFolder);
                clear();
            break;
        }
        refresh();
    } while (flag!=1);
    //return nextFolder
    return nextFolder;
}
void showFileInfo(char * filename)
{
    struct stat buffer;
    //Obtain stat data of this filename and save it into buffer.
    //If it doesnt exist, return.
    if(stat(filename, &buffer) < 0)    
        return;
    //Print file stat data.
    printw("Information for %s\n", filename);
    printw("---------------------------\n");
    printw("File Size: \t\t%d bytes\n", buffer.st_size);
    printw("Number of Links: \t%d\n", buffer.st_nlink);
    printw("File inode: \t\t%d\n", buffer.st_ino);
    printw("File Permissions: \t");
    printw( (S_ISDIR(buffer.st_mode)) ? "d" : "-");
    printw( (buffer.st_mode & S_IRUSR) ? "r" : "-");
    printw( (buffer.st_mode & S_IWUSR) ? "w" : "-");
    printw( (buffer.st_mode & S_IXUSR) ? "x" : "-");
    printw( (buffer.st_mode & S_IRGRP) ? "r" : "-");
    printw( (buffer.st_mode & S_IWGRP) ? "w" : "-");
    printw( (buffer.st_mode & S_IXGRP) ? "x" : "-");
    printw( (buffer.st_mode & S_IROTH) ? "r" : "-");
    printw( (buffer.st_mode & S_IWOTH) ? "w" : "-");
    printw( (buffer.st_mode & S_IXOTH) ? "x" : "-");
    printw("\nPress any key to continue... ");
    //Wait for a key press to clear screen and go back.
    getch();
    clear();
}