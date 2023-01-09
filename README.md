# CSHEP Project MARCO COLONNA

in this git repository is contained the Marco's project for the Computer Science in High Energy Physics course

it is already cloned and compiled on a virtual machine

the virtual machine's IP is: 34.175.73.150

in the virtual machine you will find a "containers/IMAP22" directory;
in this directory there are 2 directories (Docker1 and Docker2) containing 2 Dockerfile;
  the first Dockerfile has been used to create the image "compile"
  the second Dockerfile has been used to create the image "execution"
in the IMAP22 directory there is also the cloned git repository, the code has been compiled in release mode with the container "compile"
the other files in the directory are the output files of the code, that has been run with the container "execution"

if one want to do again all the procedure he just has to (inside IMAP22):
for the compilation:

sudo su
docker run -v /home/marco_colonna111/containers/IMAP22/:/workspace/ -it compile bash
git clone https://github.com/MarcoColonna/CSHEP.git (not necessary if the repository is there already)
cd CSHEP
cmake -S . -B build_release -DCMAKE_BUILD_TYPE=Release
cmake --build build_release
exit

and for the execution:

docker run -v /home/marco_colonna111/containers/IMAP22/:/workspace/ -it execution bash
CSHEP/build_release/mandelbrot
exit

the content of the code is largely described in the comments.

thank you.

Marco
