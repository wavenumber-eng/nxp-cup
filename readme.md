# Background

This repository constains a simple example of a zephyr application as a west manifest file.

Some background on Zephyr workspaces and west manifest files.

https://docs.zephyrproject.org/latest/develop/application/index.html

https://blog.golioth.io/improving-zephyr-project-structure-with-manifest-files/


# Setup

1.) Create a working folder on your local machine i.e. 'nxp-cup'


2.) cd into that folder and run:

```
west init -m https://github.com/wavenumber-eng/nxp-cup --mr main
```

This initializezes the folder as a west/zephyr workspace registered to our application repository.


3.)  Run "west update".   This may take several minutes to pull in all of the dependencies.

In this step,  west will look at the manifest and pull down all the dependencies.   In this case, the dependecy is the vanilla Zephyr repository.  
It is quite large and can take a few minutes but only has to be initialize once.  Future calls to west update are much quicker.


4.) open the folder your created in step 1 in vs code.

You will see another folder of the same name "projects/nxp-cup" which has a hello world folder.

Right click on the hello world folder an "open in integrated terminal"

Run the command 

`west build -b<board_name> --pristine`
