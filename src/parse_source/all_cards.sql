create table Edition
(
  name                  varchar(40) character set utf8,
  code                  varchar(10) character set utf8,
  primary key (code),
  gathererCode          varchar(10) character set utf8,
  magicCardsInfoCode    varchar(10) character set utf8,
  releaseDate           datetime,
  border                varchar(10) character set utf8,
  type                  varchar(20) character set utf8,
  mkm_name              varchar(20) character set utf8,
  mkm_id                int
);

create table Card
(
  artist                varchar(40) character set utf8,
  cmc                   int,
  colorIdentity         set('W','R','G','U','B'),
  colors                varchar(20) character set utf8,
  flavor                varchar(1024) character set utf8,
  id                    binary(40),
  primary key (id),
  imageName             varchar(80) character set utf8,
  layout                varchar(20) character set utf8,
  manaCost              varchar(30) character set utf8,
  mciNumber             varchar(20) character set utf8,
  multiverseid          int,
  name                  varchar(70) character set utf8,
  power                 varchar(10) character set utf8,
  rarity                varchar(20) character set utf8,
  reserved              bool,
  subtypes              varchar(40) character set utf8,
  supertypes            varchar(40) character set utf8,
  text                  varchar(1024) character set utf8,
  toughness             varchar(10) character set utf8,
  type                  varchar(40) character set utf8,
  types                 varchar(40) character set utf8,
  edition_code          varchar(10) character set utf8,
  foreign key (edition_code) references Edition (code),
  datadate              datetime,
  pricedate             datetime,
  priceeuro             varchar(1024) character set utf8
);

