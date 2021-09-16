# Network_programming_sockets_threads

Η εκφώνηση της εργασίας βρίσκεται [εδώ](https://github.com/KostasLiako/Network_programming_sockets_threads/blob/master/hw3-spring-2021.pdf)

```
# Εντολή Μεταγλώτισσης:
travelMonitorClient.c : 
gcc -o travelMonitorClient travelMonitorClient.c citizen.c list.c travelFunctions.c skipList.c
monitorServer.c : 
gcc -o monitorServer monitorServer.c citizen.c list.c monitorFunctions.c skipList.c -lpthread


# Εντολή Εκτέλεσης:
./travelMonitorClient -m <numberOfMonitor> -b <socketBufferSize> -c <cyclicBufferSize> -s <sizeOfBloomFilter> -I input_dir -t <numberOfThreads>


```


### TravelMonitorClient.c:

Ξεκινώντας το πρόγραμμα δημιουργώ έναν πίνακα από monitor[numMonitor] όπου θα αποθηκεύω τις πληροφορίες κάθε monitor.
Περνάω τις πληροφορίες για το ποιες χώρες θα διαχειρίζεται κάθε monitor , το port στο οποίο θα ακούει για συνδέσεις καθώς και ένα πίνακα με τα ορίσματα που θα δωθεί στην exec.
Η διαδικασία αυτη γίνεται στις συναρτήσεις passingCountriesIntoChilds() και passingArguments(). 
Στην συνέχεια καλεί την execv με ορίσματα των πίνακα που έχω δημιουργήσει για το κάθε monitor.

<br/>
### Connect στα monitorServer: 
Αφου δημιουργηθούν τα monitorServer (περιγράφω αναλυτικά παρακάτω ) το TravelMonitorClient περιμένει τα συνδεθεί (έχω βάλει μία sleep(3) έτσι ώστε να προλάβουν να δημιουργηθούν οι servers) με την gethostbyname βρισκω την public IP του δικτύου μου και κάνω connect στο port του αντίστοιχου monitorServer. 
Στην συνέχεια είναι έτοιμο να δεχθεί ερωτήματα. 

### monitorServer.c

Το πρώτο πράγμα που κάνω είναι να διαβάσω τα αρχεία των χωρών από τα path που μου έχει στείλει το travelMonitor μέσω threads από ένα κυκλικό buffer. Βασίστηκα στο παράδειγμα με των producer/consumer αλλά υλοποιώντας το με πολλούς consumers. 
Για να γίνει αυτό έχω δημιουργήσει ένα extra condition var και ένα extra mutex από αυτά του παραδείγματος .
Ο producer ξεκινάει να γεμίζει το κυκλικό buffer και παράλληλα στέλνει signal για να τα διαβάζουν οι consumers. 
Σε περίπτωση που γεμίσει περιμένει να διαβαστούν κάποια αρχεία μέχρι να βάλει τα υπόλοιπα. 
Μόλις κάποιο thread διαβάσει και το τελευταιό αρχείο στέλνει signal και κανει unlock to mutex για να τερματίσουν και τα υπόλοιπα threads.
<br />

### Δημιουργία Server:
Μόλις τερματίσουν επιτυχώς όλα τα threads (με χρήση της join) δημιουργώ τoν server που θα συνδεθεί το travelMonitorClient μόλις συνδεθεί κάνω fork. Tερματίζει το parent process και το παιδί είναι έτοιμο να δεχτεί ερωτήματα απο το travelMonitorClient. 
Για την υλοποίηση των ερωτημάτων χρησιμοποιώ την ίδια λογική με την εργασία 2 με την διαφορά ότι η επικοινωνία γίνεται πάνω απο socket και δεν κάνω trigger κάποιο monitor με κάποιο signal άλλα η πληροφορία στέλνεται σε όλα τα monitorServer και απαντάει το κατάλληλο.
