-- Fresh-install schema. Only the events table is seeded.
CREATE DATABASE IF NOT EXISTS `undefinedSpace`
  DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE `undefinedSpace`;

CREATE TABLE `servers` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `ip` text NOT NULL,
  `hostname` text NOT NULL,
  `token` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `events` (
  `id` int(11) NOT NULL,
  `description` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `items` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `id_server` int(11) NOT NULL,
  `id_type` tinyint(4) NOT NULL,
  `id_parent` bigint(20) DEFAULT NULL,
  `uuid` char(36) NOT NULL,
  `inode` bigint(20) DEFAULT NULL,
  `name` text NOT NULL,
  `time` timestamp NOT NULL,
  `hash` bigint(20) DEFAULT NULL,
  `deleted` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_items_uuid` (`uuid`),
  KEY `idx_items_inode` (`id_server`,`inode`),
  KEY `idx_items_parent` (`id_parent`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `projects` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_item` bigint(20) DEFAULT NULL,
  `id_server` int(11) NOT NULL,
  `path` text NOT NULL,
  `time_start` timestamp NOT NULL DEFAULT current_timestamp(),
  `time_stop` timestamp NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `accords` (
  `id_item` bigint(20) NOT NULL,
  `id_project` int(11) NOT NULL,
  PRIMARY KEY (`id_item`,`id_project`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `changes` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `id_item` bigint(20) NOT NULL,
  `id_event` bigint(20) DEFAULT NULL,
  `description` text DEFAULT NULL,
  `time` timestamp NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- IDs correspond to ResultOfCompare in src/DirSnapshot.h.
INSERT INTO `events` (`id`, `description`) VALUES
  (-1, 'NO_SNAPSHOT'),
  (0, 'IS_EMPTY'),
  (1, 'INPUT_IS_EMPTY'),
  (2, 'OUTPUT_IS_EMPTY'),
  (3, 'IS_CREATED'),
  (4, 'IS_DELETED'),
  (5, 'NEW_NAME'),
  (6, 'NEW_TIME'),
  (7, 'NEW_HASH'),
  (8, 'IS_EQUAL'),
  (9, 'DIRECTORY_END'),
  (10, 'INIT_PROJECT'),
  (11, 'START_CONTENT'),
  (12, 'END_CONTENT'),
  (13, 'START_FILE_LIST'),
  (14, 'END_FILE_LIST'),
  (15, 'NEW_PARENT'),
  (16, 'IS_REPLACED');
