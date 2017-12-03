# speech
Please make sure you have installed pocketsphinx-5prealpha and sphinxbase-5prealpha.

Replace your 'continuous.c' which is under /pocketsphinx-5prealpha/src/programs with I provided in pocketsphinx-5prealpha_needs.

Then run "gcc continuous.c -c".

if something wrong appeard, find what you lack and copy it to /usr/includes/

After you compiled successfully, run "make".

cd to /pocketsphinx-5prealpha and run "make" then run "sudo make install"
