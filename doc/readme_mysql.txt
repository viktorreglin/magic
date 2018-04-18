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

Das Ganze funktioniert so nur mit MariaDB, nicht mit neueren Versionen von mysql.
Hier eine Installationsanleitung für Ubuntu:

Alles als Super-user (Kommando su):
(falls su nicht geht: mit sudo passwd root ein neues Passwort für root festlegen, dann sollte su funktionieren)

# mysql entfernen:
apt-get remove mysql-server
apt-get remove mysql-client

# MariaDB installieren
apt-get install mariadb-server
apt-get install mariadb-client

# plugin loeschen, damit man mariadb ohne Passwort benutzen kann
mysql –u root
> update mysql.user set plugin='' where user='root';
> flush privileges;
> quit
Ctrl-D

Nun sollte alles gehen ...

