#!/bin/bash
# a sample comment

#echo Hello World!



mail -s "Emailbestaetigung" -aFrom:Kitesforfuture\<benjamin@kitesforfuture.de\> $1 <<< "Bestaetige deine Email, indem du auf https://www.kitesforfuture.de/control/confirmEmail.php?user=$2&code=$3 klickst."

