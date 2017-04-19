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
