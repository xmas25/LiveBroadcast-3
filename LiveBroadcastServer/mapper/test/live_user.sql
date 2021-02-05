DROP TABLE IF EXISTS `live_user`;

CREATE TABLE `live_user` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `user` varchar(20) NOT NULL,
  `passwd` char(32) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `user` (`user`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

LOCK TABLES `live_user` WRITE;

INSERT INTO `live_user` VALUES (1,'lsmg','123456789'),(2,'zs','123456'),(3,'ls','abc'),(4,'ww','123');

UNLOCK TABLES;
