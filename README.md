# GVFS Incremental Backup Utility

A simple, lightweight C utility for performing an incremental backup of a local directory to a destination that is mounted using GVFS (GNOME Virtual File System), such as Google Drive.

The script recursively scans a source directory and copies only new or modified files to the destination directory. This makes it efficient for regular backups to cloud storage.

***

## How It Works

The program compares files between a source and a destination directory based on their **modification times**.

1.  It recursively traverses the source directory.
2.  For each file, it checks if a corresponding file exists in the destination.
3.  If the destination file does not exist, or if the source file has a more recent modification timestamp, the program proceeds to copy it.
4.  Instead of using standard file I/O functions (which can be slow and unreliable with GVFS), it leverages the `gio copy` command-line tool. This command is specifically designed to handle file operations on virtual file systems, ensuring a more robust and efficient transfer.
5.  New directories in the source are automatically created in the destination.

**Note**: This utility does not handle file deletions. If you delete a file from the source, it will remain in the destination. It also ignores all hidden files and directories (those starting with a `.`).

***

## Requirements 📋

Before compiling and using this utility, make sure you have the following dependencies installed:

* **A C Compiler**: Such as GCC, which is used in the compilation instructions below.
* **GIO Command-Line Tool**: This is essential for the copy operation. If you don't have it, you can install it on Debian-based systems (like Ubuntu) with the following command:
    ```sh
    sudo apt-get update && sudo apt-get install libglib2.0-bin
    ```
* **A Mounted GVFS Drive**: You must have your Google Drive or other cloud storage mounted and accessible through your file system. The destination path will typically be located in `/run/user/<UID>/gvfs/`.

***

## Installation ⚙️

1.  Save the code to a file named `gdrive_backup.c`.
2.  Open your terminal and compile the program using GCC:

    ```sh
    gcc -o monodirectional_backup monodirectional_backup.c
    ```

    This command will create an executable file named `monodirectional_backup` in the same directory.

***

## Usage 🚀

To run the backup utility, execute the compiled program from your terminal, providing the source and destination directories as arguments.

### Syntax

```sh
./monodirectional_backup /path/to/your/source/directory /path/to/your/gvfs/destination
```
### Example

In this example, we back up the local ~/Documents folder to a Google Drive folder named "Backups".

``` sh
./monodirectional_backup ~/Documents /run/user/1000/gvfs/google-drive:host=example.com,user=someuser/0A...XYZ/Backups
```

The program will print the files it is copying to the console.

```
copying /home/user/Documents/report.docx > /run/user/1000/gvfs/.../Backups/report.docx ... DONE.
copying /home/user/Documents/project/data.csv > /run/user/1000/gvfs/.../Backups/project/data.csv ... DONE.
```
