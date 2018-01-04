# Gauntlet of Steel and Sorcery
A game where 2 challengers face-off in a clash of swords and magic. Different 
stats are assigned to the challengers to represent their physical and magical 
prowess. They take turns using their abilities in pursuance of victory over 
their adversary. The first to dwindle their opponent's life to zero is 
declared the champion.

## Note
To compile on FreeBSD, use `gmake` instead of `make`.
The executables `team1client` and `team1server` will be in the root directory.

## Directory Tree
<pre>
CMPS352
|
+-- client
|   +-- team1client.c
|
+-- common
|   |-- messages.h
|   +-- stats.h
|
+-- server
|   |-- connectionlist.c
|   |-- connectionlist.h
|   |-- database.c
|   |-- database.h
|   |-- gamestate.c
|   |-- gamestate.h
|   |-- statsserver.c
|   |-- statsserver.h
|   |-- subserver.c
|   |-- subserver.h
|   +-- team1server.c
|
+-- .gitignore
|
+-- Makefile
|
+-- README.md
</pre>

## File Descriptions
#### team1client.c:
Client will connect to server in order to play the game. They will be prompted 
to pick their attack (strike, fireball, heal). The client communicates with the 
server to send their attack and then recieve their updated stats. If one player 
reaches 0hp, then the game will end and the winner will be declared.

#### messages.h:
Contains the structs for sending a recieving attacks from the server and 
client. Allows the sending of both players current stats in one struct.

#### stats.h:
Defines the struct for the players stats which include hp, attack, defense, 
magic, and mana. Also defines the cost of using 'fireball' and 'heal'. 

#### gamestate.c:
Contains the game loop for the two clients and the server. It will loop until
at least one player has hp remaining. This loop has the responsibility of 
updating the clients and also checking to see if they used a valid attack. 
If a player disconnects they will automatically lose the game.

#### gamestate.h:
Defines the struct of GameState. This contains the players stats and their socket
descriptor which is used to communicate.

#### statsserver.c:
Contains the generation of the players stats and the function of each of the 
attacks. Each stat will be randomly generated for each player. The attack
strike will damage the opponent with no cost to the user. The attack fireball
will deal damage to the opponent but will cost the user's mana. The attack
heal will raise the casting player's hp but will cost the user's mana. If
the user does not have enough mana to cast fireball or heal, then they will
only be dealing or healing 1 hp.

#### statsserver.h:
Defines the functions of the three attacks, which are used to change the
hp of the two players. Also defines the function to generate each players stats.

#### subserver.c:
Responsible for updating and initializing the client thread. 

#### subserver.h:
Defines the struct ClientThreadArgs which is used to hold the socket descriptor,
the stats of both the attacker and defender, and the index of the attack.

#### team1server.c:
Makes a socket and then binds it to the server. The server then listens for
incomming connections. Then a connection will be accepted on a new socket
descriptor. Once there has been two connections, the subserver will start
and will launch the thread for the game loop. Once both players are done
playing, the server will then go back to accepting connections to play the
next game. 

#### Makefile:
Use the command `make` when in the CMPS352 directory in order to compile and link 
the program correctly. It will create two executables in the CMPS352 directory,
`team1server` and `team1client`, which will be executed by using the commands
`./team1server` and `./team1client`.

#### database.c:
Functions used for the database. These player records will be maintained in binary files and will persist between server restarts. Contains functions for updating, adding, and maintaining the player records.

#### database.h:
Defines the functions and structs used for the database for persisting the player records.

#### connectionlist.c
Functions for the connection list. This includes creation, manipulation, and deleting of connections. This also includes the mutex lock and unlock functions for maintaining the connection list.

#### connectionlist.h
Defines the functions and structs used for the connection list for all the players/spectators.