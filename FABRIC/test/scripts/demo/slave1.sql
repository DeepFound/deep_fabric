CREATE DATABASE DeepDemo;
USE DeepDemo;
CREATE TABLE table1 (
  `id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  ROLE SLAVE,
  SERVER_ID 2
);

