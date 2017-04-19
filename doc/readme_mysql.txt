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

Neue Datenbank anlegen:
> mysql -u root
CREATE DATABASE magic1;

Solange kein MySql-User eingerichtet ist, geht nur 'root' für alle
Aktionen. Um '-u root' bei jedem mysql-Aufruf zu sparen, kann man
im eigenen home-Verzeichnis eine Datei  '.my.cnf' anlegen mit folgenden Inhalt:
[client]
user=root

Tabelle anlegen, Beispiel:
> mysql magic1 < table1_magic1.sql

table1_magic1.sql:

create table card
(
name          varchar(80),
pricecent     int,
edition       char(8),
colorIdentity varchar(15),
id            char(40),
manaCost      smallint,
types         varchar(40),
power         tinyint,
toughness     tinyint,
text          text,
constraint pk_id primary key (id)
);


