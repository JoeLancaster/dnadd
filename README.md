# Declarative NixOS add

# dnadd(1) - Add nix packages declaratively on the command line


<a name="synposis"></a>

# Synposis

**dnadd [OPTION] ... [PACKAGE] ...**

<a name="description"></a>

# Description

dnadd is a utility that allows you to add packages to your Nix-based system in a classic command-line style while adhering to the declarative principles of NixOS. It does this by inserting the package name into your nominated .nix file and then (optionally) calling the "nixos-rebuild switch" command.

<a name="options"></a>

# Options


* **-c** **_PATH_**  
  Specify the path to the .nix file to edit.
* **-C** **_PATH_**  
  Set the persistent path to the .nix file
* **-t**   
  Text only mode. Do not call nixos-rebuild.
* **-q**  
  Only print the output of nixos-rebuild if it fails.
* **-h**  
  Displays a help message

<a name="setup-and-usage"></a>

# Setup and Usage


* 1.  
  Before first run, set the location of your .nix file that contains your packages with dnadd -C
* 2.  
  Within your .nix file inside your packages array you should write the comment #DNA# immediately above the first package. dnadd will use this to find where to place the packages and use the proper indentation.
  
The location of the persistent file used by dnadd and set with -C is located at ~/.config/.dna  

<a name="notes"></a>

# Notes


* 1.  
  dnadd updates your .nix file by creating a backup and swapping their names. A residual file with a .dna suffix will always be left as a backup.
* 2.  
  The path to your .nix file is resolved with realpath(3) which expands symbolic links. If, for example, your /etc/nixos/configuration.nix is a symlink for /home/alice/configuration.nix your backup file will be made in /home/alice
* 3.  
  dnadd reads the file at $(HOME)/.config/.dna. As you will likely be using dnadd with sudo or as root, you should make sure you set your location with dnadd -C as root user or sudo or manually in /root/.config/.dna

<a name="exit-status"></a>

# Exit Status

* **0** 
If ok

* **Non-zero** 
  dnadd exits with 1 (EXIT_FAILURE) if any error occurs or exits with nixos-rebuild's status code if nixos-rebuild does not exit successfully.

