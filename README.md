### Calendar and To Do List

## English version

# Introduction
Our application is intended to let an user interact with a personal calendar, located in a third-party server (e.g. NextCloud), which must be compatible with the CalDAV standard protocol. The user must select a server (even more than one), setting credentials to log in it and create one or more calendars; from this moment on, this application can be used to interact with them.

# Launch
At first launch, the user will be asked some initial information:
 - Principal user who signed-up in the server, in url form ("Principal user");
 - Server to connect to, again as an url ("Host");
 - Username, usually the e-mail address used to log in the server ("Username");
 - Password set at sign-up ("Password");

# Calendar Choice
After inserting the requested data, the application will try to connect to the specified server, and if the operation ends with success, it'll ask the user to choose which existing calendar he/she wants to interact to. At this development stage of our application, as mentioned before, at least one personal calendar must exist in the server; if no calendar is found, the application won't give any chance of iteraction with the server, even if the main interface appears. If we choose a calendar, the application will redirect the user to the principal interface, with a possible waiting time required by the server, at the end of which all the events and tasks existing in the calendar will be displayed.
 
# Main interface
Once a calendar is chosen and the user has waited for the server to do its job, the principal user interface will show, with its features:
- A principal calendar, in weekly format, in which all the events (red labels) and tasks (blue labels) are displayed, w.r.t. the hour of the day;
- A clickable calendar widget, in the left side, in monthly format, to select the visualization of the 7 days displayed in the main calendar; the selectd day will be the first to appear, starting from the left;
- A sidebar, in the right, to list all dateless tasks;
- A button, on the top left corner, used to issue the creation of a new event or task; if the user clicks on it, a form will pop up, with some fields to fill: a drop-down menu to choose whether the user wants to create an event or a task; in the first case, it'll ask for a title, starting and finishing date and hour, location and description; in the second one, the only difference is that date and hour are unique (a task has only the starting date).
- Some options, above the creation button:
    1. The first one lets the user choose the main calendar format, either weekly or daily; as well as that, he/she can choose to hide the dateless tasks sidebar, if not interested on them, in order to make the main calendar bigger.
	2. The second one is used to set a synchronization interval, namely the time after which the application will ask the server for changes; if they are actually present, it'll update its local displayed info. The user can also choose to force a synch.
	3. The third one lets the user change server or yet another calendar in within the same server. It results in being a very comfortable way to navigate through different personal calendars across different servers, without the need to search them one by one by means of a browser, for instance.
		
When creating a new event/task, it'll be visualized in the main calendar as a button and sent to the server (the user can check the existance of the cient-side newly created element connecting to the server through a browser). In order to update an element, just click on the corresponding button in the main calendar: a new form will appear, with the same fields of the creation one, but with the info of the clicked existing element; then just change the information in the fields you want to update and save, so that the updated element will be sent to the server.
As already mentioned, there's a synchronization interval; when it is up, the application will interrogate the server upon possible changes on the info stored in it and, if there actually are, it'll align with the server information.

Please note: if the user tries to interact with the calendar before the server gives the stored events and tasks, at launch time, some warning messages will appear, telling the user to wait for the server to do its job.

	 
 
## Versione italiana

# Presentazione
La nostra applicazione ha lo scopo di permettere all'utente l'interazione con un calendario personale, localizzato in un server di terze parti (come NextCloud), che sia compatibile con lo standard CalDAV. L'utente deve scegliere il server di riferimento (anche più di uno), effettuare un log-in al suo interno, impostando delle credenziali, e creare uno o più calendari; da quel momento questa applicazione permetterà l'interazione con essi.

# Lancio dell'applicazione
Al primo avvio verrà chiesto di inserire delle informazioni: 
 - L'utente principale che ha effettuato il sign-up nel server, sotto forma di url ("Principal user");
 - Il server al quale l'utente desidera collegarsi, sempre sotto forma di url ("Host");
 - Username dell'utente, in genere l'indirizzo mail con il quale ci si è loggati nel server ("Username");
 - Password scelta al momento del sign-up ("Password");

# Scelta del calendario
Una volta inseriti i dati richiesti, l'applicazione cercherà di connettersi con il server indicato e, se l'operazione andrà a buon fine, chiederà all'utente di scegliere con quale calendario preesistente interagire. A questo stadio di sviluppo, come menzionato in precedenza, l'utente deve aver già creato almeno un calendario; se questo non esiste ancora, l'applicazione non ci darà nessuna possibilità di interazione con il server, anche se dovesse presentarsi la schermata principale. Se abbiamo scelto un calendario, reindirizzerà l'utente all'interfaccia principale, con eventuali tempi di attesa dati dal server prima di visualizzare effettivamente gli eventi e le attività preesistenti nel calendario.

# Schermata principale
Una volta dentro, scelto un calendario e aspettato, si presenta l'interfaccia utente, con vari componenti:
- Un calendario principale, in formato settimanale, in cui vengono visualizzati gli eventi (in rosso) e le attività (in blu), per ogni ora del giorno;
- Un calendario cliccabile in formato mensile, sulla sinistra, da usare per impostare la visualizzazione dei giorni nel calendario principale; il giorno selezionato dall'utente sarà il primo dei 7 visualizzati nel principale, partendo da sinistra;
- Una barra laterale, sulla destra, che contiene la lista di attività prive di una specifica data e ora;
- Un pulsante, in alto a sinistra, usato per la creazione di un nuovo evento o attività; selezionandolo, comparirà un form con alcuni campi: l'utente può scegliere se creare un nuovo evento o una nuova attività da un menu a tendina; nel primo caso, chiederà di scegliere titolo, data e ora di inizio e fine, luogo e descrizione; nel secondo, l'unica differenza è che la data e l'ora saranno uniche (un'attività ha solo una data di inizio).
- Una serie di opzioni, in alto a sinistra:
	1. La prima permette di scegliere la visualizzazione del calendario principale, se in formato settimanale o giornaliero; si può inoltre scegliere di nascondere la barra laterale dei task senza data, in modo da ingrandire la visione del calendario principale.
	2. La seconda serve a impostare un intervallo di sincronizzazione con il server, che rappresenta il tempo dopo il quale l'applicazione chiederà i cambiamenti avvenuti nel server e, se presenti, aggiornerà la sua vista locale. Si può anche scegliere di forzare una sincronizzazione.
	3. La terza permette di scegliere un altro server a cui collegarsi o un altro calendario all'interno dello stesso server, in modo da poter comodamente navigare tra i diversi calendari personali di un utente, nello stesso server o tra vari server.

Creando un nuovo evento o attività, questo verrà visualizzato nel calendario principale sotto forma di pulsante e sarà inviato al server (l'utente può verificare l'esistenza del nuovo elemento collegandosi al server tramite browser). Per aggiornare un evento o attività esistente, è sufficiente premere il corrispondente pulsante nel calendario principale e cambiare le informazioni nel form che appare, che ricalca quello di creazione, ma ha già in sè le informazioni dell'elemento esistente. Le modifiche saranno ovviamente inviate al server.
Come già detto, viene impostato un intervallo di sincronizzazione, scaduto il quale l'applicazione interrogherà il server per sapere se sono avvenuti cambiamenti nel frattempo e, in caso affermativo, si allineerà col server.
 
NB: se cerchiamo di interagire con il calendario prima che l'applicazione abbia ottenuto dal server tutti gli eventi e le attività, al lancio dell'applicazione, compariranno messaggi di warning che dicono di aspettare che il server faccia il proprio dovere.
