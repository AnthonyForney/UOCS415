CS 415 Fall 2026
Project 1: Pseudo Shell
Author: Anthony Forney
Duck ID: 951962242
Date: 10.23.26

-------------------------------------
1. DESCRIPTION:

What I built is a low level command line interface shell that takes in the most basic linux commands like: cd, ls, and cat. Just like the name implies it's used by writing commands within the terminal to accomplish tasks. By inputting a command, it uses get line to read it, and then parses it with a filter to break it up from spaces or semicolons. It then uses strcmp to compare the inputted commands with the recognized ones written in main. Depending on which mode you activate the shell with - interactive or file - results in how you write your commands. Interactive lets you interact with files and directories, but file mode restricts you with reading into files only.

-------------------------------------
2. SUPPORTED COMMANDS:


    ls                  – displays alphabetically the files within your current path.
    pwd                 – displays what your current file path is.
    mkdir <name>        – creates a new directory named <name> within your current path
    cd <directory>      – allows you to change your current directory by putting a directory name in, ".." To go to your parent directory, "." To stay in your current directory, or leave it blank to go to the root directory.
    cp <src> <dst>      – allows you to copy and paste from your source to your destination, keeping the original file and writing a copy to your new destination.
    mv <src> <dst>      – allows you to cut and paste from your source to your destination. deleting the original file and writing a copy to your new destination.
    rm <filename>       – deletes a desired file by unlinking the pointer to it. Doing so removes it from your directory and is impossible to recover.
    cat <filename>      – reads out the contents from the selected file into your terminal.
    exit                – exits out of the shell, returning your to your regular terminal.

-------------------------------------
3. COMPILATION:


The shell is compiled by running the Makefile, that uses the "-o" flag that just specifies the output file. I run make clean; make , then ./pseudo-shell or ./pseudo-shell -f <filename>.


-------------------------------------
4. USAGE:

1. Interactive Mode

	interactive mode is started by inputting "./pseudo-shell" into the terminal. From there, ">>>" will appear in the text lines letting you know you're now in the interactive mode of the shell.
	>>>ls
	<filename> <filename> <filename> <directory>
	>>>cat <filename> ; ls ; pwd ;
	<file contents>
	<filename> <filename> <filename> <directory>
	root/dir/dir/dir
	>>exit or cmd c to quit interactive mode

2. File Mode

	
	file mode is activated by running "./pseudo-shell -f <filename>" from there, the shell will read the contents within the inputted file and output it in an overwritten or new txt file called output.txt, depending on the existence of an output.txt file. though, similar to interactive mode, file mode can only read commands. So if you tell it to read a file that has text not pertaining to the supported commands, the only thing you will receive in the output.txt is an error message.

If you try running file mode on a non existent file, you will receive an error message in the terminal.

