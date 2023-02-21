#!/bin/bash
# a sample comment

#echo Hello World!



mail -s "neue Anmeldung" -aFrom:Kitesforfuture\<benjamin@kitesforfuture.de\> "benjamin.kutschan@posteo.de" <<< "Der neue Nutzer namens $2 hat sich mit der Emailadresse $1 registriert."

