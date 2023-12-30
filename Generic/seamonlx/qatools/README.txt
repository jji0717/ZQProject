


To build the bcast test app use make:

1) make clean
2) make


There are 2 additional bash scripts (tester.sh, killit.sh)
tester.sh allows you to start the bcast app. You pass 2 params
the path which is where the bcast  application exists,
and the server node to bind too.

cmd line example:  prompt> ./tester.sh -p . -n servername

a) this starts up a gnome-terminal and passes the correct bcast path and server to listen to.
b) when the server generates a message you should see aan XML message in the window.
c) you can start up several instances by running tester.sh again and again

d) finally you can call killit.sh with one param as follows:

	./killit.sh bcast


