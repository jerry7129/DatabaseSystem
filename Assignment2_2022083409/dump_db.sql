-- MySQL dump 10.13  Distrib 8.4.6, for Win64 (x86_64)
--
-- Host: localhost    Database: library_db
-- ------------------------------------------------------
-- Server version	8.4.6

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `authors`
--

DROP TABLE IF EXISTS `authors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `authors` (
  `author_id` int NOT NULL AUTO_INCREMENT,
  `author_name` varchar(255) NOT NULL,
  PRIMARY KEY (`author_id`),
  UNIQUE KEY `author_name` (`author_name`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `authors`
--

LOCK TABLES `authors` WRITE;
/*!40000 ALTER TABLE `authors` DISABLE KEYS */;
INSERT INTO `authors` VALUES (3,'Agatha Christie'),(4,'Frank Herbert'),(2,'Isaac Asimov'),(1,'J.R.R Tolkien'),(5,'test_author'),(6,'testAuthor');
/*!40000 ALTER TABLE `authors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `book_authors`
--

DROP TABLE IF EXISTS `book_authors`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `book_authors` (
  `info_id` int NOT NULL,
  `author_id` int NOT NULL,
  PRIMARY KEY (`info_id`,`author_id`),
  KEY `author_id` (`author_id`),
  CONSTRAINT `book_authors_ibfk_1` FOREIGN KEY (`info_id`) REFERENCES `book_info` (`info_id`),
  CONSTRAINT `book_authors_ibfk_2` FOREIGN KEY (`author_id`) REFERENCES `authors` (`author_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `book_authors`
--

LOCK TABLES `book_authors` WRITE;
/*!40000 ALTER TABLE `book_authors` DISABLE KEYS */;
INSERT INTO `book_authors` VALUES (1,1),(2,1),(3,2),(4,3),(6,3),(5,4),(6,4);
/*!40000 ALTER TABLE `book_authors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `book_categories`
--

DROP TABLE IF EXISTS `book_categories`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `book_categories` (
  `info_id` int NOT NULL,
  `category_id` int NOT NULL,
  PRIMARY KEY (`info_id`,`category_id`),
  KEY `category_id` (`category_id`),
  CONSTRAINT `book_categories_ibfk_1` FOREIGN KEY (`info_id`) REFERENCES `book_info` (`info_id`),
  CONSTRAINT `book_categories_ibfk_2` FOREIGN KEY (`category_id`) REFERENCES `categories` (`category_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `book_categories`
--

LOCK TABLES `book_categories` WRITE;
/*!40000 ALTER TABLE `book_categories` DISABLE KEYS */;
INSERT INTO `book_categories` VALUES (1,1),(2,1),(5,1),(6,1),(3,2),(5,2),(4,3),(6,3);
/*!40000 ALTER TABLE `book_categories` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `book_info`
--

DROP TABLE IF EXISTS `book_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `book_info` (
  `info_id` int NOT NULL AUTO_INCREMENT,
  `book_title` varchar(255) NOT NULL,
  PRIMARY KEY (`info_id`),
  UNIQUE KEY `book_title` (`book_title`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `book_info`
--

LOCK TABLES `book_info` WRITE;
/*!40000 ALTER TABLE `book_info` DISABLE KEYS */;
INSERT INTO `book_info` VALUES (4,'And Then There Were None'),(5,'Dune'),(3,'Foundation'),(6,'TestBook'),(1,'The Hobbit'),(2,'The Lord of the Rings');
/*!40000 ALTER TABLE `book_info` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `books`
--

DROP TABLE IF EXISTS `books`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `books` (
  `book_id` int NOT NULL AUTO_INCREMENT,
  `info_id` int NOT NULL,
  `status` enum('Available','Checkout','Lost') NOT NULL DEFAULT 'Available',
  PRIMARY KEY (`book_id`),
  KEY `info_id` (`info_id`),
  CONSTRAINT `books_ibfk_1` FOREIGN KEY (`info_id`) REFERENCES `book_info` (`info_id`)
) ENGINE=InnoDB AUTO_INCREMENT=32 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `books`
--

LOCK TABLES `books` WRITE;
/*!40000 ALTER TABLE `books` DISABLE KEYS */;
INSERT INTO `books` VALUES (1,1,'Available'),(2,1,'Available'),(3,1,'Available'),(4,1,'Available'),(5,2,'Available'),(6,2,'Available'),(7,2,'Available'),(8,3,'Available'),(9,3,'Available'),(10,3,'Available'),(11,3,'Available'),(12,3,'Available'),(13,4,'Available'),(14,4,'Available'),(15,4,'Available'),(16,5,'Available'),(17,5,'Available'),(18,5,'Available'),(19,5,'Available'),(20,6,'Available'),(21,6,'Available'),(22,6,'Available'),(23,6,'Available'),(24,6,'Available');
/*!40000 ALTER TABLE `books` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `categories`
--

DROP TABLE IF EXISTS `categories`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `categories` (
  `category_id` int NOT NULL AUTO_INCREMENT,
  `category_name` varchar(255) NOT NULL,
  PRIMARY KEY (`category_id`),
  UNIQUE KEY `category_name` (`category_name`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `categories`
--

LOCK TABLES `categories` WRITE;
/*!40000 ALTER TABLE `categories` DISABLE KEYS */;
INSERT INTO `categories` VALUES (1,'Fantasy'),(4,'Happy'),(3,'Mystery'),(2,'Science Fiction');
/*!40000 ALTER TABLE `categories` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `checkout`
--

DROP TABLE IF EXISTS `checkout`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `checkout` (
  `checkout_id` int NOT NULL AUTO_INCREMENT,
  `user_id` varchar(255) NOT NULL,
  `book_id` int NOT NULL,
  `checkout_date` date NOT NULL,
  `due_date` date NOT NULL,
  `return_date` date DEFAULT NULL,
  `late_fee` int DEFAULT '0',
  PRIMARY KEY (`checkout_id`),
  KEY `user_id` (`user_id`),
  KEY `book_id` (`book_id`),
  CONSTRAINT `checkout_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`),
  CONSTRAINT `checkout_ibfk_2` FOREIGN KEY (`book_id`) REFERENCES `books` (`book_id`)
) ENGINE=InnoDB AUTO_INCREMENT=22 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `checkout`
--

LOCK TABLES `checkout` WRITE;
/*!40000 ALTER TABLE `checkout` DISABLE KEYS */;
INSERT INTO `checkout` VALUES (1,'Seunggwan',1,'2025-09-14','2025-09-20','2025-10-14',2300),(2,'Seunggwan',2,'2025-10-15','2025-10-21','2025-10-15',0),(3,'Seunggwan',2,'2025-10-15','2025-10-21','2025-10-15',0),(4,'Seunggwan',2,'2025-10-15','2025-10-21','2025-10-15',0),(5,'Seunggwan',1,'2025-10-15','2025-10-21','2025-10-15',0),(6,'Seunggwan',2,'2025-10-15','2025-10-21','2025-10-15',0),(7,'Seunggwan',20,'2025-10-15','2025-10-21','2025-10-17',0),(8,'human',13,'2025-10-17','2025-10-23','2025-10-17',0),(9,'user1',14,'2025-10-23','2025-10-29','2025-10-23',0),(10,'user1',2,'2025-10-23','2025-10-29','2025-10-23',0),(11,'user1',14,'2025-10-23','2025-10-29','2025-10-23',0),(12,'user1',14,'2025-10-23','2025-10-29','2025-10-23',0),(13,'user1',14,'2025-09-16','2025-09-22','2025-10-23',3000),(14,'user1',14,'2024-10-23','2025-10-29','2025-10-23',0),(18,'user',16,'2025-10-23','2025-10-29','2025-10-23',0),(19,'user',15,'2025-10-23','2025-10-29','2025-10-23',0),(20,'user',8,'2025-09-17','2025-09-23','2025-10-23',3000),(21,'user',17,'2025-10-23','2025-10-29','2025-10-23',0);
/*!40000 ALTER TABLE `checkout` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `user_id` varchar(255) NOT NULL,
  `user_pwd` varchar(255) DEFAULT NULL,
  `is_admin` tinyint(1) DEFAULT '0',
  `penalty` int DEFAULT '0',
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES ('admin','$2b$10$NWr7k2sgfU1xbmIPdtaD2O2lB086B6nl8yx0SVHSFfOSi7Kvc8eqe',1,0),('human','$2b$10$QiNz5wLWeqVELgshsjzF4uwiUnfTt.BQFUNwHg3HEVKaRDccZfKfq',0,0),('Seunggwan','$2b$10$eiuu04Tx2NsHRuTNlqFBUOTOFJsNdrJWtNJjvjQss05AhpwfQ79hi',1,0),('user','$2b$10$mIlwHAEHPBOdf.nO/hOiYuiciCnmSmiSBY23MEcFgzJpjnj63bKlC',0,0),('user1','$2b$10$2iMHyHQiF8lgtr0D6wzswuCaWjzz1H5giUF9W5XQ/Qle6NJhV/6aq',0,0);
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2025-10-23 23:31:02
