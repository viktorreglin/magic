Benutzung des Datenbanksystems MariaDB aka MySql
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Der Server muss laufen: /usr/sbin/mysqld
(In Suse: YaST/Diensteverwaltung)
Defaults stehen in /etc/my.cnf

Die Datenbanken sind Unterverzeichnisse von /var/lib/mysql und gehören
dem User mysql. In Suse geibt es diesen User, aber standardmäßig ohne
Login-Shell. Wenn man ihm eine gibt (auch Passwort geben !), kann man mittels
> su mysql
zu diesem werden und in das genannte Verzeichnis wechseln.

MySql-User != Linux-User. Standardmäßig gibt es nur 'root' als MySql-User.
In der Datenbank 'mysql' stehen die User; ansehen mit
> mysql -u root
SELECT Host, User, Password, plugin FROM mysql.user;
EXIT

Solange kein MySql-User eingerichtet ist, geht nur 'root' für alle
Aktionen. Um '-u root' bei jedem mysql-Aufruf zu sparen, kann man
im eigenen home-Verzeichnis eine Datei  '.my.cnf' anlegen mit folgenden Inhalt:
[client]
user=root

Alle weiteren mysql-Zugriffe (Datenbank anlegen, Tabellen anlegen, Werte eintragen/abfragen, ...)
erfolgen über query_db, siehe query_db_if.txt

