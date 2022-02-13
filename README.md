<div align="center">
  Calendar and To Do List
  <br />
  <a href="#about"><strong>Explore the docs »</strong></a>
  <br />
</div>

<details open="open">
<summary>Table of Contents</summary>

- [About](#about)
  - [Built With](#built-with)
- [Usage](#usage)
- [Contributing](#contributing)
- [Authors & contributors](#authors--contributors)
- [License](#license)

</details>

---

## About

The project was built for the course Programming System attended at Polytechnic University of Turin. 

The application is intended to let an user interact with a personal calendar, located in a third-party server, which must be compatible with the CalDAV standard protocol.

### Built With

- Qt

## Usage

### Launch

At first launch, the user will be asked some initial information:

- Principal user who signed-up in the server, in url form ("Principal user");
- Server to connect to, again as an url ("Host");
- Username, usually the e-mail address used to log in the server ("Username");
- Password set at sign-up ("Password");

### Calendar Choice

After inserting the requested data, the application will try to connect to the specified server, and if the operation ends with success, it'll ask the user to choose which existing calendar he/she wants to interact to. At this development stage of our application, as mentioned before, at least one personal calendar must exist in the server; if no calendar is found, the application won't give any chance of iteraction with the server, even if the main interface appears. If we choose a calendar, the application will redirect the user to the principal interface, with a possible waiting time required by the server, at the end of which all the events and tasks existing in the calendar will be displayed.

### Main interface

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

## Contributing

First off, thanks for taking the time to contribute! Contributions are what make the open-source community such an amazing place to learn, inspire, and create. Any contributions you make will benefit everybody else and are **greatly appreciated**.

The interface really needs a more expert person than us, so if you want to contribute feel free to contact us and eventually fork the project.

## Authors & contributors

For a full list of all authors and contributors, see [the contributors page](https://gitlab.com/DarthReca/calendar-to-do-list/-/project_members).

## License

This project is licensed under the **Apache Software License 2.0**.

See [LICENSE](LICENSE) for more information.
