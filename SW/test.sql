-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Aug 31, 2024 at 10:11 PM
-- Server version: 10.4.28-MariaDB
-- PHP Version: 8.2.4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `test`
--

-- --------------------------------------------------------

--
-- Table structure for table `health_data`
--

CREATE TABLE `health_data` (
  `id` int(11) NOT NULL,
  `userid` int(11) NOT NULL,
  `timestamp` timestamp NOT NULL DEFAULT current_timestamp(),
  `dev` varchar(10) DEFAULT NULL,
  `bat` int(11) DEFAULT NULL,
  `sis` int(11) DEFAULT NULL,
  `dia` int(11) DEFAULT NULL,
  `hrr` int(11) DEFAULT NULL,
  `spo` int(11) DEFAULT NULL,
  `kon` tinyint(1) DEFAULT NULL,
  `con` tinyint(1) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

--
-- Dumping data for table `health_data`
--

INSERT INTO `health_data` (`id`, `userid`, `timestamp`, `dev`, `bat`, `sis`, `dia`, `hrr`, `spo`, `kon`, `con`) VALUES
(1, 1, '2024-08-31 10:14:44', 'saya', 1, 1, 1, 1, 1, 1, 1),
(4, 1, '2024-08-31 10:17:46', 'AA', 97, 110, 80, 90, 98, 1, 1),
(5, 1, '2024-08-31 10:29:15', 'AA', 98, 102, 72, 87, 97, 1, 1),
(6, 1, '2024-08-31 10:29:51', 'AA', 99, 100, 70, 85, 96, 1, 1);

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` int(11) UNSIGNED NOT NULL,
  `username` varchar(50) NOT NULL,
  `password` varchar(255) NOT NULL,
  `roles` varchar(50) NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT current_timestamp(),
  `dev` varchar(32) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`id`, `username`, `password`, `roles`, `created_at`, `dev`) VALUES
(1, 'admin', '$2y$10$IbaOtphYEVIGjIqOaMcFTu3Plu9U6bqDEwQqPxEPJJILZN9BQBSpu', 'admin', '2024-08-28 11:45:08', '11'),
(2, 'user', '$2y$10$/0.2KRDuSdWL2VcTDb1eYOxnxc8CMR/Qf33.nLEzwUz8UB.qkuw0i', 'user', '2024-08-28 12:01:55', 'AA'),
(3, 'editor', '$2y$10$K5keu7JH1hYDUyaMzrFSge9Dye2C1V9D/IJT2t4z.DqSCYVAgnK1C', 'editor', '2024-08-28 12:03:07', NULL),
(5, 'latif', '$2y$10$FLDljVrui.X6Be2fmUTTP.GYJx4ASBJrgz2bGLdop8Lav1qK6NS8G', 'user', '2024-08-31 16:32:29', 'ab'),
(10, 'saya', '$2y$10$yB0yaHqG/ErQCC.StjI0Rel0OewlO9OHM6lnVumEfvLYG18jERoKW', 'user', '2024-08-31 16:41:27', 'saya'),
(12, 'bb', '$2y$10$LwaeWLY1umlFTe6kplWz6OviQxZf5XJMEidClyjMDAT48FMYLMKP2', 'user', '2024-08-31 16:49:53', 'bb');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `health_data`
--
ALTER TABLE `health_data`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `username` (`username`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `health_data`
--
ALTER TABLE `health_data`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=12;

--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=13;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
